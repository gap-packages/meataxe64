/*
              ztr2.c     Slab based transpose
              ======     R. A. Parker   21.5.2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "slab.h"
#include "io.h"

int main(int argc, char **argv)
{
    FIELD * f;
    EFIL *e1, *e2;
    uint64 hdr[5];
    DSPACE ads,bds;
    Dfmt *am,*bm;
    uint64 fdef,nora,noca;
    LogCmd(argc,argv);
    if (argc != 3)
    {
        LogString(80,"usage ztr m1 ans");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nora=hdr[2];
    noca=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    hdr[2]=noca;
    hdr[3]=nora;
    e2=EWHdr(argv[2],hdr);
    DSSet(f,noca,&ads);
    DSSet(f,nora,&bds);
    am=malloc(ads.nob*nora);
    bm=malloc(bds.nob*noca);   
    ERData(e1,ads.nob*nora,am);
    SLTra(f,am,bm,nora,noca);
    EWData(e2,bds.nob*noca,bm);
    ERClose(e1);
    EWClose(e2);
    free(f);
    free(am);
    free(bm);
    return 0;
}

/* end of ztr2.c  */
