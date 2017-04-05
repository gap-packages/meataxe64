/*
      zcx.c     meataxe-64 column extract
      =====     R. A. Parker    7.07.2014
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
    FIELD * f;
    DSPACE ds,dssel,dsnon;
    uint64 fdef,nor,noc,noc1,noc2;
    uint64 siz,r,r1;
    uint64 hdr1[5],hdr2[5];
    uint64 * bs;
    Dfmt *d,*d1,*d2;

    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 5)
    {
        LogString(80,"usage zcx bs m sel nonsel");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // matrix
    if(hdr1[2]!=hdr2[3])      // bits must equal columns
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr2[1];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    nor=hdr2[2];
    noc=hdr2[3];
    noc1=hdr1[3];
    noc2=hdr1[2]-hdr1[3];
    DSSet(f,noc,&ds);
    DSSet(f,noc1,&dssel);
    DSSet(f,noc2,&dsnon);
    d=malloc(100*ds.nob);
    d1=malloc(100*dssel.nob);
    d2=malloc(100*dsnon.nob);
    siz = 8*(2+(noc+63)/64);
    bs=malloc(siz);
    ERData(e1,siz,(uint8 *)bs);
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor;
    hdr1[3]=noc1;
    hdr1[4]=0;
    e3 = EWHdr(argv[3],hdr1);
    hdr1[3]=noc2;
    e4 = EWHdr(argv[4],hdr1);
    r=0;
    while(r!=nor)
    {
        r1=100;
        if(r1>(nor-r)) r1=nor-r;
        ERData(e2,r1*ds.nob,d);
        BSColSelect(f,bs,r1,d,d1,d2);
        EWData(e3,r1*dssel.nob,d1);
        EWData(e4,r1*dsnon.nob,d2);
        r+=r1;
    }
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    EWClose(e4);
    free(f);
    free(d);
    free(d1);
    free(d2);
    free(bs);
    return 0;

}      /******  end of zcx.c    ******/
