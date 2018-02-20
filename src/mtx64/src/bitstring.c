// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// bitstring.c  -   Bit String Operations

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "bitstring.h"
#include "pcrit.h"

int  BSBitRead (const uint64_t *bs, uint64_t bitno)
{
    uint64_t ix,val,sh;
    ix=2+(bitno/64);
    val=1;
    sh=bitno%64;
    val=(val<<sh);
    if ( (bs[ix]&val)==0 ) return 0;
    return 1;
}

void BSBitSet (uint64_t * bs, uint64_t bitno)
{
    uint64_t ix,val,sh;
    ix=2+(bitno/64);
    val=1;
    sh=bitno%64;
    val=(val<<sh);
    bs[ix] |= val;
    return;
}

void BSColSelect (const FIELD * f, const uint64_t * bs, uint64_t nor,
                  const Dfmt * d, Dfmt * sel, Dfmt * nonsel)
{
    uint64_t noc,noc1,noc0;
    uint64_t maxnor,maxnor0,maxnor1;
    DSPACE ds,ds1,ds0,dscb;
    Dfmt * cb;
    Dfmt *selptr,*nonselptr;
    const Dfmt * dptr;
    uint32_t *gpc,*gptr;
    uint64_t dorows,rowsleft;
    uint32_t bit,out0,out1,cols,incol;

// no rows?  nothing to do
    if(nor==0) return;
    noc=bs[0];
    noc1=bs[1];
    noc0=noc-noc1;
    DSSet(f,noc,&ds);  //  in space
    DSSet(f,noc1,&ds1);  // sel out space
    DSSet(f,noc0,&ds0);  // nonsel out space
// special case 1 all bits set
    if(noc0==0)
    {
        memcpy(sel,d,nor*ds.nob);
        return;
    }
// special case 0 all bits unset
    if(noc1==0)
    {
        memcpy(nonsel,d,nor*ds.nob);
        return;
    }
    maxnor=pcstride(ds.nob);  // compute maxnor for cache-friendly
    maxnor1=pcstride(ds1.nob);
    maxnor0=pcstride(ds0.nob);
    if(maxnor1<maxnor) maxnor=maxnor1;
    if(maxnor0<maxnor) maxnor=maxnor0;
    cb=malloc(maxnor*ds.nob);
    gpc=BSGpc(bs);  // convert bs to grouped columns format
    rowsleft=nor;
    dptr=d;
    selptr=sel;
    nonselptr=nonsel;
    while(rowsleft!=0)
    {
        dorows=maxnor;
        if(dorows>rowsleft) dorows=rowsleft;
        memset(nonselptr,0,ds0.nob*dorows);
        memset(selptr,0,ds1.nob*dorows);
        gptr=gpc+1;
        bit=gpc[0];
        out1=0;
        out0=0;
        while(gptr[0]!=ENDGPC)
        {
            cols=gptr[1];
            incol=gptr[0];
            DSSet(f,cols,&dscb);
            DCut(&ds,dorows,incol,dptr,&dscb,cb);
            if(bit==1)
            {
                DPaste(&dscb,cb,dorows,out1,&ds1,selptr);
                out1+=cols;
            }
            else
            {
                DPaste(&dscb,cb,dorows,out0,&ds0,nonselptr);
                out0+=cols;
            }
            bit^=1;
            gptr+=2;
        }
        rowsleft-=dorows;
        dptr+=ds.nob*dorows;
        selptr+=ds1.nob*dorows;
        nonselptr+=ds0.nob*dorows;
    }
    free(cb);
    free(gpc);
}

// list indices of unset bits

uint32_t * BSLixUn(const uint64_t * bs)
{
    uint32_t i,j;
    uint32_t * ix;

    ix=malloc((bs[0]-bs[1])*sizeof(uint32_t));
    j=0;
    for(i=0;i<bs[0];i++)
    {
        if(BSBitRead(bs,i)==0)
        {
            ix[j]=i;
            j++;
        }
    }
    return ix;
}

// String starts 0 or 1 giving first bit, then pairs 
// (startpos, length) for alternating bits including last
// then three copies of ENDGPC 0xffffffff

uint32_t * BSGpc(const uint64_t * bs)
{
    uint32_t *gpc,*gptr;
    uint32_t phase;
    uint32_t bitno,bitct;
    uint64_t maxgps0,maxgps;
    maxgps=bs[1];      // groups of 1
    maxgps0=(bs[0]-bs[1]);    // groups of 0 
    if(maxgps0<maxgps) maxgps=maxgps0;
/* phase, 3 terminators, 4 for each group and one extra half-group */
    gpc=malloc((6+4*maxgps)*sizeof(uint32_t));
    if(bs[0]==0)   // no bits at all
    {
        gpc[0]=0;   // just for syntax
        gpc[1]=ENDGPC;
        gpc[2]=ENDGPC;
        gpc[3]=ENDGPC;
        return gpc;
    }
    gptr=gpc;
    phase=BSBitRead(bs,0);
    *(gptr++)=phase;
    bitno=0;
    bitct=0;         // avoid compiler warnings
// maybe later fastpath if bs[0]==bs[1] or bs[1]=0
    while(bitno<bs[0])
    {
        if(BSBitRead(bs,bitno)==phase)
        {
            bitct++;
            bitno++;
            continue;
        }
        *(gptr++)=bitno-bitct;
        *(gptr++)=bitct;
        bitct=0;
        phase^=1;
    }
    *(gptr++)=bitno-bitct;
    *(gptr++)=bitct;
    *(gptr++)=ENDGPC; 
    *(gptr++)=ENDGPC; 
    *(gptr++)=ENDGPC; 
    return gpc;  
}

void BSColRifZ (const FIELD * f, const uint64_t * bs, uint64_t nor,
                  const Dfmt * mtxin, Dfmt * mtxout)
{
    uint64_t noc1,noc2;
    uint64_t maxnor,maxnor2,maxnoc;
    uint64_t dorows,rowsleft;
    uint64_t outcol,inpos,cols;
    DSPACE ds1,ds2,ds3;
    uint32_t *gpc,*gptr;
    Dfmt * cb;

//  Special cases with no real work needed

    if(nor==0) return;    // zero rows in output
    noc2=bs[0];           // total bits = noc(output)
    if(noc2==0) return;   // zero columns in output
    noc1=bs[1];           // set bits   = noc(input)
    DSSet(f,noc1,&ds1);   // input space
    if(noc1==noc2)        // no unset bits . . .
    {                     // just copy input to output
        memcpy(mtxout,mtxin,ds1.nob*nor);
        return;
    }
    DSSet(f,noc2,&ds2);   // output space
    memset(mtxout,0,ds2.nob*nor);  // clear output
    if(noc1==0) return;   // no set bits

    gpc=BSGpc(bs);  // convert bs to grouped columns format
    maxnor=pcstride(ds1.nob);  // compute maxnor for cache-friendly
    maxnor2=pcstride(ds2.nob);
    if(maxnor2<maxnor) maxnor=maxnor2;    // maxnor is at most 200
    maxnoc=noc2;                          // maxnoc is greater of
    if((noc2-noc1)>maxnoc) maxnoc=(noc2-noc1);  // #(1-bits), #(0-bits)
    DSSet(f,maxnoc,&ds3);
    cb=(Dfmt *) malloc(ds3.nob*maxnor);  // allocate clipboard
    rowsleft=nor;
    while(rowsleft!=0)
    {
        dorows=maxnor;
        if(dorows>rowsleft) dorows=rowsleft;
        gptr=gpc+1;
        inpos=0;
        if(gpc[0]==0)
            gptr+=2;  // only interested in 1-bits 
        while(gptr[0]!=ENDGPC)
        {
            cols=gptr[1];
            outcol=gptr[0];
            DSSet(f,cols,&ds3);
            DCut(&ds1,dorows,inpos,mtxin,&ds3,cb);
            DPaste(&ds3,cb,dorows,outcol,&ds2,mtxout);
            inpos+=cols;
            gptr+=4;
        }
        rowsleft-=dorows;
        mtxout+=ds2.nob*dorows;
        mtxin+=ds1.nob*dorows;
    }
    free(cb);
    free(gpc);
}

/* Column put scalar on bits=0 in consecutive rows  */

void BSColPutS (const FIELD * f, const uint64_t * bs, uint64_t nor,
              FELT scalar, Dfmt * mtx)
{
    uint64_t i,inoc,onoc;
    uint32_t * lix;
    Dfmt * optr;
    DSPACE ds;              // output space

    onoc=bs[0];   // columns in output
    inoc=bs[1];   // matrix is set bits

    if((onoc-inoc)!=nor)
    {
        printf(" BSColPutS %lu x %lu failure\n",nor,onoc-inoc);
        exit(13);
    }

//  Special case with no real work needed
    if(nor==0) return;      // zero rows in output
    DSSet(f,onoc,&ds);      // set output space

    lix=BSLixUn(bs);   // where does the scalar go

    optr=mtx;

    for(i=0;i<nor;i++)
    {
        DPak(&ds,lix[i],optr,scalar);
        optr+=ds.nob;
    }
    free(lix);
}

void BSCombine (const uint64_t * bs1, const uint64_t * bs2,
                  uint64_t * comb, uint64_t * rif)
{
    uint64_t bc1, sb1, sb2;
    uint64_t wd1,wd2,wdc,wdr;   // word containing the bits
    uint64_t ix1,ix2,ixc,ixr;   // index in array
    uint64_t sg1,sg2,sgc,sgr;   // sig bits in word
    uint64_t bt;                // a bit
    wd1=0;             // stop compiler warnings
    bc1=bs1[0];   // bits to be processed
    sb1=bs1[1];   // bits set in bs1
    sb2=bs2[1];   // bits set in bs2;
    comb[0]=bc1;
    comb[1]=sb1+sb2;
    rif[0]=sb1+sb2;
    rif[1]=sb1;

    sg1=0;
    ix1=2;
    sg2=0;
    ix2=2;
    sgc=0;
    ixc=2;
    sgr=0;
    ixr=2;
    wdr=0;   // stop
    wdc=0;   //  compiler
    wd2=0;   //   warnings
    while(bc1!=0)
    {
        bc1--;
        if(sg1==0)
        {
            wd1=bs1[ix1++];
            sg1=64;
        }
        bt=wd1&1;
        sg1--;
        wd1>>=1;
        if(bt==1)
        {
            if(sgc==64)
            {
                comb[ixc++]=wdc;
                sgc=0;
            }
            if(sgc==0) wdc=0;
            wdc>>=1;
            wdc+=0x8000000000000000ull;
            sgc++;

            if(sgr==64)
            {
                rif[ixr++]=wdr;
                sgr=0;
            }
            if(sgr==0) wdr=0;
            wdr>>=1;
            wdr+=0x8000000000000000ull;
            sgr++;
            continue;
        }
        if(sg2==0)
        {
            wd2=bs2[ix2++];
            sg2=64;
        }
        bt=wd2&1;
        wd2>>=1;
        sg2--;
        if(bt==1)
        {
            if(sgc==64)
            {
                comb[ixc++]=wdc;
                sgc=0;
            }
            if(sgc==0) wdc=0;
            wdc>>=1;
            wdc+=0x8000000000000000ull;
            sgc++;

            if(sgr==64)
            {
                rif[ixr++]=wdr;
                sgr=0;
            }
            if(sgr==0) wdr=0;
            wdr>>=1;
            sgr++;
            continue;
        }
        if(sgc==64)
        {
            comb[ixc++]=wdc;
            sgc=0;
        }
        if(sgc==0) wdc=0;
        wdc>>=1;
        sgc++;
    }
    if(sgc!=0) comb[ixc]=(wdc>>(64-sgc));
    if(sgr!=0)  rif[ixr]=(wdr>>(64-sgr));
}

extern void BSMkr(const uint64_t * lit, const uint64_t * big, uint64_t * rif)
{
    uint64_t riflen,i,j;
    int bit;
    riflen=big[1];
    rif[0]=riflen;
    rif[1]=lit[1];
    memset(rif+2,0,((riflen+63)/64)*8);
    j=0;
    for(i=0;i<big[0];i++)
    {
        bit=BSBitRead(big,i);
        if(bit==0)
        {
            bit=BSBitRead(lit,i);
            if(bit==1)
            {
                printf("BSMkr error - little not in big\n");
                exit(17);
            }
            continue;
        }
        bit=BSBitRead(lit,i);
        if(bit==1) BSBitSet(rif,j);
        j++;
    }
    return;
}

extern void BSShiftOr (const uint64_t * bs1, uint64_t shift, uint64_t * bs2)
{
    uint64_t src,dst;
    int bit;
    dst=shift;
    for(src=0;src<bs1[0];src++)
    {
        bit=BSBitRead(bs1,src);
        if(bit==1) BSBitSet(bs2,dst);
        dst++;
    }
}

/* end of bitstring.c */
