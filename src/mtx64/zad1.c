/*
      zad1.c     meataxe-64 Simple matrix addition.
      ======     R. A. Parker    29.12.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3;
    FIELD * f;
    uint64 hdr1[5],hdr2[5];
    uint64 fdef,nor,noc;
    DSPACE ds;
    Dfmt *v1,*v2;
    uint64 i;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zad m1 m2 ans");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);
    e2=ERHdr(argv[2],hdr2);

    if( (hdr1[1]!=hdr2[1]) || (hdr1[2]!=hdr2[2]) || (hdr1[3]!=hdr2[3]) )
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr1[1];
    nor=hdr1[2];
    noc=hdr1[3];

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet1(fdef,f,NOMUL);
    e3 = EWHdr(argv[3],hdr1);
    DSSet(f,noc,&ds);

    v1=malloc(ds.nob);
    v2=malloc(ds.nob);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
	ERData(e2,ds.nob,v2);
        DAdd(&ds,1,v1,v2,v2);
        EWData(e3,ds.nob,v2);
    }
    free(v1);
    free(v2);
    free(f);
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    return 0;
}

/******  end of zad1.c    ******/
