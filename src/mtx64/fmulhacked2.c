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
#include "slab.h"

void fMultiply(const char * tmp,const char *m1, int s1, 
         const char *m2, int s2, const char *m3, int s3)
// tmp ignored at the moment
{
    uint64_t hdr1[5],hdr2[5];
    FIELD * f;
    uint64_t fdef,siz,sizb,chops,nor1,noc1,nor2,noc2;
    DSPACE ds1,ds2;
    EFIL *ef1,*ef2,*ef3;
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
// first look if matrix "small" - N^3 < 10^6
        siz=nor1*noc1;
        if(siz<=1000000) siz=siz*noc2;
        if(siz<1000000)   // small matrix - do in Dfmt;
        {
            ef1 = ERHdr(m1,hdr1);
            ef2 = ERHdr(m2,hdr2);
// should I check that someone switched the files
// between peek and read?
printf("Doing in Dfmt\n");
            DSSet(f,noc1,&ds1);
            DSSet(f,noc2,&ds2);
            hdr2[2]=nor1;
            ef3 = EWHdr(m3,hdr2);
 
	    /******  Allocate space for all the matrices  */
            m1=malloc(ds1.nob*nor1);
            m2=malloc(ds2.nob*nor2);
            m3=malloc(ds2.nob);

/* first read in matrices 1 and 2  */
            ERData(ef1,nor1*ds1.nob,m1);
            ERData(ef2,nor2*ds2.nob,m2);
            ERClose(ef2);
            ERClose(ef1);
            da=m1;

            for(i=0;i<nor1;i++)
            {
                memset(m3,0,ds2.nob);
                db=(Dfmt *) m2;
                for(j=0;j<noc1;j++)
                {
                    e=DUnpak(&ds1,j,da);
                    DSMad(&ds2,e,1,db,m3);
                    db+=ds2.nob;
                }
                EWData(ef3,ds2.nob,m3);
                da+=ds1.nob;
            }
            EWClose(ef3);
            free(f);
            free(m1);
            free(m2);
            free(m3);
            return;
        }
//  Consider disk chopping
        DSSet(f,hdr2[3],&ds1);    // look at size of matrix B
        sizb=ds1.nob*hdr2[2];
        sizb=sizb/f->megabytes;
        sizb=sizb/660000;        // how many memoryfulls
        chops=1;
        while((chops*chops)<=sizb) chops++;
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
    printf("fMultiply cannot handle these matrix types\n");
    exit(20);
}

/* end of fmul.c  */
