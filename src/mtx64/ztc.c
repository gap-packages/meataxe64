/*
      ztc.c     meataxe-64 V2.0 Trace progam
      =====     R. A. Parker 29.12.16
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    uint64 fdef,nor,noc;
    FIELD * f;
    Dfmt * v1;
    DSPACE ds;
    FELT fel,fel1;
    uint64 hdr[5];
    int i;
    char st[200];
    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage ztc <m1>");
        exit(14);
    }

    e = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    if(nor!=noc)
    {
        LogString(80,"Only square matrices have a trace");
        exit(30);
    }
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(15);
    }
    FieldASet1(fdef,f,NOMUL);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        LogString(81,"Can't malloc a single vector");
        exit(19);
    }
    fel=0;
    for(i=0;i<nor;i++)
    {
	ERData(e,ds.nob,v1);
        fel1=DUnpak(&ds,i,v1);
        fel=FieldAdd(f,fel,fel1);
    }
    sprintf(st,"Trace of %s is %ld",argv[1],fel);
    printf("%s\n",st);
    ERClose(e);
    LogString(20,st);
    free(f);
    free(v1);
    return 0;
}

/*  end of ztc.c    */
