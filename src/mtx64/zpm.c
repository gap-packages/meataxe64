/*
      zpm.c     meataxe-64 Permutation to matrix convert.
      =====     R. A. Parker    27.1.2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc,i,j;
    FIELD *f;
    DSPACE ds;
    Dfmt * v1;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zpm p1 field m2");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);
    if( hdr[0]!=3 )
    {
        LogString(80,"Input is not a permutation");
        exit(22);
    }
    fdef=strtoul(argv[2],NULL,0);
    nor=hdr[2];
    noc=hdr[3];
    hdr[0]=1;   // matrix
    hdr[1]=fdef;
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,NOMUL);
    e2 = EWHdr(argv[3],hdr);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    for(i=0;i<nor;i++)
    {
        memset(v1,0,ds.nob);
	ERData(e1,8,(uint8_t *)&j);
        DPak(&ds,j,v1,1);
        EWData(e2,ds.nob,v1);
    }
    free(v1);
    free(f);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zpm.c    ******/
