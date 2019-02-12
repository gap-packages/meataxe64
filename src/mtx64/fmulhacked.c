// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// fmul.c  fMultiply function

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mezz.h"
#include "field.h"
#include "io.h"
#include "funs.h"

void fMultiply(const char * tmp,const char *m1, int s1, 
         const char *m2, int s2, const char *m3, int s3)
{
    uint64_t hdr1[5],hdr2[5];
    FIELD * f;
    uint64_t fdef,sizb;
    DSPACE ds;

    EPeek(m1,hdr1);
    EPeek(m2,hdr2);
    if( (hdr1[0]==1) && (hdr2[0]==1) )  // flat matrix multiply
    {
//  Consider disk chopping
        fdef=hdr1[1];
        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        DSSet(f,hdr2[3],&ds);    // look at size of matrix B
        sizb=ds.nob*hdr2[2];
        sizb=sizb/f->megabytes;
        sizb=sizb/660000;        // how many memoryfulls
        chops=1;
        while((chops*chops)<=sizb) chops++;
        if(chops==1)   // chopping into one piece!
        {
            mmul(m1,s1,m2,s2,m3,s3);
            return;
        }
// pratt around with filenames?
// chop into "chops" pieces
// do chops^3 multiplies
// assemble the answer back together again       
    }
    if( (hdr1[0]==1) && (hdr2[0]==3) )
    {
        fMulMatrixMap(m1,s1,m2,s2,m3,s3);
        return;
    }
    if( (hdr1[0]==3) && (hdr2[0]==3) )
    {
        fMulMaps(m1,s1,m2,s2,m3,s3);
        return;
    }
    printf("fMultiply cannot handle these matrix types\n");
    exit(20);
}

/* end of fmul.c  */
