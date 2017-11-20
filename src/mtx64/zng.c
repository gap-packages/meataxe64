/*
      zng.c     meataxe-64 matrix negate V-1 candidate
      =====     R. A. Parker    18.5.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    FIELD * f;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    FELT min1;
    DSPACE ds;
    Dfmt *v1;
    uint64_t i;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zng m ans");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);

    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    min1=FieldNeg(f,1);
    e2 = EWHdr(argv[2],hdr);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
        DSMul(&ds,min1,1,v1);
        EWData(e2,ds.nob,v1);
    }
    free(v1);
    free(f);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zng.c    ******/
