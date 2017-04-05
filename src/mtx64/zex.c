/*
      zex.c     meataxe-64 Echelon form EXplicit
      =====     R. A. Parker    18.05.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3;
    uint64 hdr1[5],hdr2[5],hdr3[5];
    FIELD * f;
    uint64 fdef,nor,nor1,nor2,noc,noc1,noc2,i;
    DSPACE ds,ds1,ds2;
    Dfmt *d,*d1,*d2;
    FELT min1;
    size_t siz;
    uint64 * bs;

    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zex bs rem result");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // remnant
    nor1=hdr1[3];     // -1 nor = set bits;
    noc1=nor1;        // -1 noc also = set bits
    nor2=hdr2[2];     // rem nor from header
    noc2=hdr2[3];     // rem noc also from header
    noc=hdr1[2];      // result noc = number of bits
    nor=nor1;         // result nor = nor1 = nor2
    if( (nor1!=nor2) || (noc!=noc1+noc2) )
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr2[1];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    hdr3[0]=1;
    hdr3[1]=fdef;
    hdr3[2]=nor;
    hdr3[3]=noc;
    hdr3[4]=0;
    e3 = EWHdr(argv[3],hdr3);
    DSSet(f,noc,&ds);
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    d=malloc(ds.nob);
    d1=malloc(ds1.nob);
    d2=malloc(ds2.nob);
    min1=FieldNeg(f,1);
    siz = 8*(2+(noc+63)/64);
    bs=malloc(siz);
    ERData(e1,siz,(uint8 *)bs);
    ERClose(e1);
    for(i=0;i<nor;i++)
    {
        memset(d1,0,ds1.nob);
        DPak(&ds1,i,d1,min1);
        ERData(e2,ds2.nob,d2);
        BSColRiffle (f,bs,1,d1,d2,d);
        EWData(e3,ds.nob,d);
    }
    ERClose(e2);
    EWClose(e3);
    free(f);
    free(d);
    free(d1);
    free(d2);
    free(bs);
    return 0;
}
/******  end of zex.c    ******/
