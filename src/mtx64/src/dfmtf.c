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

// Base case echelize - under redevelopment

uint64_t DEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    uint64_t *piv, *prow;
    uint8_t * sta;
    uint64_t ppiv[20],pprow[20];
    FELT fel,fel0,fel1;
    uint64_t nck,rank,i,j,pp,z,col,col0,col1,col2;
    uint64_t scheme,batch,npool;
    size_t sbsr, sbsc;
    FELT deter;
    DSPACE dsk,dsks,dsm,dsr;
    Dfmt *vo, *junk;
    Dfmt *k,*gr,*grk;
    Dfmt *va,*vk,*wa,*wk,*va0,*va1,*va2,*vk0,*vk1,*vk2;
    const FIELD * f;

    f=ds->f;
    deter=1;
    scheme=1;
    if((f->fdef*2)<nor)  scheme=2;  // projective grease 2 worth doing
    piv=malloc(nor*sizeof(uint64_t));
    sta=malloc(nor*sizeof(uint8_t));   // 0 not yet, 1 pool, 2 done
    prow=malloc(ds->noc*sizeof(uint64_t));
    for(i=0;i<nor;i++) piv[i]=ZEROROW;
    for(i=0;i<nor;i++) sta[i]=0;       // not yet pools
    for(i=0;i<ds->noc;i++) prow[i]=ZEROROW;

    vo=malloc(ds->nob);
    junk=malloc(ds->nob);    // for the non-selected columns
    nck=nor;
    if(nck>ds->noc) nck=ds->noc;
    DSSet(f,nck,&dsk);
    if(scheme==2)
    {
        gr=malloc((f->fdef+1)*ds->nob);
        grk=malloc((f->fdef+1)*dsk.nob);
        memset(grk,0,(f->fdef+1)*dsk.nob);   // so short dsks can be used
    }
    k=malloc(nor*dsk.nob);
    memset(m,0,nck*dsk.nob);
    memset(k,0,nor*dsk.nob);   // keeptrack starts as zero

    sbsr=8*(2+(nor+63)/64);
    memset(rs,0,sbsr);
    rs[0]=nor;
    sbsc=8*(2+(ds->noc+63)/64);
    memset(cs,0,sbsc);
    cs[0]=ds->noc;

    rank=0;
    DSSet(f,rank,&dsks);

    npool=0;
    for(i=0;i<nor;i++)
    {
        va=a+i*ds->nob;    // new vector
        vk=k+i*dsk.nob;    // new vector in keeptrack
        for(j=0;j<npool;j++)   // clean new vector with old pool
        {
            wa=a+pprow[j]*ds->nob;
            wk=k+pprow[j]*dsk.nob;
            col=ppiv[j];
            fel=DUnpak(ds,col,va);
            DSMad(ds,fel,1,wa,va);
            DSMad(&dsks,fel,1,wk,vk);
        }
        col=DNzl(ds,va);   // is new vector pivotal?
        if(col==ZEROROW)
        {
            sta[i]=2;
            continue;
        }
        sta[i]=1;
        pprow[npool]=i;
        ppiv[npool]=col;
// need to sort these before MEX

// process the new pivotal row/column
        piv[i]=col;
        prow[col]=i;
        fel=DUnpak(ds,col,va);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(ds,fel,1,va);
        DPak(&dsk,rank,vk,1);
        rank++;
        DSSet(f,rank,&dsks);
        DSMul(&dsks,fel,1,vk);
        deter=FieldMul(f,fel,deter);
        BSBitSet(rs,i);
        BSBitSet(cs,col);
// back-clean old pool with new vector
        for(j=0;j<npool;j++) 
        {
            wa=a+pprow[j]*ds->nob;
            wk=k+pprow[j]*dsk.nob;
            fel=DUnpak(ds,col,wa);
            DSMad(ds,fel,1,va,wa);
            DSMad(&dsks,fel,1,vk,wk);
        }
        npool++;
        batch=0;
        switch (scheme)
        {
          case 1:     // simple method, but clear two at once
            if(npool<3) continue;
            col0=ppiv[0];
            col1=ppiv[1];
            col2=ppiv[2];
            va0=a+pprow[0]*ds->nob;
            vk0=k+pprow[0]*dsk.nob;
            va1=a+pprow[1]*ds->nob;
            vk1=k+pprow[1]*dsk.nob;
            va2=a+pprow[2]*ds->nob;
            vk2=k+pprow[2]*dsk.nob;
            for(j=0;j<nor;j++)
            {
                if(sta[j]==1) continue;
                fel=DUnpak(ds,col0,a+j*ds->nob);
                DSMad(ds,fel,1,va0,a+j*ds->nob);
                DSMad(&dsks,fel,1,vk0,k+j*dsk.nob);
                fel=DUnpak(ds,col1,a+j*ds->nob);
                DSMad(ds,fel,1,va1,a+j*ds->nob);
                DSMad(&dsks,fel,1,vk1,k+j*dsk.nob);
                fel=DUnpak(ds,col2,a+j*ds->nob);
                DSMad(ds,fel,1,va2,a+j*ds->nob);
                DSMad(&dsks,fel,1,vk2,k+j*dsk.nob);
            }
            batch=3;
            break;
          case 2:   // projective grease level 2
            if(npool<2) continue;
            col0=ppiv[0];
            col1=ppiv[1];
            va0=a+pprow[0]*ds->nob;
            vk0=k+pprow[0]*dsk.nob;
            va1=a+pprow[1]*ds->nob;
            vk1=k+pprow[1]*dsk.nob;
// prepare grease table
            memcpy(gr,va0,ds->nob);
            memcpy(grk,vk0,dsk.nob);
            memcpy(gr+f->fdef*ds->nob,va1,ds->nob);
            memcpy(grk+f->fdef*dsk.nob,vk1,dsk.nob);
            for(j=1;j<f->fdef;j++)
            {
                memcpy(gr+j*ds->nob,gr,ds->nob);
                memcpy(grk+j*dsk.nob,grk,dsk.nob);
                DSMad(ds,j,1,va1,gr+j*ds->nob);
                DSMad(&dsks,j,1,vk1,grk+j*dsk.nob);
            }
// use it
            for(j=0;j<nor;j++)
            {
                if(sta[j]==1) continue;
                fel0=DUnpak(ds,col0,a+j*ds->nob);
                fel1=DUnpak(ds,col1,a+j*ds->nob);
                if(fel0==0)
                {
                    va=gr+f->fdef*ds->nob;
                    vk=grk+f->fdef*dsk.nob;
                    fel0=fel1;
                }
                else
                {
                    fel=FieldInv(f,fel0);
                    fel=FieldMul(f,fel,fel1);
                    va=gr+fel*ds->nob;
                    vk=grk+fel*dsk.nob;
                }

                DSMad(ds,fel0,1,va,a+j*ds->nob);
                DSMad(&dsks,fel0,1,vk,k+j*dsk.nob);
            }
            batch=2;
            break;
          default:
            printf("No scheme set in echelonize\n");
            exit(22);
        }
// take batch out of pool
        if(batch!=0)
        {
            for(j=0;j<batch;j++) sta[pprow[j]]=2;
            for(j=0;j<npool-batch;j++)
            {
                pprow[j]=pprow[j+batch];
                ppiv[j]=ppiv[j+batch];
            }
            npool=npool-batch;
        }
    }
// clear out matrix with rest of pool
    for(pp=0;pp<npool;pp++)
    {
        col=ppiv[pp];
        va=a+pprow[pp]*ds->nob;
        vk=k+pprow[pp]*dsk.nob;
        for(j=0;j<nor;j++)
        {
            if(sta[j]==1) continue;
            fel=DUnpak(ds,col,a+j*ds->nob);
            DSMad(ds,fel,1,va,a+j*ds->nob);
            DSMad(&dsks,fel,1,vk,k+j*dsk.nob);
        }
    }

    rs[1]=rank;
    cs[1]=rank;

    DSSet(f,rank,&dsm);    // first get the cleaner out
    memset(c,0,(nor-rank)*dsm.nob);
    z=0;
    for(j=0;j<nor;j++)
    {
        if(BSBitRead(rs,j)==1) continue;
        DCut(&dsk,1,0,k+j*dsk.nob,&dsm,c+z*dsm.nob);
        z++;     
    }
    z=0;                       // now get multiplier and remnant out
    DSSet(f,ds->noc-rank,&dsr);
    memset(r,0,rank*dsr.nob);
    for(i=0;i<ds->noc;i++)     // by permuting the rows
    {
        col=prow[i];
        if(col==ZEROROW) continue;
        DCut(&dsk,1,0,k+col*dsk.nob,&dsm,m+z*dsm.nob);
        BSColSelect(f,cs,1,a+col*ds->nob,junk,r+z*dsr.nob);
        z++;
    }

    free(piv);
    free(prow);
    free(vo);
    free(junk);
    free(k);
    free(sta);
    if(scheme==2)
    {
        free(gr);
        free(grk);
    }
    *det=deter;
    return rank;

}

/* end of dfmtf.c  */
