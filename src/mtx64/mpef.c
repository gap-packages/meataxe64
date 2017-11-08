/*
    mpef.c meataxe-64 subroutine "Produce Echelon Form"
    ====== J G Thackray 13.08.15  R A Parker 1.6.17  ======
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "bitstring.h"
#include "tuning.h"

typedef struct
{
    MOJ A;      // pivotal matrix to clean with old B
    MOJ M;      // matrix to multiply new rows by
    MOJ K;      // cleaner to take new rows out of remaining ones
    MOJ rs;     // row-select bitstring for new pivotal rows
    MOJ E;      // clean old B with new rows
    MOJ rf;     // riffle to put new rows into B
}  STA;

typedef struct
{
    MOJ R;      // remnant from blocks above
    MOJ cs;     // total pivotal rows in this column so far
}  STD;

void CD(int pri, MOJ fmoj, MOJ C, STD * D, STA * A)
{
    MOJ Ad,H,Gmd,R,Rd1,Rd;
    CEX(pri, fmoj, D->cs, C, A->A, Ad);
    MAD(pri, fmoj, A->A, D->R, Ad, H);
    ECH(pri, fmoj, H, A->rs, Gmd, A->M, A->K, R);
    CEX(pri, fmoj, Gmd, D->R, A->E, Rd1);
    MAD(pri, fmoj, A->E, R, Rd1, Rd);
    PVC(pri, D->cs, Gmd, D->cs, A->rf);
    RRF(pri, fmoj, A->rf, Rd, R, D->R);
}

void CD0(int pri, MOJ fmoj, MOJ C, STD * D, STA * A)
{
    ECH(pri, fmoj, C, A->rs, D->cs, A->M, A->K, D->R);
}

void UR(int pri, MOJ fmoj, STA * A, MOJ * B, MOJ * C)
{
    MOJ Z,V,W,X,S;
    MAD(pri, fmoj, A->A, *B, *C, Z);
    REX(pri, fmoj, A->rs, Z, V, W);
    MUL(pri, fmoj, A->M, V, X);
    MAD(pri, fmoj, A->E, X, *B, S);
    RRF(pri, fmoj, A->rf, S, X, *B);
    MAD(pri, fmoj, A->K, V, W, *C);
}

void UR0(int pri, MOJ fmoj, STA * A, MOJ * B, MOJ * C)
{
    MOJ V,W,X;
    REX(pri, fmoj, A->rs, *C, V, W);
    MUL(pri, fmoj, A->M, V, X);
    MCP(pri, fmoj, X, *B);
    MAD(pri, fmoj, A->K, V, W, *C);
}

uint64_t mpef(const char *m1, int s1, const char *b2, int s2,
              const char *m3, int s3)
{
    FIELD *f;
    DSPACE ds,ds1;
    EFIL * e;
    uint64 hdr[5];
    uint64 fdef, noc,nor,shift,maxrows;
    uint64 xc,xr,chsz;
    uint64_t cha;   //  rows of blocks
    uint64_t chb;   //  cols of blocks
    size_t rslen;
    uint64 *cs;
    uint64 * bsrs;
    uint64_t i;          // rows of blocks
    uint64_t j, k, l;    // cols of blocks
    MOJ fmoj;
    uint64_t rank = 0;
    STA A;
    STD D[MAXCHOP];
/* 2 d mojes */
    MOJ X;
    MOJ B[MAXCHOP][MAXCHOP];
    MOJ C[MAXCHOP][MAXCHOP];
    MOJ R[MAXCHOP][MAXCHOP];
    M3 CM;
    long *chp,*chpcol,*colstart;
    Dfmt * buf;
    Dfmt * mx;

    TFInit(THREADS); /*  Start Thread farm */
    TFStopMOJFree();

    CM=M3Cons(m1,s1);
    M3Peek(CM);
    fdef = CM->fdef;
    nor = CM->nor;
    noc = CM->noc;
    fmoj=M3FieldMOJ(fdef);
    f = (FIELD *) TFPointer(fmoj);
    CM->fmoj=fmoj;

// determine chsz
    xr=nor*THREADS;
    xc=noc*THREADS;
    chsz=1;
    while( (xr>600) && (xc>600))
    {
        chsz=chsz+1+chsz/5;
        xr=xr/2;
        xc=xc/2;
    }
    if(chsz>MAXCHOP) chsz=MAXCHOP;

    cha=chsz;
    chb=chsz;

// printf("Chopping %d\n",(int)chsz);

    CM->r=cha;
    CM->c=chb;
    M3EvenChop(CM,f->entbyte,f->entbyte);
    M3MOJs(CM);
    M3Read(CM);


#define PRI1 ((i)+(j))
#define PRI2 ((i)+(k))

/*  copy the input mojes as C will change and CM must not */
    for(i=0;i<cha;i++)
        for(j=0;j<chb;j++)
            C[i][j]=CM->m[i][j];

/* CHIEF starts here */
    for (i = 0; i < cha; i++) /* Row echelise count in i */
    {
        for (j = 0; j < chb; j++) /* Column echelise count in j */
        {
/* (D,A) = ClearDown(C,D) */
            if (i == 0)
            {
                CD0(PRI1,fmoj,C[i][j],&D[j],&A);
                if(i==(cha-1)) TFGetReadRef(D[j].cs);
            } 
            else 
            {
                CD(PRI1,fmoj,C[i][j],&D[j],&A);
                if(i==(cha-1)) TFGetReadRef(D[j].cs);
            }
            for (k = j + 1; k < chb; k++)
            {
/* (C,B) = UpdateRow(A,C,B)  */
                if (i==0)
                    UR0(PRI2,fmoj,&A,&B[j][k],&C[i][k]);
                else
                     UR(PRI2,fmoj,&A,&B[j][k],&C[i][k]);
            }
        }
    }

/* Now do the back-cleaning  */
#define PRI3 (chb-k)
    for(k=0;k<chb;k++)
/* R = Copy(D) */
        MCP(PRI3, fmoj, D[k].R, R[k][k]);

    for(k=chb-1;k>0;k--)
    {
        for(j=0;j<k;j++)
        {
/* (X,R) = PreClearUp(B,D)  */
            CEX(PRI3, fmoj, D[k].cs, B[j][k], X, R[j][k]);
            for(l=k;l<chb;l++)
/* R = ClearUp(R,X,R)  */
                MAD(PRI3, fmoj, X, R[k][l], R[j][l], R[j][l]);
        }
    }
/* Keep the bits we need for the remnant */
    for(i=0;i<chb;i++)
        for(j=i;j<chb;j++)
            TFGetReadRef(R[i][j]);

    TFStartMOJFree();

/* Now wait for the results and compute the answer */
    for (j = 0; j < chb; j++)
    {
        TFWait(D[j].cs);
        cs = TFPointer(D[j].cs);
        rank += cs[1];    /* Add in number of set bits */
    }
/* compute the column-select bit string */
    rslen=16+((noc+63)/64)*8;
    bsrs=malloc(rslen);
    memset(bsrs,0,rslen);
    bsrs[0]=noc;
    bsrs[1]=rank;
    shift=0;
    for(j=0;j<chb;j++)
    {
        cs = TFPointer(D[j].cs);
        BSShiftOr(cs,shift,bsrs);
        shift+=cs[0];
    }
    hdr[0]=2;
    hdr[1]=1;
    hdr[2]=noc;
    hdr[3]=rank;
    hdr[4]=0;
    e=EWHdr(b2,hdr);
    EWData(e,rslen,(uint8 *)bsrs);
    EWClose1(e,s2);
/* now write out remnant  */
    chp=malloc(chb*sizeof(long));
    chpcol=malloc(chb*sizeof(long));
    colstart=malloc(chb*sizeof(long));
    k=0;
    maxrows=0;
    for(j=0;j<chb;j++)   // amalgamate with above loop?
    {
        cs=TFPointer(D[j].cs);
        chp[j]=cs[1];
        if(maxrows<cs[1]) maxrows=cs[1];
        chpcol[j]=cs[0]-cs[1];
        colstart[j]=k;
        k+=(cs[0]-cs[1]);
        TFRelease(D[j].cs);
    }
    DSSet(f,noc-rank,&ds);
    buf=malloc(ds.nob*maxrows);
    hdr[0]=1;
    hdr[1]=f->fdef;
    hdr[2]=rank;
    hdr[3]=noc-rank;
    hdr[4]=0;
    e=EWHdr(m3,hdr);
    for(j=0;j<chb;j++)
    {
        memset(buf,0,chp[j]*ds.nob);
        for(k=j;k<chb;k++)
        {
            TFWait(R[j][k]);
            mx=TFPointer(R[j][k]);
            DSSet(f,chpcol[k],&ds1);
            DPaste(&ds1,mx+16,chp[j],colstart[k],&ds,buf);
        }
        EWData(e,chp[j]*ds.nob,(uint8 *)buf);
    }
    EWClose1(e,s3);
/* Free the bits we needed for the remnant */
    for(j=0;j<chb;j++)
        for(k=j;k<chb;k++)
            TFRelease(R[j][k]);
    TFWaitEnd();
    
    TFRelease(fmoj);
    TFClose();
    M3Dest(CM);
    free(bsrs);
    free(chp);
    free(chpcol);
    free(colstart);
    free(buf);
    return rank;
}

/* end of mpef.c  */
