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
// tmp ignored at the moment
{
    uint64_t hdr1[5],hdr2[5];
    EPeek(m1,hdr1);
    EPeek(m2,hdr2);
    if( (hdr1[0]==1) && (hdr2[0]==1) )
    {
        mmul(m1,s1,m2,s2,m3,s3);
        return;
    }
    if( (hdr1[0]==1) && (hdr2[0]==3) )
    {
        fMulMatrixMap(m1,s1,m2,s2,m3,s3);
        return;
    }
    printf("fMultiply cannot handle these matrix types\n");
    exit(20);
}

/* end of fmul.c  */
