// Copyright (C) Richard Parker   2017
// 24.12.2018 after multiple bug fixes
// ftra.c    fTranspose function

#define DISKCHOP 20

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mezz.h"
#include "field.h"
#include "funs.h"
#include "slab.h"
#include "io.h"

void fTranspose(const char *tmp, const char *in, int sin, 
         const char *out, int sout)
{
    FIELD * f;
    EFIL *e1, *e2, *e3;
    EFIL **et;
    uint64_t hdr[5];
    DSPACE ads,bds,tds;
    Dfmt *am,*bm;
    uint64_t i,j,fdef,nora,noca,tsiz,chops;
    uint64_t rowstodo,rowsleft,pcnora;
    char ** tempfn;
    char x[500];

    EPeek(in,hdr);
    if(hdr[0]==3)   // permutation
    {
        fInvert(tmp,in,sin,out,sout);
        return;
    }

    e1=ERHdr(in,hdr);
    fdef=hdr[1];
    nora=hdr[2];
    noca=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noca,&ads);
    DSSet(f,nora,&bds);
    tsiz=ads.nob*nora+bds.nob*noca;
    hdr[2]=noca;
    hdr[3]=nora;
    e3=EWHdr(out,hdr);
    if(((tsiz+tsiz/2)/1000000)<f->megabytes)
    {

        am=malloc(ads.nob*nora);
        bm=malloc(bds.nob*noca);   
        ERData(e1,ads.nob*nora,am);
        SLTra(f,am,bm,nora,noca);
        EWData(e3,bds.nob*noca,bm);
        ERClose1(e1,sin);
        EWClose1(e3,sout);
        free(f);
        free(am);
        free(bm);
        return;
    }
    chops=(tsiz+tsiz/2)/(f->megabytes);
    chops=chops/1000000;
    chops++;
    if(chops<=1) chops=2;    // how many chops do we do?

    pcnora=(nora+chops-1)/chops;    // rows/chops rouded up
    while((pcnora%f->entbyte)!=0) pcnora++;   // integral no of bytes
//  recompute chops - very rarely needed.
    chops=(nora+pcnora-1)/pcnora;
// so (chops-1) of pcnora then the rest
    tempfn=malloc(chops*sizeof(char *));
    for(i=0;i<chops;i++)
    {
        tempfn[i]=malloc(10 + strlen(tmp));
        strcpy(tempfn[i],tmp);
        strcat(tempfn[i],"a");
        sprintf(x,"%03ld",i);
        strcat(tempfn[i],x);
    }
    rowsleft=nora;
    DSSet(f,pcnora,&tds);
    am=malloc(ads.nob*pcnora);   // pcnora rows of input length
    bm=malloc(tds.nob*noca);     // noca rows of pcnora length 
    i=0;
    while(rowsleft!=0)
    {
// read, transpose, write in chunks
        rowstodo=pcnora;
        if(rowstodo>rowsleft)rowstodo=rowsleft;
        DSSet(f,rowstodo,&tds);
        rowsleft-=rowstodo;
        hdr[2]=noca;
        hdr[3]=rowstodo;
        e2=EWHdr(tempfn[i],hdr);  
        ERData(e1,ads.nob*rowstodo,am);
        SLTra(f,am,bm,rowstodo,noca);
        EWData(e2,tds.nob*noca,bm);
        EWClose1(e2,1);
        i++;
    }
    ERClose1(e1,sin);
    et=malloc(chops*sizeof(EFIL *));
    for(i=0;i<chops;i++)
    {
        et[i]=ERHdr(tempfn[i],hdr);
    }
    DSSet(f,pcnora,&tds);
    free(am);
    am=malloc(tds.nob);
    free(bm);
    bm=malloc(bds.nob);
    for(j=0;j<noca;j++)
    {
        memset(bm,0,bds.nob);
        for(i=0;i<chops;i++)
        {
            if((i+1)==chops) DSSet(f,nora-i*pcnora,&tds);
                 else        DSSet(f,pcnora,&tds);
            ERData(et[i],tds.nob,am);
            DPaste(&tds,am,1,i*pcnora,&bds,bm);
        }
        EWData(e3,bds.nob,bm);
    }
    EWClose1(e3,sout);
    for(i=0;i<chops;i++)
    {
        ERClose1(et[i],1);
        remove(tempfn[i]);
        free(tempfn[i]);
    }
    free(tempfn);
    free(et);
    free(f);
    free(am);
    free(bm);
}

/* end of ftra.c  */
