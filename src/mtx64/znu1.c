/*
      znu1.c     meataxe-64 Simple Null-Space program
      ======     R. A. Parker 04.08.14
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc,i,j;
    uint64_t rank,nullity,col;
    FIELD * f;
    DSPACE ds1,ds2;
    int * piv;
    Dfmt *v1,*v2;
    Dfmt *m1,*m2;
    FELT fel;
    char st[200];

    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage znu <m1> <ns>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds1);
    DSSet(f,nor,&ds2);
    piv=malloc(nor*sizeof(int));
    v1=malloc(ds1.nob);
    v2=malloc(ds2.nob);
    i=nor;
    if(i>noc) i=noc;
    m1=malloc(ds1.nob*i);   // only needs independent ones
    m2=malloc(ds2.nob*nor);
    rank=0;
    nullity=0;
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds1.nob,v1);
        memset(v2,0,ds2.nob);
        DPak(&ds2,i,v2,1);
        for(j=0;j<rank;j++)
        {
            fel=DUnpak(&ds1,piv[j],v1);
            DSMad(&ds1,fel,1,m1+j*ds1.nob,v1);
            DSMad(&ds2,fel,1,m2+j*ds2.nob,v2);
        }
        col=DNzl(&ds1,v1);
        if(col==ZEROROW)
        {
            nullity++;
            DCpy(&ds2,v2,1,m2+(nor-nullity)*ds2.nob);
            continue;
        }
        piv[rank]=col;
        fel=DUnpak(&ds1,col,v1);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(&ds1,fel,1,v1);
        DSMul(&ds2,fel,1,v2);
        DCpy(&ds1,v1,1,m1+rank*ds1.nob);
        DCpy(&ds2,v2,1,m2+rank*ds2.nob);
        rank++;
    }
    ERClose(e1);
    hdr[2]=nullity;
    hdr[3]=nor;
    e2=EWHdr(argv[2],hdr);
    for(i=1;i<=nullity;i++)
        EWData(e2,ds2.nob,m2+(nor-i)*ds2.nob);
    EWClose(e2);

    sprintf(st,"Nullity %ld",(long)nullity);
    printf("%s\n",st);

    LogString(20,st);

    free(f);
    free(piv);
    free(v1);
    free(v2);
    free(m1);
    free(m2);

    return 0;
}

/*  end of znu1.c    */
