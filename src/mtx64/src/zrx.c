/*
      zrx.c     meataxe-64 row extract
      =====     R. A. Parker    6.07.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
#include "pcrit.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3,*e4;
    FIELD * f;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor,noc,nor1,nor2;
    DSPACE ds;
    Dfmt *v;
    uint64_t i,siz;
    uint64_t * bs;

    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 5)
    {
        LogString(80,"usage zrx bs m sel nonsel");
        exit(21);
    }

    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // matrix
    if(hdr1[2]!=hdr2[2])      // bits must equal rows
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr2[1];
    nor=hdr2[2];
    noc=hdr2[3];
    nor1=hdr1[3];
    nor2=nor-nor1;
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor1;
    hdr1[3]=noc;
    hdr1[4]=0;
    e3 = EWHdr(argv[3],hdr1);
    hdr1[2]=nor2;
    e4 = EWHdr(argv[4],hdr1);
    v=malloc(ds.nob);
    siz = 8*(2+(nor+63)/64);
    bs=malloc(siz);
    ERData(e1,siz,(uint8_t *)bs);
    for(i=0;i<nor;i++)
    {
	ERData(e2,ds.nob,v);;
        if(BSBitRead(bs,i)==1) EWData(e3,ds.nob,v);
            else               EWData(e4,ds.nob,v);
    }
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    EWClose(e4);
    free(f);
    free(v);
    free(bs);
    return 0;

}      /******  end of zrx.c    ******/
