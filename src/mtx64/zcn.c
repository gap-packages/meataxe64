/*
      zcn.c     meataxe-64 matrix concatenation
      =====     R. A. Parker    22.5.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    int nof,i;
    uint64 fdef,noc,maxnor,norout;
    uint64 hdr[5];
    DSPACE ds;
    EFIL *e1,*e2;
    FIELD *f;
    Dfmt *m;

    LogCmd(argc,argv);
    nof=argc-2;

    EPeek(argv[1],hdr);
    fdef=hdr[1];
    noc=hdr[3];
    norout=hdr[2];
    maxnor=hdr[2];
    for(i=2;i<=nof;i++)
    {
        EPeek(argv[i],hdr);
        if( (fdef!=hdr[1]) || (noc!=hdr[3]) )
        {
            printf("Matrices incompatible\n");
            exit(7);
        }
        norout+=hdr[2];
        if(maxnor<hdr[2]) maxnor=hdr[2];
    }
    hdr[2]=norout;
    e2=EWHdr(argv[nof+1],hdr);
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    m=malloc(ds.nob*maxnor);
    for(i=1;i<=nof;i++)
    {
        e1=ERHdr(argv[i],hdr);
        ERData(e1,ds.nob*hdr[2],m);
        EWData(e2,ds.nob*hdr[2],m);
        ERClose(e1);
    }
    EWClose(e2);
    free(m);
    free(f);
    return 0;
}

/******  end of zcn.c    ******/
