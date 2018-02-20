/*
      zfr1.c     meataxe-64 Frobenius automorphism
      ======     R. A. Parker    4.8.2016
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
    FELT f1,f2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    FIELD * f;
    DSPACE ds;
    Dfmt *v1,*v2;
    uint64_t i,j,k;


    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zfr m1 m2");
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
    e2 = EWHdr(argv[2],hdr);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);

/******  Do it one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
        memset(v2,0,ds.nob);
        for(j=0;j<noc;j++)
        {
            f1=DUnpak(&ds,j,v1);
            f2=1;
            for(k=0;k<f->charc;k++)
                f2=FieldMul(f,f1,f2);
            DPak(&ds,j,v2,f2);
        }
        EWData(e2,ds.nob,v2);
    }
    free(v1);
    free(v2);
    free(f);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zfr1.c    ******/
