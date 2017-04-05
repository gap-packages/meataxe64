/*
    zad3.c meataxe64 layer-3 matrix addition
    ====== R. A. Parker 30.8.2016
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
    MOJ flowa,flowb,X,fmoj;
    uint64_t i,j,rows;
    if (argc != 4)
    {
        LogString(80,"usage zad <m1> <m2> <sum>");
        exit(14);
    }
    LogCmd(argc,argv);
    TFInit(THREADS);
    TFStopMOJFree();

    A=M3Cons(argv[1]);  // Construct and set filename
    M3Peek(A);          // get field, nor noc by peeking
    B=M3Cons(argv[2]);  // Construct and set filename
    M3Peek(B);          // get field, nor noc by peeking

/* sanity check */

    if( (A->fdef!=B->fdef) || (A->nor!=B->nor) || (A->noc != B->noc) )
    {
        LogString(80,"matrices incompatible");
        exit(15);
    }

/* Allocate and set field MOJ */

    fmoj=M3FieldMOJ(A->fdef);  // Create the field MOJ
    A->fmoj=fmoj;
    B->fmoj=fmoj;

/*  =====  Decide how to chop the matrices ====== */

    rows=1000;
    A->r=(A->nor+rows-1)/rows;
    A->c=1;

/*  =====     End of chopping decisions    ====== */

    M3EvenChop(A);      // Chop as evenly as possible
    M3CopyChop(B,A);    // Chop B the same as A
    M3MOJs(A);          // Allocate MOJs for A
    M3MOJs(B);          // ditto for B
    C=M3Cons(argv[3]);  // Construct and set filename
    M3Same(C,A);        // get field fmoj nor noc from A
    M3CopyChop(C,A);    // Chop C the same as A
    M3MOJArray(C);      // allocate MOJ array but not MOJs
    flowa=TFFlowCons(3,1,A->c); // Flow control for A
    flowb=TFFlowCons(3,1,1); // Flow control for B
    A->fl=flowa;         //    a waits for the adds
    B->fl=flowb;         //    B waits for the writes
    C->fl=flowb;         //
    M3Read(A);          // set off read thread for A
    M3Read(B);          // set off read thread for B
    for(i=0;i<A->r;i++)
        for(j=0;j<A->c;j++)
        {
            ADD(i+2,A->fmoj,A->m[i][j],B->m[i][j],X);
            FMV(1,X,flowa,C->m[i][j]);
        }
    M3Write(C);
    TFStartMOJFree();
    TFWaitEnd();
    TFFlowDest(flowa);
    TFFlowDest(flowb);
    TFRelease(fmoj);
    M3Dest(A);
    M3Dest(B);
    M3Dest(C);
    TFClose();
}

/*   end of zad3.c  */
