/*
      zcr.c     meataxe-64 columns riffle
      =====     R. A. Parker    14.07.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3,*e4;
    uint64 hdr1[5],hdr2[5],hdr3[5];
    FIELD * f;
    uint64 fdef,nor,noc,noc1,noc2;
    DSPACE ds,ds1,ds2;
    Dfmt *d,*d1,*d2;
    uint64 siz,r;
    uint64 * bs;

    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 5)
    {
        LogString(80,"usage zcr bs set unset result");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // matrix set
    e3=ERHdr(argv[3],hdr3);   // matrix unset

    if( (hdr1[3]!=hdr2[3]) || (hdr2[2]!=hdr3[2]) )
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
//  probably ought to check some more sanity
    fdef=hdr2[1];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    nor=hdr2[2];
    noc1=hdr2[3];
    noc2=hdr3[3];
    noc=noc1+noc2;
    DSSet(f,noc,&ds);
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    d=malloc(ds.nob);
    d1=malloc(ds1.nob);
    d2=malloc(ds2.nob);
    siz = 8*(2+(nor+63)/64);
    bs=malloc(siz);
    ERData(e1,siz,(uint8 *)bs);
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor;
    hdr1[3]=noc;
    hdr1[4]=0;
    e4 = EWHdr(argv[4],hdr1);
    for(r=0;r<nor;r++)
    {
        ERData(e2,ds1.nob,d1);
        ERData(e3,ds2.nob,d2);
        BSColRiffle(f,bs,1,d1,d2,d);
        EWData(e4,ds.nob,d);
    }
    ERClose(e1);
    ERClose(e2);
    ERClose(e3);
    EWClose(e4);
    free(f);
    free(d);
    free(d1);
    free(d2);
    free(bs);
    return 0;

}      /******  end of zcr.c    ******/
