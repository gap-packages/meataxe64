/*
         gauss.c  -   Low-level Gaussian functions
         =======      R. A. Parker 9.5.2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "bitstring.h"
#include "gauss.h"

GAUSS * GaussCreate(const FIELD * f)
{
    GAUSS * gs1;
    int maxsiz;
    maxsiz=100;
    if(f->madtyp==10) maxsiz=25;
    if(f->madtyp==12) maxsiz=50;
    gs1=malloc(sizeof(GAUSS));
    gs1->maxrows=maxsiz;
    gs1->maxcols=maxsiz;
    gs1->f=f;
// allocate memory for things
//  (again, not very sensitive yet)
    gs1->dsm=malloc(sizeof(DSPACE));
    DSSet(f,gs1->maxcols,gs1->dsm);
    gs1->mtx=malloc(gs1->maxrows*(*(gs1->dsm)).nob);
    gs1->dst=malloc(sizeof(DSPACE));
    DSSet(f,gs1->maxrows,gs1->dst);
    gs1->trf=malloc(gs1->maxrows*(*(gs1->dst)).nob);
    gs1->piv=malloc(gs1->maxrows*sizeof(int16_t));
    gs1->sorted=malloc(gs1->maxrows*sizeof(int16_t));  // maxrank?
    gs1->batch=malloc((gs1->dsm)->nob);
    gs1->battrf=malloc((gs1->dst)->nob);
    return gs1;
}

void GaussDestroy(GAUSS * gs)
{
    free(gs->battrf);
    free(gs->batch);
    free(gs->dsm);
    free(gs->dst);
    free(gs->mtx);
    free(gs->trf);
    free(gs->piv);
    free(gs->sorted);
    free(gs);
}

void GaussStart(GAUSS * gs, uint64_t nor, uint64_t noc, Dfmt * m)
{
    int i;
    gs->nor=nor;
    gs->noc=noc;
    DSSet(gs->f,noc,gs->dsm);
    gs->nobm=(gs->dsm)->nob;
    DSSet(gs->f,nor,gs->dst);
    gs->nobt=(gs->dst)->nob;
    memcpy(gs->mtx,m,gs->nobm*nor);
    memset(gs->trf,0,gs->nobt*nor);
    gs->rank=0;
    gs->phase=1;
    for(i=0;i<nor;i++) gs->piv[i]=-1;
    gs->det=1;
    gs->detsign=0;
}

void GaussBack(GAUSS * gs)
{
    DSPACE ds;
    gs->phase=2;
    DSSet(gs->f,gs->rank,&ds);
    gs->nobrealt=ds.nob;
}

int GaussNewRowFwd(GAUSS * gs, int row)
{
    uint64_t col;
    Dfmt *rptr,*tptr;
    FELT fel;
    rptr=gs->mtx+row*gs->nobm;
    tptr=gs->trf+row*gs->nobt;
    col=DNzl(gs->dsm,rptr);
    if(col==ZEROROW) return 0;
    fel=DUnpak(gs->dsm,col,rptr);
    fel=FieldInv(gs->f,fel);
    fel=FieldNeg(gs->f,fel);
    DSMul(gs->dsm,fel,1,rptr);
    DPak(gs->dst,gs->rank,tptr,1);
    DSMul(gs->dst,fel,1,tptr);
    gs->det=FieldMul(gs->f,gs->det,fel);
    memcpy(gs->batch,rptr,gs->nobm);
    memcpy(gs->battrf,tptr,gs->nobt);
    gs->piv[row]=col;
    gs->batpiv=col;
    gs->rank++;
    return 2;       // batch full
}

int GaussNewRowBack(GAUSS * gs, int row)
{
    Dfmt *rptr,*tptr;
    if(gs->piv[row]==-1) return 0;
    rptr=gs->mtx+row*gs->nobm;
    tptr=gs->trf+row*gs->nobt;
    memcpy(gs->batch,rptr,gs->nobm);
    memcpy(gs->battrf,tptr,gs->nobt);
    gs->batpiv=gs->piv[row];
    return 2;       // batch full
}

void GaussCleanC(GAUSS * gs, int row)
{
    FELT fel;
    fel=DUnpak(gs->dsm,gs->batpiv,gs->mtx+row*gs->nobm);
    DSMad(gs->dsm,fel,1,gs->batch,gs->mtx+row*gs->nobm);
    DSMad(gs->dst,fel,1,gs->battrf,gs->trf+row*gs->nobt);
}

void GaussCleanA(GAUSS * gs, int row)
{
    return;     // no batch => no rows at all
}

void GaussReduce(GAUSS * gs)
{
    return;
}

void GaussMultiplier(GAUSS * gs, Dfmt * mul)
{
    int i;
    for(i=0;i<gs->rank;i++)
        memcpy(mul+i*gs->nobrealt,
             gs->trf+gs->sorted[i]*gs->nobt,gs->nobrealt);
}

void GaussCleaner(GAUSS * gs, Dfmt * cln)
{
    Dfmt * pt;
    int i;
    pt=cln;
    for(i=0;i<gs->nor;i++)
    {
        if(gs->piv[i]!=-1) continue;
        memcpy(pt,gs->trf+i*gs->nobt,gs->nobrealt);
        pt+=gs->nobrealt;
    }
}

void GaussRemnant(GAUSS * gs, Dfmt * rem)
{
    const FIELD * f;
    DSPACE * dsm;
    DSPACE dsr;
    FELT fel;
    int i,j,j1,j2,j3;
    f=gs->f;
    DSSet(f,gs->noc-gs->rank,&dsr);
    memset(rem,0,dsr.nob*gs->rank);
    dsm=gs->dsm;
    j2=0;
    j3=0;
    for(j=0;j<=gs->rank;j++)
    {
        if(j==gs->rank) j1=gs->noc;
        else j1=gs->piv[gs->sorted[j]];;
        while(j2!=j1)
        {
            for(i=0;i<gs->rank;i++)
            {
                fel=DUnpak(dsm,j2,gs->mtx+gs->sorted[i]*gs->nobm);
                DPak(&dsr,j3,rem+i*dsr.nob,fel);
            }
            j3++;
            j2++;
        }
        j2++;
    }
}

void GaussRS(GAUSS * gs, uint64_t * rs)
{
    uint64_t len,i;
    len=8*((gs->nor+63)/64) + 16;
    memset(rs,0,len);
    rs[0]=gs->nor;
    rs[1]=gs->rank;
    for(i=0;i<gs->nor;i++)
        if(gs->piv[i]!=-1) BSBitSet(rs,i);
}

void GaussCS(GAUSS * gs, uint64_t * cs)
{
    uint64_t len,i;
    len=8*((gs->noc+63)/64) + 16;
    memset(cs,0,len);
    cs[0]=gs->noc;
    cs[1]=gs->rank;
    for(i=0;i<gs->nor;i++)
        if(gs->piv[i]!=-1) BSBitSet(cs,gs->piv[i]);
}

void GaussSort(GAUSS * gs)
{
    int16_t temp,i,j;
    int parity;
    j=0;
    parity=0;
    for(i=0;i<gs->nor;i++)
    {
        if(gs->piv[i]!=-1)
        {
            gs->sorted[j++]=i;
            gs->detsign^=parity;
        }
        else
        {
            parity^=1;
        }
    }
// bubble sort for now
    i=0;
    while(1)
    {
        if((i+1)>=gs->rank) break;
        if(gs->piv[gs->sorted[i]]>gs->piv[gs->sorted[i+1]])
        {
            temp=gs->sorted[i];
            gs->sorted[i]=gs->sorted[i+1];
            gs->sorted[i+1]=temp;
            gs->detsign^=1;
            if(i>0) i--;
        }
        else i++;
    }
}

uint64_t BCEch(GAUSS *gs, DSPACE * ds, Dfmt *a, 
               uint64_t *rs, uint64_t *cs, FELT * det,
               Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor, int flags)
{
    long currow,rowx;
    int res;
// initialization
    GaussStart(gs,nor,ds->noc,a);
    currow=0;
// forward clean
    while(currow<nor)
    {
        GaussCleanA(gs,currow);
        res=GaussNewRowFwd(gs,currow);
        currow++;
        if(res!=2) continue;
        for(rowx=currow;rowx<nor;rowx++)
            GaussCleanC(gs,rowx);
        GaussReduce(gs);
    }
    GaussBack(gs);
    GaussRS(gs,rs);
    GaussCS(gs,cs);
    GaussCleaner(gs,c);
    currow=nor-1;
    while(1)
    {
        if(currow==-1) break;
// skip zero rows
// deal with zero rows left
        GaussCleanA(gs,currow);
        res=GaussNewRowBack(gs,currow);  // Should never be zero?
        currow--;
        if(res!=2) continue;
        for(rowx=currow;rowx>=0;rowx--)
            GaussCleanC(gs,rowx);
        GaussReduce(gs); 
    }
    GaussSort(gs);
    if(gs->detsign==0) *det=gs->det;
          else         *det=FieldNeg(gs->f,gs->det);
    GaussMultiplier(gs,m);
    GaussCleaner(gs,c);
    GaussRemnant(gs,r);
    return gs->rank;
}

/* end of gauss.c */
