/*
      zir.c     meataxe-64 row riffle
      =====     R. A. Parker    6.07.2014
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
    uint64 hdr1[5],hdr2[5],hdr3[5];
    uint64 fdef,nor,noc,nor1,nor0;
    DSPACE ds;
    Dfmt *v;
    uint64 i,siz;
    uint64 * bs;

    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 5)
    {
        LogString(80,"usage zrr bs m1 m0 m");
        exit(21);
    }

    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // m1
    e3=ERHdr(argv[3],hdr3);   // m0
    fdef=hdr2[1];
    nor1=hdr2[2];
    noc=hdr2[3];
    nor0=hdr3[2];
    nor=nor0+nor1;
    if( (hdr3[1]!=fdef) || (nor1+nor0!=hdr1[2]) || (nor1!=hdr1[3]) )
    {
        printf("Matrices incompatible\n");
        exit(22);
    }
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc,&ds);
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor;
    hdr1[3]=noc;
    hdr1[4]=0;
    e4 = EWHdr(argv[4],hdr1);
    v=malloc(ds.nob);
    siz = 8*(2+(nor+63)/64);
    bs=malloc(siz);
    ERData(e1,siz,(uint8 *)bs);
    for(i=0;i<nor;i++)
    {
	if(BSBitRead(bs,i)==1) ERData(e2,ds.nob,v);
              else             ERData(e3,ds.nob,v);
        EWData(e4,ds.nob,v);
    }
    ERClose(e1);
    ERClose(e2);
    ERClose(e3);
    EWClose(e4);
    free(f);
    free(v);
    free(bs);
    return 0;

}      /******  end of zrr.c    ******/
