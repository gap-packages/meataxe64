/*
      zcs.c     meataxe-64 characteristic polynomial on sparsified matrix
      =====     R. A. Parker 17.2.19
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "io.h"

typedef struct
{
    EFIL * e;
    DSPACE * ds;
    uint64_t fdef;
    uint64_t nor;
    uint64_t noc;
    uint64_t spblk;
    uint64_t deblk;
    uint64_t * spsc;
    uint64_t * splen;
    uint64_t * spdest;
    uint64_t * desc;
    uint64_t * delen;
    uint64_t denrows;
    Dfmt * m;
} SpMtx;

void SpMulSingle(SpMtx * spm, Dfmt * vec, Dfmt * res, Dfmt * cpb)
{
    DSPACE *ds, dsc;
    const FIELD * f;
    uint64_t i,j,k;
    FELT fel;

    ds=spm->ds;
    f=ds->f;
    memset(res,0,ds->nob);
    for(i=0;i<spm->spblk;i++)
    {
        DSSet(f,spm->splen[i],&dsc);
        memset(cpb,0,dsc.nob);
        DCut(ds,1,spm->spsc[i],vec,&dsc,cpb);
        DPaste(&dsc,cpb,1,spm->spdest[i],ds,res);
    }
    k=0;
    for(i=0;i<spm->deblk;i++)
    {
        for(j=0;j<spm->delen[i];j++)
        {
            fel=DUnpak(ds,spm->desc[i]+j,vec);
            DSMad(ds,fel,1, spm->m + k*ds->nob ,res);
            k++;
        }
    }
}

SpMtx * SpRead1(char * fn)
{
    EFIL * e;
    uint64_t hdr[5];
    SpMtx * spm;
    uint64_t i;
    e = ERHdr(fn,hdr);
    if(hdr[0]!=5)
    {
        printf("Input must be in sparsified format\n");
        exit(44);
    }
    spm=malloc(sizeof(SpMtx));
    spm->e = e;
    spm->fdef=hdr[1];
    spm->nor=hdr[2];
    spm->noc=hdr[3];
    ERData(e,8,(uint8_t *)&spm->spblk);
    ERData(e,8,(uint8_t *)&spm->deblk);
    spm->spsc=malloc(8*spm->spblk);
    spm->splen=malloc(8*spm->spblk);
    spm->spdest=malloc(8*spm->spblk);
    spm->desc=malloc(8*spm->deblk);
    spm->delen=malloc(8*spm->deblk);
    for(i=0;i<spm->spblk;i++)
    {
        ERData(e,8,(uint8_t *)(spm->spsc+i));
        ERData(e,8,(uint8_t *)(spm->splen+i));
        ERData(e,8,(uint8_t *)(spm->spdest+i));
    }
    for(i=0;i<spm->deblk;i++)
    {
        ERData(e,8,(uint8_t *)(spm->desc+i));
        ERData(e,8,(uint8_t *)(spm->delen+i));
    }
    return spm;
}

void SpRead2(SpMtx * spm)
{
    uint64_t i;
    spm->denrows=0;
    for(i=0;i<spm->deblk;i++)
        spm->denrows+=spm->delen[i];
    spm->m=malloc(((spm->ds)->nob)*spm->denrows);
    ERData(spm->e,((spm->ds)->nob)*spm->denrows,spm->m); // read input matrix
    ERClose(spm->e);
}

int main(int argc,  char **argv)
{
    SpMtx * spm;
    uint64_t hdr[5];
    FIELD * f;
    DSPACE ds,dsp;
    uint64_t * polystart;
    uint64_t * polydeg;
    uint64_t dim,gvcnt,oldgv,i,stpiv,col;
    Dfmt * polys;
    Dfmt *v1,*v2, *v3, *cpb;  // working vectors
    uint64_t * piv;  // pivot array row->piv
    char * hadpiv;
    FELT fel;
    uint64_t polix;
    uint64_t nopolys;
    Dfmt * gv;  // Gaussian vectors
    Dfmt * tm;  // transformation matrix
    EFIL * e;


    LogCmd(argc,argv);
    if( (argc != 3) && (argc != 4) ) 
    {
        LogString(80,"usage zcs <sp-mtx> <poly> <opt vecs>");
        exit(14);
    }
    spm=SpRead1(argv[1]);
    if (spm->nor != spm->noc) 
    {
        LogString(80,"Only square matrices have a characteristic poly");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(spm->fdef,f);
    dim=spm->nor;
    DSSet(f,spm->noc,&ds);
    spm->ds=&ds;             // don't touch field or ds now!
    SpRead2(spm);
    DSSet(f,2,&dsp);      // degree 1 polynomial
    polys=malloc(dsp.nob*spm->nor);   // maximum size of polynomial list
    polystart=malloc(spm->nor*sizeof(uint64_t));  // index of first byte
    polydeg=malloc(spm->nor*sizeof(uint64_t));   // degrees
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    v3=malloc(ds.nob);
    cpb=malloc(ds.nob);
    gv=malloc(ds.nob*dim);
    tm=malloc(ds.nob*dim);
    piv=malloc(dim*sizeof(uint64_t));
    hadpiv=malloc(dim);
    gvcnt=0;
    for(i=0;i<dim;i++) hadpiv[i]=0;
    stpiv=0;
    nopolys=0;
    polix=0;

    while(gvcnt<dim)
    {
        oldgv=gvcnt;
        while(hadpiv[stpiv]!=0)stpiv++;   // make stpiv a non-pivot
        memset(v1,0,ds.nob);
        DPak(&ds,stpiv,v1,1);
        while(1)
        {
            memset(v2,0,ds.nob);
            DCpy(&ds,v1,1,v3);
            for(i=0;i<gvcnt;i++)    // clean new vector with old one
            {
                fel=DUnpak(&ds,piv[i],v1);
                DSMad(&ds,fel,1,gv+i*ds.nob,v1);
                DSMad(&ds,fel,1,tm+i*ds.nob,v2);
            }
//  if it isn't zero, include it in m1 and m2
            col=DNzl(&ds,v1);
            if(col==ZEROROW) break;
            piv[gvcnt]=col;
            hadpiv[col]=1;
            fel=DUnpak(&ds,col,v1);
            fel=FieldInv(f,fel);
            fel=FieldNeg(f,fel);
            DPak(&ds,gvcnt,v2,1);
            DSMul(&ds,fel,1,v1);
            DSMul(&ds,fel,1,v2);
            DCpy(&ds,v1,1,gv+gvcnt*ds.nob);
            DCpy(&ds,v2,1,tm+gvcnt*ds.nob);
            gvcnt++;
            SpMulSingle(spm,v3,v1,cpb);    // multiply by X
        }
        DSSet(f,gvcnt-oldgv+1,&dsp);
        polystart[nopolys]=polix; 
        memset(polys+polix,0,dsp.nob);
        DCut(&ds,1,oldgv,v2,&dsp,polys+polix);
        DPak(&dsp,gvcnt-oldgv,polys+polix,1);
        polydeg[nopolys]=gvcnt-oldgv;
        polix+=dsp.nob;
        nopolys++;
    }
    hdr[0]=4;   // polynomial
    hdr[1]=spm->fdef;
    hdr[2]=dim;
    hdr[3]=nopolys;
    hdr[4]=0;  // not used
    e=EWHdr(argv[2],hdr);
    for(i=0;i<nopolys;i++)
    {
        EWData(e,8,(uint8_t *)(polydeg+i));
        DSSet(f,polydeg[i]+1,&dsp);
        EWData(e,dsp.nob,polys+polystart[i]);
    }
    EWClose(e);
    free(polys);
    free(polystart);
    free(polydeg);
    free(f);
    free(v1);
    free(v2);
    free(v3);
    free(cpb);
    free(gv);
    free(tm);

    return 0;
}
/*  end of zcs.c    */
