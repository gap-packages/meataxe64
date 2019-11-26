/*
      ztest.c     meataxe-64 program to hack to test stuff
      =======     R. A. Parker    13.2.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"

//#define PEXT 1
#define PASS 1
int main(int argc,  char **argv)
{
    EFIL *e;
    FIELD * f;
    Dfmt *v1,*v2,*v3;
    DSPACE ds,dsp;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    int i;

    e=ERHdr("a1",hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,0);
    DSSet(f,noc,&ds);
    PSSet(f,noc,&dsp);
    v1=malloc(nor*ds.nob);
    v2=malloc(nor*f->pow*dsp.nob);
    v3=malloc(nor*ds.nob);
    ERData(e,nor*ds.nob,v1);
    PExtract(&ds,v1,v2,nor,nor*dsp.nob);
    PAssemble(&ds,v2,v3,nor,nor*dsp.nob);
    i=memcmp(v1,v3,ds.nob*nor);
    if(i!=0) printf("Comparison failure\n");
#ifdef PEXT
    for(i=0;i<1000;i++)
        PExtract(&ds,v1,v2,nor,nor*dsp.nob);
#endif
#ifdef PASS
    for(i=0;i<1000;i++)
        PAssemble(&ds,v2,v3,nor,nor*dsp.nob);
#endif
    return 0;
}


/******  end of ztest.c    ******/
