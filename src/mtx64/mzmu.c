/*
    mzmu.c meataxe64 mezzanine matrix multiplication
    ======       R. A. Parker 30.11.2016
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "tuning.h"
#include "mezz.h"

void mzmu(const char *out, int sout, const char *ina, int sina, 
          const char *inb, int sinb, const char * inc, int sinc, int megabytes)
{
    M3 A,B,C,D;
    MOJ flow,fmoj,X,Y,Z;
    FIELD * f;
    uint64_t i,j,k;
    DSPACE da,db,dw;
    int strat,buffering,concur;
    uint64_t D1,D2,siza,sizb,sizc,sizw;

    TFInit(THREADS);
    TFStopMOJFree();

    A=M3Cons(ina);      // Construct and set filename
    A->silent=sina;
    M3Peek(A);          // get field, nor noc by peeking
    B=M3Cons(inb);      // Construct and set filename
    B->silent=sinb;
    M3Peek(B);          // get field, nor noc by peeking
    if(inc!=NULL)
    {
        C=M3Cons(inc);
        M3Peek(C);
        C->silent=sinc;
    }
    D=M3Cons(out);      // Construct and set filename
    D->silent=sout;
    M3Same(D,A);        // get field fmoj nor noc from A
    D->noc=B->noc;      // But D has cols of B, not A

/* sanity checks */

    if( (A->fdef!=B->fdef) || (A->noc!=B->nor) )
    {
        LogString(80,"matrices incompatible");
        exit(15);
    }
    if(inc!=NULL)
        if( (C->fdef!=B->fdef) || (C->nor!=A->nor) || (C->noc!=B->noc) )
        {
            LogString(80,"matrices incompatible");
            exit(16);
        }

/* Allocate and set field MOJ */

    fmoj=M3FieldMOJ(A->fdef);  // Create the field MOJ
    A->fmoj=fmoj;
    B->fmoj=fmoj;
    if(inc!=NULL) C->fmoj=fmoj;
    D->fmoj=fmoj;

    f=(FIELD *) TFPointer(fmoj);

/*  =====  Decide on the strategy  */

/* how much memory do the matrices take up?  */
    DSSet(f,A->noc,&da);
    DSSet(f,B->noc,&db);
    siza=da.nob*A->nor;
    sizb=db.nob*B->nor;
    sizc=db.nob*D->nor;
/* what is the minimum size to chop for the field */
/*  This needs to be adjusted as more fields get HPMI */
    D1=400;
    if(f->fdef>65536) D1=700/(f->pow+1);
    if( (f->fdef<=16) && (f->charc<5) ) D1=4000;
/*   no suitable strategy found yet  */
    strat=0;
// default concurrency and buffering
    concur=9;
    if(concur<THREADS+3) concur=THREADS+3;
    buffering=2;
    if(THREADS>10) buffering=3;
    if(THREADS>25) buffering=4;

/* First consider Strategy 3 - just blast them all off  */
    if( ((siza+sizb+sizc)/1000000)<megabytes)
    {
        D2=D1;
        while(1)
        {
            A->r=(A->nor+D2-1)/D2;
            A->c=(A->noc+D2-1)/D2;
            B->c=(B->noc+D2-1)/D2;
            if((A->r*A->c*B->c) < 3*THREADS) break;
            D2=D2+D2/5;
        }
        DSSet(f,D2,&dw);
        sizw=dw.nob*D2;
        if(((A->c*sizc+THREADS*5*sizw)/1000000)<megabytes) strat=3;
    }
/* strategy 2 may be correct if A+D smaller than B  */
    if( (strat==0) && ((siza+sizc)<sizb) && (((siza+sizc)/500000)<megabytes))
    {
        D2=D1;
        while(1)
        {       
            A->r=(A->nor+D2-1)/D2;
            B->c=(B->noc+D2-1)/D2;
            if((A->r*B->c) < concur) break;
            D2=D2+D2/5;
        }
        A->c=(A->noc+D2-1)/D2;
        strat=2;
    }
/* Now consider Strategy 1 - read B then through A,D flow-controlled */
    if( (strat==0) && ((sizb/500000)<megabytes) )
    {
        D2=D1;
        while(1)
        {
            B->c=(B->noc+D2-1)/D2;
            if((B->c) < concur) break;
            D2=D2+D2/5;
        }
        A->r=(A->nor+D2-1)/D2;
        A->c=(A->noc+D2-1)/D2;
        strat=1;
    }
// may needs a more memory-conservative strategy or two
    if(strat==0)
    {
        printf("No strategy available for this multiplication\n");
        exit(11);
    }
    
/*  =====     End of chopping decisions    ====== */

    B->r=A->c;
    D->r=A->r;
    D->c=B->c;
    if(inc!=NULL)
    {
        C->r=D->r;
        C->c=D->c;
    }
#ifdef DEBUG
printf("Strategy %d chops %lu %lu %lu buf %d\n",
                strat,A->c,A->r,D->c,buffering);
#endif

    M3EvenChop(A);      // Chop as evenly as possible
    M3EvenChop(B);      // Chop B likewise
    M3MOJs(A);          // Allocate MOJs for A
    M3MOJs(B);          // ditto for B
    if(inc!=NULL)
    {
        M3EvenChop(C);      // Chop as evenly as possible
        M3MOJs(C);          // Allocate MOJs for C
    }
    M3EvenChop(D);      // Chop D evenly also
    M3MOJArray(D);      // allocate MOJ array but not MOJs
    if(strat==1)
    {
        flow=TFFlowCons(buffering,1,1); // Flow control. 
        A->fl=flow;         //    then A must wait
        D->fl=flow;         //    for rows of D to be written.
// need to flow-control C too!
    }
    if(strat==2)
    {
        flow=TFFlowCons(buffering*B->c,1,B->c);
        B->fl=flow;
    }
    M3Read(A);          // set off read thread for A
    M3Read(B);          // set off read thread for B
    if(inc!=NULL) M3Read(C);
    if(strat==1)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                if(inc==NULL)
                    MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                else
                    MAD(i,fmoj,A->m[i][0],B->m[0][k],C->m[i][k],X);
                for(j=1;j<A->c;j++)
                {
                    MAD(i,fmoj,A->m[i][j],B->m[j][k],X,Y);
                    X=Y;
                }
                D->m[i][k]=X;
            }
    }

    if(strat==2)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                if(inc==NULL)
                    MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                else
                    MAD(i,fmoj,A->m[i][0],B->m[0][k],C->m[i][k],X);
                FMV(1,X,flow,Y);
                for(j=1;j<A->c;j++)
                {
                    MAD(i,fmoj,A->m[i][j],B->m[j][k],Y,X);
                    FMV(1,X,flow,Y);
                }
                D->m[i][k]=Y;
            }
    }
    if(strat==3)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                if(inc==NULL)
                    MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                else
                    MAD(i,fmoj,A->m[i][0],B->m[0][k],C->m[i][k],X);
                for(j=1;j<A->c;j++)
                {
                    MUL(i,fmoj,A->m[i][j],B->m[j][k],Y);
                    ADD(i,fmoj,X,Y,Z);
                    X=Z;
                }
                D->m[i][k]=X;
            }
    }

    M3Write(D);
    TFStartMOJFree();
    TFWaitEnd();
    if(strat==1)
        TFFlowDest(flow);
    TFRelease(fmoj);
    M3Dest(A);
    M3Dest(B);
    M3Dest(D);
    TFClose();
}

/*   end of mzmu.c  */
