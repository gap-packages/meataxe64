/*
         bitstring.c  -   Bit String Operations
         ===========      R. A. Parker 21.9.2015
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "bitstring.h"
#include "pcrit.h"

#define WKSIZE 3313
#define MXROWS 25

void BSColSelect (const FIELD * f, const uint64 * bs, uint64 nor,
                  const Dfmt * d, Dfmt * sel, Dfmt * nonsel)
{
    uint64 noc,noc1,noc0;
    uint64 maxcols,col,col0,col1;
    uint64 wid,row,r1;
    DSPACE ds,ds1,ds0,dscb;
    int b,bit;
    const Dfmt *pt;
    Dfmt *pt1,*pt0;
    Dfmt cb[WKSIZE];    // clipboard
    bit=0;     // get rid of compiler warnings
// first set up the spaces needed
    noc=bs[0];
    noc1=bs[1];
    noc0=noc-noc1;
    DSSet(f,noc,&ds);  //  in space
    DSSet(f,noc1,&ds1);  // sel out space
    DSSet(f,noc0,&ds0);  // nonsel out space
    memset(sel,0,ds1.nob*nor);
    memset(nonsel,0,ds0.nob*nor);
//  set maxcols as the greatest number we may want to do
    maxcols=((WKSIZE/MXROWS)*f->entbyte)/f->bytesper;

//  outer loop does a batch of equal bits
    col=0;
    col0=0;
    col1=0;
    while(col!=noc)
    {
        wid=0;
        if(maxcols>noc-col) maxcols=noc-col;
        while(wid<maxcols)
        {
            b=BSBitRead(bs,col+wid);
            if(wid==0) bit=b;
            if(b!=bit) break;
            wid++; 
        }
// so we want to move wid columns starting at col
        DSSet(f,wid,&dscb);  //  set clipboard space
        pt=d;
        pt1=sel;
        pt0=nonsel;
//  next loop does some rows
        row=0;
        while(row!=nor)
        {
// how many rows (r1) do we want to do this time round?
            r1=nor-row;
            if(r1>MXROWS) r1=MXROWS;
            DCut(&ds,r1,col,pt,&dscb,cb);
            if(bit==1) 
                DPaste(&dscb,cb,r1,col1,&ds1,pt1);
            else
                DPaste(&dscb,cb,r1,col0,&ds0,pt0);
            row+=r1;
            pt+=r1*ds.nob;
            pt1+=r1*ds1.nob;
            pt0+=r1*ds0.nob;
        }
        col+=wid;
        if(bit==1) 
            col1+=wid;
        else
            col0+=wid;
    }
}

uint32_t * BSLix(const uint64_t * bs, int flag)
{
    int nent;
    uint32_t i,j;
    uint32_t * ix;

    nent=bs[0]-bs[1];    // unset bits
    if(flag==1) nent=bs[1];
    ix=malloc(nent*sizeof(uint32_t));
    j=0;
    for(i=0;i<bs[0];i++)
    {
        if(BSBitRead(bs,i)==flag)
        {
            ix[j]=i;
            j++;
        }
    }
    return ix;
}

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

void BSColRifZ (const FIELD * f, const uint64_t * bs, uint64 nor,
                  const Dfmt * mtxin, Dfmt * mtxout)
{
    uint64_t noc1,noc2;
    uint64_t maxnor,maxnor2,maxnoc;
    uint64_t dorows,rowsleft;
    uint64_t colsleft,docols;
    uint64_t outcol,colpos;
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
    if(noc1==0)           // no set bits
    {                     // so zeroize output
        memset(mtxout,0,ds2.nob*nor);
        return;        
    }

    gpc=BSGpc(bs);  // convert bs to grouped columns format

/*  compute maxnor, maxnoc for cache-friendly operation  */

    maxnor=pcstride(ds1.nob);
    maxnor2=pcstride(ds2.nob);
    if(maxnor2<maxnor) maxnor=maxnor2;    // maxnor is at most 200
    maxnoc=16000/maxnor;                  // so maxnoc is at least 80 here 
    maxnoc=(maxnoc*f->entbyte)/f->bytesper;  // and at least 10 here

/*  allocate clipboard  */

    DSSet(f,maxnoc,&ds3);
    cb=(Dfmt *) malloc(ds3.nob*maxnor);

    rowsleft=nor;
    while(rowsleft!=0)
    {
        dorows=maxnor;
        if(dorows>rowsleft) dorows=rowsleft;
        gptr=gpc+1;
        if(gpc[0]==0) gptr+=2;  // only interested in 1-bits
        memset(mtxout,0,ds2.nob*dorows);
        outcol=0;
        while(gptr[0]!=ENDGPC)
        {
            colsleft=gptr[1];
            colpos=gptr[0];
            while(colsleft>0)
            {
                docols=colsleft;
                if(docols>maxnoc) docols=maxnoc;
                DSSet(f,docols,&ds3);
                DCut(&ds1,dorows,colpos,mtxin,&ds3,cb);
                DPaste(&ds3,cb,dorows,outcol,&ds2,mtxout);
                colpos+=docols;
                colsleft-=docols;
                outcol+=docols;
            }
            outcol+=gptr[3];   // skip zero columns in output
        }
        rowsleft-=dorows;
        mtxout+=ds2.nob*dorows;
    }

    free(gpc);
}

/* Column riffle with scalar on bits equal to flag  */

void BSColRifS (const FIELD * f, const uint64_t * bs, uint64 nor,
              int flag, FELT scalar, const Dfmt * mtxin, Dfmt * mtxout)
{
    uint64_t noc1,noc2;

//  Special cases with no real work needed
    if(nor==0) return;      // zero rows in output
    if(flag==1)
    {
        noc1=bs[0]-bs[1];   // matrix is unset bits
        if(nor!=bs[1])
        {
            printf(" identity %lu x %lu failure\n",nor,bs[1]);
            exit(13);
        }
    }
    else
    {
        noc1=bs[1];   // matrix is unset bits
        if(nor!=bs[0]-bs[1])
        {
            printf(" identity %lu x %lu failure\n",nor,bs[0]-bs[1]);
            exit(13);
        }
    }
    noc2=bs[0];             // total bits = noc(output)
    if(noc2==0) return;     // zero columns in output
    if(flag==0) noc1=bs[1]; // or set bits if flag=0
// unfinished
printf("%lu",noc1);
}

#ifdef NEVER

    uint64_t maxnor,maxnor2,maxnoc;
    uint64_t dorows,rowsleft;
    uint64_t colsleft,docols;
    uint64_t outcol,colpos;
    DSPACE ds1,ds2,ds3;
    uint32_t *gpc,*gptr;
    Dfmt * cb;




    DSSet(f,noc1,&ds1);   // input space
    if(noc1==noc2)        // no unset bits . . .
    {                     // just copy input to output
        memcpy(mtxout,mtxin,ds1.nob*nor);
        return;
    }
    DSSet(f,noc2,&ds2);   // output space
    if(noc1==0)           // no set bits
    {                     // so zeroize output
        memset(mtxout,0,ds2.nob*nor);
        return;        
    }

    gpc=BSGpc(bs);  // convert bs to grouped columns format

/*  compute maxnor, maxnoc for cache-friendly operation  */

    maxnor=pcstride(ds1.nob);
    maxnor2=pcstride(ds2.nob);
    if(maxnor2<maxnor) maxnor=maxnor2;    // maxnor is at most 200
    maxnoc=16000/maxnor;                  // so maxnoc is at least 80 here 
    maxnoc=(maxnoc*f->entbyte)/f->bytesper;  // and at least 10 here

/*  allocate clipboard  */

    DSSet(f,maxnoc,&ds3);
    cb=(Dfmt *) malloc(ds3.nob*maxnor);

    rowsleft=nor;
    while(rowsleft!=0)
    {
        dorows=maxnor;
        if(dorows>rowsleft) dorows=rowsleft;
        gptr=gpc+1;
        if(gpc[0]==0) gptr+=2;  // only interested in 1-bits
        memset(mtxout,0,ds2.nob*dorows);
        outcol=0;
        while(gptr[0]!=ENDGPC)
        {
            colsleft=gptr[1];
            colpos=gptr[0];
            while(colsleft>0)
            {
                docols=colsleft;
                if(docols>maxnoc) docols=maxnoc;
                DSSet(f,docols,&ds3);
                DCut(&ds1,dorows,colpos,mtxin,&ds3,cb);
                DPaste(&ds3,cb,dorows,outcol,&ds2,mtxout);
                colpos+=docols;
                colsleft-=docols;
                outcol+=docols;
            }
            outcol+=gptr[3];   // skip zero columns in output
        }
        rowsleft-=dorows;
        mtxout+=ds2.nob*dorows;
    }

    free(gpc);
#endif


/* This routine to be removed once RifZ/RifI working properly */

void BSColRiffle (const FIELD * f, const uint64 * bs, uint64 nor,
                  const Dfmt * set,  const Dfmt * unset, Dfmt * rif)
{
    uint64 r,c,noc,noc1,noc2;
    DSPACE ds, dsset, dsunset;
    Dfmt *pt;
    const Dfmt *pt1,*pt2;
    uint64 sg,ix,wd,bt,c1,c2;
    FELT fel;
    wd=0;                   // stop compiler warnings
    noc=bs[0];
    noc1=bs[1];
    noc2=noc-noc1;
    DSSet(f,noc,&ds);  //  rif space
    DSSet(f,noc1,&dsset);  // set in space
    DSSet(f,noc2,&dsunset);  // unset in space
    pt=rif;
    pt1=set;
    pt2=unset;
    memset(pt,0,nor*ds.nob);
    for(r=0;r<nor;r++)
    {
        c1=0;
        c2=0;
        sg=0;               // no significant bits in wd yet
        ix=2;               // bs[2] is next word to get
        for(c=0;c<noc;c++)
        {
            if(sg==0)      // if no bits left in word,
            {
                wd=bs[ix++];  // get another one
                sg=64;        // so now we've got 64 bits
            }
            bt=wd&1;          // extract the bit for this loop
            sg--;             // one fewer bits in word now
            wd=(wd>>1);       // shift new bit into position
            if(bt==1) fel=DUnpak(&dsset,c1++,pt1);
               else   fel=DUnpak(&dsunset,c2++,pt2);
            DPak(&ds,c,pt,fel);
        }
        pt+=ds.nob;
        pt1+=dsset.nob;
        pt2+=dsunset.nob;
    }
}

void BSCombine (const uint64 * bs1, const uint64 * bs2,
                  uint64 * comb, uint64 * rif)
{
    uint64 bc1, sb1, sb2;
    uint64 wd1,wd2,wdc,wdr;   // word containing the bits
    uint64 ix1,ix2,ixc,ixr;   // index in array
    uint64 sg1,sg2,sgc,sgr;   // sig bits in word
    uint64 bt;                // a bit
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

extern void BSShiftOr (const uint64 * bs1, uint64 shift, uint64 * bs2)
{
    uint64 src,dst;
    int bit;
    dst=shift;
    for(src=0;src<bs1[0];src++)
    {
        bit=BSBitRead(bs1,src);
        if(bit==1) BSBitSet(bs2,dst);
        dst++;
    }
}
 
void BSBitSet (uint64 * bs, uint64 bitno)
{
    uint64 ix,val,sh;
    ix=2 + (bitno/64);
    val=1;
    sh=bitno%64;
    if(sh!=0) val=(val<<sh);
    bs[ix] |= val;
    return;
}

int  BSBitRead (const uint64 *bs, uint64 bitno)
{
    uint64 ix,val,sh;
    ix=2 + (bitno/64);
    val=1;
    sh=bitno%64;
    if(sh!=0) val=(val<<sh);
    if ( (bs[ix]&val)==0 ) return 0;
    return 1;
}

/* end of bitstring.c */
