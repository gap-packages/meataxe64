/*
    ztr3.c meataxe64 layer-3 matrix transpose
                  Obsolete
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
    M3 A,B;
    MOJ fmoj;
    uint64_t i,j;
    if (argc != 3)
    {
        LogString(80,"usage ztr <m1> <m2> <sum>");
        exit(14);
    }
    LogCmd(argc,argv);
    TFInit(THREADS);
    TFStopMOJFree();

    A=M3Cons(argv[1]);  // Construct input and set filename
    B=M3Cons(argv[2]);  // Construct output and set filename
    M3Peek(A);          // get field, nor noc by peeking

/* Allocate and set field MOJ */

    fmoj=M3FieldMOJ(A->fdef);  // Create the field MOJ
    A->fmoj=fmoj;
    M3Same(B,A);        // get field fmoj nor noc from A
    B->nor=A->noc;      // correct nor
    B->noc=A->nor;      // and noc for output

/*  =====  Decide how to chop the matrices ====== */
/* chop into bits no smaller than 120 entries     */
/* and no more than 25 pieces in each direction   */

    A->r=A->nor/120;
    if(A->r>25) A->r=25;
    if(A->r<1) A->r=1;
    A->c=A->noc/120;
    if(A->c>25) A->c=25;
    if(A->c<1) A->c=1;

/*  =====     End of chopping decisions    ====== */

    B->r=A->c;
    B->c=A->r;

    M3EvenChop(A);      // Chop as evenly as possible
    M3EvenChop(B);      // Chop B evenly also
    M3MOJs(A);          // Allocate MOJs for A
    M3MOJArray(B);      // allocate MOJ array but not MOJs
    M3Read(A);          // set off read thread for A
    for(i=0;i<A->r;i++)
        for(j=0;j<A->c;j++)
            TRA(j,fmoj,A->m[i][j],B->m[j][i]);
    M3Write(B);
    TFStartMOJFree();
    TFWaitEnd();
    TFRelease(fmoj);
    M3Dest(A);
    M3Dest(B);
    TFClose();
}

/*   end of ztr3.c  */
