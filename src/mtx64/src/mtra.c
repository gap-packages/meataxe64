/*
              mtra.c     Mezzanine Transpose of one file
              ======     R. A. Parker   1.16.2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "slab.h"
#include "io.h"

int mtra(const char *in, int sin, const char *out, int sout)
{
    FIELD * f;
    EFIL *e1, *e2;
    uint64_t hdr[5];
    DSPACE ads,bds;
    Dfmt *am,*bm;
    uint64_t fdef,nora,noca;
    e1=ERHdr(in,hdr);
    fdef=hdr[1];
    nora=hdr[2];
    noca=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    hdr[2]=noca;
    hdr[3]=nora;
    e2=EWHdr(out,hdr);
    DSSet(f,noca,&ads);
    DSSet(f,nora,&bds);
    am=malloc(ads.nob*nora);
    bm=malloc(bds.nob*noca);   
    ERData(e1,ads.nob*nora,am);
    SLTra(f,am,bm,nora,noca);
    EWData(e2,bds.nob*noca,bm);
    ERClose1(e1,sin);
    EWClose1(e2,sout);
    free(f);
    free(am);
    free(bm);
    return 0;
}

/* end of mtra.c  */
