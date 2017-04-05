/*
    zmu3.c meataxe64 layer-3 matrix multiplication
    ====== R. A. Parker 11.9.2016
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "tuning.h"

int main(int argc, char **argv)
{
    M3 A,B,C;
    MOJ flow,fmoj,X,Y,Z;
    FIELD * f;
    uint64_t i,j,k;
    DSPACE da,db,dw;
    int strat,buffering,concur;
    uint64_t D1,D2,siza,sizb,sizc,sizw;
    if (argc != 4)
    {
        LogString(80,"usage zmu <m1> <m2> <sum>");
        exit(14);
    }
    LogCmd(argc,argv);
    TFInit(THREADS);
    TFStopMOJFree();

    A=M3Cons(argv[1]);  // Construct and set filename
    M3Peek(A);          // get field, nor noc by peeking
    B=M3Cons(argv[2]);  // Construct and set filename
    M3Peek(B);          // get field, nor noc by peeking
    C=M3Cons(argv[3]);  // Construct and set filename
    M3Same(C,A);        // get field fmoj nor noc from A
    C->noc=B->noc;      // But C has cols of B, not A

/* sanity check */

    if( (A->fdef!=B->fdef) || (A->noc!=B->nor) )
    {
        LogString(80,"matrices incompatible");
        exit(15);
    }

/* Allocate and set field MOJ */

    fmoj=M3FieldMOJ(A->fdef);  // Create the field MOJ
    A->fmoj=fmoj;
    B->fmoj=fmoj;
    C->fmoj=fmoj;

    f=(FIELD *) TFPointer(fmoj);

/*  =====  Decide on the strategy  */

/* how much memory do the matrices take up?  */
    DSSet(f,A->noc,&da);
    DSSet(f,B->noc,&db);
    siza=da.nob*A->nor;
    sizb=db.nob*B->nor;
    sizc=db.nob*C->nor;
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
    if( ((siza+sizb+sizc)/1000000)<MEGABYTES)
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
        if(((A->c*sizc+THREADS*5*sizw)/1000000)<MEGABYTES) strat=3;
    }
/* strategy 2 may be correct if A+C smaller than B  */
    if( (strat==0) && ((siza+sizc)<sizb) && (((siza+sizc)/500000)<MEGABYTES))
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
/* Now consider Strategy 1 - read B then through A,C flow-controlled */
    if( (strat==0) && ((sizb/500000)<MEGABYTES) )
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
    C->r=A->r;
    C->c=B->c;

#ifdef DEBUG
printf("Strategy %d chops %lu %lu %lu buf %d\n",
                strat,A->c,A->r,C->c,buffering);
#endif


    M3EvenChop(A);      // Chop as evenly as possible
    M3EvenChop(B);      // Chop B likewise
    M3MOJs(A);          // Allocate MOJs for A
    M3MOJs(B);          // ditto for B
    M3EvenChop(C);      // Chop C evenly also
    M3MOJArray(C);      // allocate MOJ array but not MOJs
    if(strat==1)
    {
        flow=TFFlowCons(buffering,1,1); // Flow control. 
        A->fl=flow;         //    then A must wait
        C->fl=flow;         //    for rows of C to be written.
    }
    if(strat==2)
    {
        flow=TFFlowCons(buffering*B->c,1,B->c);
        B->fl=flow;
    }
    M3Read(A);          // set off read thread for A
    M3Read(B);          // set off read thread for B
    if(strat==1)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                for(j=1;j<A->c;j++)
                {
                    MAD(i,fmoj,A->m[i][j],B->m[j][k],X,Y);
                    X=Y;
                }
                C->m[i][k]=X;
            }
    }

    if(strat==2)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                FMV(1,X,flow,Y);
                for(j=1;j<A->c;j++)
                {
                    MAD(i,fmoj,A->m[i][j],B->m[j][k],Y,X);
                    FMV(1,X,flow,Y);
                }
                C->m[i][k]=Y;
            }
    }
    if(strat==3)
    {
        for(i=0;i<A->r;i++)
            for(k=0;k<B->c;k++)
            {
                MUL(i,fmoj,A->m[i][0],B->m[0][k],X);
                for(j=1;j<A->c;j++)
                {
                    MUL(i,fmoj,A->m[i][j],B->m[j][k],Y);
                    ADD(i,fmoj,X,Y,Z);
                    X=Z;
                }
                C->m[i][k]=X;
            }
    }


    M3Write(C);
    TFStartMOJFree();
    TFWaitEnd();
    if(strat==1)
        TFFlowDest(flow);
    TFRelease(fmoj);
    M3Dest(A);
    M3Dest(B);
    M3Dest(C);
    TFClose();
}

/*   end of zmu3.c  */
