// Copyright (C) Richard Parker 2019
// Meataxe64 Early Diskchop version
// fmul.c  fMultiply function  

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mezz.h"
#include "field.h"
#include "io.h"
#include "funs.h"
#include "slab.h"

void fMultiply(const char * tmp,const char *m1, int s1, 
         const char *m2, int s2, const char *m3, int s3)
{
    uint64_t hdr1[5],hdr2[5];
    FIELD * f;
    EFIL *e1,*e2,*e3;
    Dfmt *am,*bm,*cm;
    uint64_t fdef,siz,sizac,sizb,chops,nor1,noc1,nor2,noc2;
    DSPACE ds1,ds2;
    uint64_t i,j;
    EPeek(m1,hdr1);
    EPeek(m2,hdr2);
    if( (hdr1[0]==1) && (hdr2[0]==1) )  //flat matrix multiply
    {
        fdef=hdr1[1];
        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        nor1=hdr1[2];
        noc1=hdr1[3];
        nor2=hdr2[2];
        noc2=hdr2[3];
        if( (noc1!=nor2) || (fdef!=hdr2[1]) )
        {
            printf("Matrices incompatible\n");
            exit(27);
        }
        DSSet(f,noc1,&ds1);
        DSSet(f,noc2,&ds2);
// first look if matrix "small" - N^3 < 10^6
        siz=nor1*noc1;
        if(siz<=1000000) siz=siz*noc2;   // avoid overflow
        if(siz<1000000)   // small matrix - do in using slab;
        {
            e1=ERHdr(m1,hdr1);
            e2=ERHdr(m2,hdr2);
            hdr2[2]=nor1;
            e3=EWHdr(m3,hdr2);
            am=malloc(ds1.nob*nor1);
            bm=malloc(ds2.nob*nor2);
            cm=malloc(ds2.nob*nor1);
            ERData(e1,ds1.nob*nor1,am);
            ERClose1(e1,s1);
            ERData(e2,ds2.nob*nor2,bm);
            ERClose1(e2,s2);
            SLMul(f,am,bm,cm,nor1,noc1,noc2);
            EWData(e3,ds2.nob*nor1,cm);
            EWClose1(e3,s3);
            free(f);
            free(am);
            free(bm);
            free(cm);
            return;
        }
//  Consider disk chopping
//  if B, or A+C fit in memory,mmul is the way to go
        sizb=ds2.nob*nor2;
        sizac=(ds1.nob+ds2.nob)*nor1;
        siz=sizb;
        if(sizac<siz) siz=sizac;    // siz is smaller of B, A+C
        siz=siz/f->megabytes;
        siz=siz/660000;        // how many memoryfulls
        chops=1;
        while((chops*chops)<=siz) chops++;
        if(chops==1)   // chopping into one piece!
        {
            mmul(m1,s1,m2,s2,m3,s3);
            return;
        }
        printf("Chopping needed but not yet implemented\n");
        exit(2);
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
    if( (hdr1[0]==3) && (hdr2[0]==1) )    // map * matrix
    {
        e1=ERHdr(m1,hdr1);
        e2=ERHdr(m2,hdr2);
        fdef=hdr2[1];
        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        nor1=hdr1[2];
        noc1=hdr1[3];
        nor2=hdr2[2];
        noc2=hdr2[3];
        DSSet(f,noc2,&ds2);
        if(noc1!=nor2)
        {
            printf("map and matrix incompatible\n");
            exit(7);
        }
        hdr2[2]=nor1;
        e3=EWHdr(m3,hdr2);
        bm=malloc(ds2.nob*nor2);
        ERData(e2,ds2.nob*nor2,bm);
        ERClose1(e2,s2);
        for(i=0;i<nor1;i++)
        {
            ERData(e1,8,(uint8_t *)&j);
            EWData(e3,ds2.nob,bm+j*ds2.nob);
        }
        ERClose1(e1,s1);
        EWClose1(e3,s3);
        free(bm);
        return;
    }
    printf("fMultiply cannot handle these matrix types\n");
    exit(20);
}

/* end of fmul.c  */
