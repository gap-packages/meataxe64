/*
      zss.c     meataxe-64 Subtract Scalar
      =====     R. A. Parker    24.5.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    FELT k1,k2;
    FIELD * f;
    uint64 hdr[5];
    uint64 fdef,nor;
    DSPACE ds;
    Dfmt *v;
    uint64 i;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zss mx scalar ans");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);

    if(hdr[2]!=hdr[3])
    {
        LogString(80,"Matrix not square");
        exit(22);
    }
    fdef=hdr[1];
    nor=hdr[2];

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    e2 = EWHdr(argv[3],hdr);
    DSSet(f,nor,&ds);

    v=malloc(ds.nob);

    k1=atoi(argv[2]);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v);
        k2=DUnpak(&ds,i,v);
        k2=FieldSub(f,k2,k1);
        DPak(&ds,i,v,k2);
        EWData(e2,ds.nob,v);
    }
    free(v);
    free(f);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zss.c    ******/
