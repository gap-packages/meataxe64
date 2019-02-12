/*
      zsc.c     meataxe-64 Scalar linear combinations
      =====     R. A. Parker    22.12.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2, *e3;
    uint64_t hdr[5];
    FIELD * f;
    uint64_t fdef,nor,noc,i;
    FELT s1,s2,k;
    DSPACE ds;
    Dfmt *v,*v1;
    unsigned long long ll;

    e3=NULL;  s2=0;    // avoid compiler warnings
    LogCmd(argc,argv);
    if((argc!=4)&&(argc!=6))
    {
        printf("usage zsc X s1 Y s2 Z for Z=X+s1.Y-s2\n");
        exit(31);
    }
    e1=ERHdr(argv[1],hdr);
    if(hdr[2]!=hdr[3])
    {
        LogString(80,"Matrix not square");
        exit(22);
    }
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    if(argc==6)
    {
        e2=ERHdr(argv[3],hdr);
        if( (hdr[1]!=fdef) || (hdr[2]!=nor) || (hdr[3]!=noc)  )
        {
            LogString(80,"zsc - Matrices incompatible");
            exit(23);
        }
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,nor,&ds);
    v=malloc(ds.nob);
    if(argc==4)
    {
        ll=strtoull(argv[2],NULL,10);
        s2=ll;
        s2=FieldNeg(f,s2);
        e3 = EWHdr(argv[3],hdr);
    }
    if(argc==6)
    {
        e3 = EWHdr(argv[5],hdr);
        v1=malloc(ds.nob);
        ll=strtoull(argv[2],NULL,10);
        s1=ll;
        ll=strtoull(argv[4],NULL,10);
        s2=ll;
        s2=FieldNeg(f,s2);
    }

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
        ERData(e1,ds.nob,v);
        if(argc==6)
        {
            ERData(e2,ds.nob,v1);
            DSMad(&ds,s1,1,v1,v);
        }
        k=DUnpak(&ds,i,v);
        k=FieldAdd(f,k,s2);
        DPak(&ds,i,v,k);
        EWData(e3,ds.nob,v);
    }
    free(v);
    free(f);
    ERClose(e1);
    EWClose(e3);
    if(argc==6)
    {
        free(v1);
        ERClose(e2);
    }
    return 0;
}

/******  end of zsc.c    ******/
