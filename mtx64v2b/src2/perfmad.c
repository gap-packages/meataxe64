/*
      perfmad.c     meataxe-64 performance test of DSMad
      ======     R. A. Parker    14.9.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    FIELD * f;
    uint64_t fdef,noc,i,gb,lop;
    DSPACE ds;
    Dfmt *v1,*v2;
    FELT k;

    gb=10;

    fdef=strtoul(argv[1],NULL,0);

    f = malloc(FIELDLEN);
    FieldASet(fdef,f);

    DSSet(f,1000,&ds);
    noc=(8000/ds.nob)*1700;
    lop=125000*gb;
    DSSet(f,noc,&ds);

    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    memset(v1,0,ds.nob);
    memset(v2,0,ds.nob);

    for(i=0;i<noc;i++) DPak(&ds,i,v1,i%fdef);
    for(i=0;i<noc;i++) DPak(&ds,i,v2,(3*i+11)%fdef);
    k=0;
    for(i=0;i<lop;i++)
    {
        DSMad(&ds,k,1,v1,v2);
        k++;
        if(k==fdef)k=0;
    }

    return 0;
}

/******  end of perfmad.c    ******/
