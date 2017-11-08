/*
      zrn1.c     meataxe-64 simple Rank progam
      ======     R. A. Parker 02.05.14
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    uint64 fdef,nor,noc,col;
    FIELD * f;
    uint64 hdr[5];
    int rank,i,j;
    DSPACE ds;
    int * piv;
    Dfmt * v1;
    Dfmt * m1;
    FELT fel;
    char st[200];
    
    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage zrn <m1>");
        exit(14);
    }

    e = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(15);
    }
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    piv=malloc(nor*sizeof(int));
    if(piv==NULL)
    {
        LogString(81,"Can't malloc the pivot array");
        exit(19);
    }
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        LogString(81,"Can't malloc a single vector");
        exit(19);
    }
    m1=malloc(ds.nob*nor);
    if(m1==NULL)
    {
        LogString(81,"Can't malloc the space for the matrix");
        exit(19);
    }
    rank=0;
    for(i=0;i<nor;i++)
    {
	ERData(e,ds.nob,v1);
        for(j=0;j<rank;j++)
        {
            fel=DUnpak(&ds,piv[j],v1);
            DSMad(&ds,fel,1,m1+j*ds.nob,v1);
        }
        col=DNzl(&ds,v1);
        if(col==ZEROROW) continue;
        piv[rank]=col;
        fel=DUnpak(&ds,col,v1);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(&ds,fel,1,v1);
        DCpy(&ds,v1,1,m1+rank*ds.nob);
        rank++;
    }

    sprintf(st,"Rank of %s is %d",argv[1],rank);
    printf("%s\n",st);
    ERClose(e);
    LogString(20,st);

    free(f);
    free(piv);
    free(v1);
    free(m1);

    return 0;
}

/*  end of zrn1.c    */
