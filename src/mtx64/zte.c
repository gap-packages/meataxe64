/*
      zte.c     MTX64 tensor product IV candidate
      =====     R. A. Parker   04.10.2015 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "tuning.h"
 
int main(int argc,  char **argv)
{
    EFIL *ef1,*ef2,*ef3;
    FIELD * f;
    FELT e1;
    uint64 hdr1[5],hdr2[5],hdr3[5];
    uint64 fdef,nor1,noc1,nor2,noc2,nor3,noc3;
    uint64 i1,i2,j1;
    DSPACE ds1,ds2,ds3;
    Dfmt *v1,*m2,*v3,*v4;
    long mem,wkr,thiswkr;

     LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zte m1 m2 ans");
        exit(21);
    }
 
    ef1 = ERHdr(argv[1],hdr1);
    ef2 = ERHdr(argv[2],hdr2);
    if(hdr1[1]!=hdr2[1])
    {
        LogString(80,"Matrices have different fields");
        exit(22);
    }
    fdef=hdr1[1];
    nor1=hdr1[2];
    nor2=hdr2[2];
    noc1=hdr1[3];
    noc2=hdr2[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    nor3=nor1*nor2;
    noc3=noc1*noc2;
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    DSSet(f,noc3,&ds3);
    hdr3[0]=1;
    hdr3[1]=fdef;
    hdr3[2]=nor3;
    hdr3[3]=noc3;
    hdr3[4]=0;
    ef3 = EWHdr(argv[3],hdr3);
/*  Strategy 0     zero rows or cols           */
/*  Still need to read the input for checksums!*/
    if( (noc3==0) || (nor3==0) )
    {
        if( (nor1!=0) && (noc1!=0) )
        {
            v1=malloc(ds1.nob);
            for(i1=0;i1<nor1;i1++)
                ERData(ef1,ds1.nob,v1);
            free(v1);
        }
        if( (nor2!=0) && (noc2!=0) )
        {
            v1=malloc(ds2.nob);
            for(i1=0;i1<nor2;i1++)
                ERData(ef2,ds1.nob,v1);
            free(v1);
        }
        ERClose(ef1);
        ERClose(ef2);
        EWClose(ef3);
        free(f);
        return 0;
    }
/*  Strategy 1 - read in matrix 2, and then - */
/*      wkr rows at a time (ideally wkr=nor2) */
/*      make wkr rows of the output by taking */
/*      that many rows of matrix 2, scaling   */
/*      and then pasting into the answer      */
    mem=MEGABYTES;
    wkr=1000000;
    mem=mem*wkr;
    mem-=nor2*ds2.nob;
    mem-=ds1.nob;
    wkr=mem/(ds2.nob+ds3.nob);
    if(wkr>nor2)wkr=nor2;
    if(wkr>0)                // strategy 1 it is
    {
        v1=malloc(ds1.nob);
        m2=malloc(ds2.nob*nor2);
        v3=malloc(ds3.nob*wkr);
        v4=malloc(ds2.nob*wkr);
        ERData(ef2,nor2*ds2.nob,m2);   // Read in matrix 2
/* for each row of matrix 1 */
        for(i1=0;i1<nor1;i1++)
        {
            ERData(ef1,ds1.nob,v1);
            i2=0;
/* for each wkr-load of matrix 2  */
            while(i2<nor2)
            {
                thiswkr=nor2-i2;
                if(thiswkr>wkr) thiswkr=wkr;
                memset(v3,0,ds3.nob*thiswkr);
/* for each column of matrix 1 */
                for(j1=0;j1<noc1;j1++)
                {
                    e1=DUnpak(&ds1,j1,v1);
                    DCpy(&ds2,m2+ds2.nob*i2,thiswkr,v4);
                    DSMul(&ds2,e1,thiswkr,v4);
                    DPaste(&ds2,v4,thiswkr,j1*noc2,&ds3,v3);
                }
                EWData(ef3,ds3.nob*thiswkr,v3);
                i2+=thiswkr;
            }
        }
        ERClose(ef1);
        ERClose(ef2);
        EWClose(ef3);
        free(v1);
        free(m2);
        free(v3);
        free(v4);
        free(f);
        return 0;
    }
/*  Strategy 2 - read through matrix 2 once   */
/*      for each row of matrix 1.  wkr rows   */
/*      of matrix 2 and the result are        */
/*      dealt with at once using copy, scale  */
/*      and paste                             */
    mem=MEGABYTES;
    wkr=1000000;
    mem=mem*wkr;
    mem-=ds1.nob;
    wkr=mem/(2*ds2.nob+ds3.nob);
    if(wkr>nor2) wkr=nor2;    // not sure this can happen
    if(wkr>0)   // is strategy 2 viable?
    {
        v1=malloc(ds1.nob);
        m2=malloc(ds2.nob*wkr);
        v3=malloc(ds3.nob*wkr);
        v4=malloc(ds2.nob*wkr);

/* for each row of matrix 1 */
        for(i1=0;i1<nor1;i1++)
        {
            ERData(ef1,ds1.nob,v1);
            i2=0;
/* for each wkr-load of matrix 2  */
            while(i2<nor2)
            {
                thiswkr=nor2-i2;
                if(thiswkr>wkr) thiswkr=wkr;
                ERData(ef2,thiswkr*ds2.nob,m2);   // Read in some matrix 2
                memset(v3,0,ds3.nob*thiswkr);
/* for each column of matrix 1 */
                for(j1=0;j1<noc1;j1++)
                {
                    e1=DUnpak(&ds1,j1,v1);
                    DCpy(&ds2,m2,thiswkr,v4);
                    DSMul(&ds2,e1,thiswkr,v4);
                    DPaste(&ds2,v4,thiswkr,j1*noc2,&ds3,v3);
                }
                EWData(ef3,ds3.nob*thiswkr,v3);
                i2+=thiswkr;
            }
            if((i1+1)<nor1)
            {
                ERClose1(ef2,1);
                ef2 = ERHdr(argv[2],hdr2);
            }
        }
        ERClose(ef1);
        ERClose(ef2);
        EWClose(ef3);
        free(v1);
        free(m2);
        free(v3);
        free(v4);
        free(f);
        return 0;
    }
    printf("No strategy available for tensor product\n");
    return 1;
}      

/******  end of zte.c    ******/
