/*
    mmul.c meataxe64 single file matrix multiply 
    ====== R. A. Parker 1.6.2017
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "tuning.h"

// #define DEBUG 1

void mmul(const char *m1, int s1, const char *m2, int s2, const char * m3, int s3)
{
    M3 A,B,C;
    MOJ flow,fmoj,X,Y,Z;
    FIELD * f;
    uint64_t i,j,k;
    uint64_t caul,base;
    DSPACE da,db,dw;
    int strat,buffering,concur;
    uint64_t D1,D2,siza,sizb,sizc,sizw;

    TFInit(THREADS);
    TFStopMOJFree();

    A=M3Cons(m1,s1);    // Construct and set filename
    M3Peek(A);          // get field, nor noc by peeking
    B=M3Cons(m2,s2);    // Construct and set filename
    M3Peek(B);          // get field, nor noc by peeking
    C=M3Cons(m3,s3);    // Construct and set filename
    C->fdef=A->fdef;
    C->nor=A->nor;
    C->noc=B->noc;    

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
    D1=800;
    if(f->fdef>65536) D1=1400/(f->pow+1);
    if( (f->fdef<=16) && (f->charc<5) ) D1=8000;
/*   no suitable strategy found yet  */
    strat=0;
// default concurrency and buffering
    concur=20;
    if(concur<2*(THREADS+3)) concur=2*(THREADS+3);
    buffering=2;
    if(THREADS>10) buffering=3;
    if(THREADS>25) buffering=4;

/* First consider Strategy 3 - just blast them all off  */
    if( ((siza+sizb+sizc)/1000000)<f->megabytes)
    {
        D2=D1;
        while(1)
        {
            A->r=(A->nor+D2-1)/D2;
            A->c=(A->noc+D2-1)/D2;
            B->c=(B->noc+D2-1)/D2;
            if((A->r*A->c*B->c) < 3*concur) break;
            D2=D2+D2/5;
        }
        DSSet(f,D2,&dw);
        sizw=dw.nob*D2;
        if(((A->c*sizc+THREADS*5*sizw)/1000000)<f->megabytes) strat=3;
    }
/* strategy 2 may be correct if A+C smaller than B  */
    if( (strat==0) && ((siza+sizc)<sizb) &&
                (((siza+sizc)/500000)<f->megabytes))
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
    if( (strat==0) && ((sizb/500000)<f->megabytes) )
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
    if(A->c > MAXCHOP) A->c=MAXCHOP;
    if(A->r > MAXCHOP) A->r=MAXCHOP;
    if(B->c > MAXCHOP) B->c=MAXCHOP;
    if(A->c==0) A->c=1;
    if(A->r==0) A->r=1;
    if(B->c==0) B->c=1;

    B->r=A->c;
    C->r=A->r;
    C->c=B->c;
// Chop all as evenly as possible
    caul=f->entbyte*8;
    if( (f->cauldron!=0) && ((f->pow==1) || (f->linfscheme!=0)) )  //HPMI
    {
        caul=f->cauldron;
        base=C->noc/(B->c * caul);
        if(base>=2) caul=caul*2;
        if(base>=6) caul=caul*2;
        if(base>=16) caul=caul*2;
    }
    M3EvenChop(A,8,f->entbyte*8,8,f->entbyte*8); 
    M3EvenChop(B,f->entbyte*8,caul,f->entbyte*8,f->entbyte*8);
    M3MOJs(A);          // Allocate MOJs for A
    M3MOJs(B);          // ditto for B
    M3EvenChop(C,8,caul,8,f->entbyte*8);
#ifdef DEBUG
printf("%lu %ld Strategy %d chops %lu %lu %lu buf %d\n",
              A->fdef, A->nor, strat,A->c,A->r,C->c,buffering);
printf(" Arow");
for(i=0;i<A->r;i++) printf(" %ld",A->rnor[i]);
printf(" Acol");
for(i=0;i<A->c;i++) printf(" %ld",A->cnoc[i]);
printf(" Bcol");
for(i=0;i<B->c;i++) printf(" %ld",B->cnoc[i]);
printf("\n");
#endif
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
    if(strat==1)            // I think this needs doing
        TFFlowDest(flow);   // also if strat==2
    TFRelease(fmoj);
    M3Dest(A);
    M3Dest(B);
    M3Dest(C);
    TFClose();
}

/*   end of zmu3.c  */
