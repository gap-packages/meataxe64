/*
         dfmtf.c  -   Dfmt functions
         ======      R. A. Parker 19.2.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"
#include "slab.h"
#include "linf.h"
#include "pmul.h"
#include "bitstring.h"
#include "dfmtf.h"

// New Dfmt multiplication

void DFMul(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c)
{
    memset(c,0,nora*dsbc->nob);
    DFMad(dsa,dsbc,nora,a,astride,b,bstride,c,dsbc->nob);
}

void DFMad(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, uint64_t cstride)
{
    const Dfmt *da, *db;
    Dfmt *dc;
    Dfmt *gt;
    uint64_t i,j,fdef,slot;
    const FIELD * f;
    FELT e,e2;

    f=dsa->f;
    fdef=f->fdef;
    if(dsa->ground!=0) fdef=f->charc;

    j=0;
    db=(Dfmt *) b;
    gt=NULL;
    if(nora>fdef*2)
    {
        gt=malloc(dsbc->nob*(fdef+1));

        while((j+1)<dsa->noc)
        {
            da=a;
            dc=c;
            memcpy(gt,db,dsbc->nob);
            memcpy(gt+fdef*dsbc->nob,db+bstride,dsbc->nob);
            for(i=1;i<fdef;i++)
            {
                memcpy(gt+i*dsbc->nob,db,dsbc->nob);
                DSMad(dsbc,i,1,gt+fdef*dsbc->nob,gt+i*dsbc->nob);
            }
            for(i=0;i<nora;i++)
            {
                e=DUnpak(dsa,j,da);
                e2=DUnpak(dsa,j+1,da);
                if(e==0)
                {
                    slot=fdef;
                    e=e2;
                }
                else
                {
                    slot=FieldDiv(f,e2,e);
                }
                DSMad(dsbc,e,1,gt+slot*dsbc->nob,dc);
                da+=astride;
                dc+=cstride;
            }
            db+=2*bstride;
            j+=2;
        }
    }

// tidy up code at the end when no full slice left

    while(j<dsa->noc)
    {
        da=a;
        dc=c;
        for(i=0;i<nora;i++)
        {
            e=DUnpak(dsa,j,da);
            DSMad(dsbc,e,1,db,dc);
            da+=astride;
            dc+=cstride;
        }
        db+=bstride;
        j++;
    }
    if(gt!=NULL) free(gt);
}

/* end of dfmtf.c  */
