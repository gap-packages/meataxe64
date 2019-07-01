/*
    mech.c meataxe-64 mezzanine echelize
    ====== R A Parker 4.6.17, Gabi, Jon, Steve, Alice  ======
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

typedef struct
{
    MOJ rs;     // Rows selected so far
    MOJ rf;     // Riffle
}  STE;

static void CD(int pri, MOJ fmoj, MOJ C, STD * D, STA * A)
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

static void CD0(int pri, MOJ fmoj, MOJ C, STD * D, STA * A)
{
    ECH(pri, fmoj, C, A->rs, D->cs, A->M, A->K, D->R);
}

static void UR(int pri, MOJ fmoj, STA * A, MOJ * B, MOJ * C)
{
    MOJ Z,V,W,X,S;
    MAD(pri, fmoj, A->A, *B, *C, Z);
    REX(pri, fmoj, A->rs, Z, V, W);
    MUL(pri, fmoj, A->M, V, X);
    MAD(pri, fmoj, A->E, X, *B, S);
    RRF(pri, fmoj, A->rf, S, X, *B);
    MAD(pri, fmoj, A->K, V, W, *C);
}

static void URT(int pri, MOJ fmoj, STA *A, STE *E, MOJ *M, MOJ *K,
            int uc)
{
    MOJ S,V,W,X,Z;

    if( (uc==2) || (uc==4) || (uc==6) )
      CRZ(pri,fmoj,E->rf,*K,*K);
    if(uc==1)
      MUL(pri,fmoj,A->A,*M,Z);
    if(uc==2)
      MAD(pri,fmoj,A->A,*M,*K,Z);
    if( (uc==4) || (uc==6) )
      MCP(pri,fmoj,*K,Z);
    if( (uc==1) || (uc==2) || (uc==4) || (uc==6) )
      REX(pri,fmoj,A->rs,Z,V,W);
    if( (uc==4) || (uc==6) )
      ADI(pri,fmoj,E->rf,V,V);
    if( (uc==1) || (uc==2) || (uc==4) || (uc==6) )
      MUL(pri,fmoj,A->M,V,X);
    if( (uc==3) || (uc==5) )
      MCP(pri,fmoj,A->M,X);     
    if( (uc==1) || (uc==2) )
      MAD(pri,fmoj,A->E,X,*M,S);
    if((uc==3)|| (uc==4) )
      MUL(pri,fmoj,A->E,X,S);
    if( (uc==1) || (uc==2) || (uc==3) || (uc==4))
      RRF(pri,fmoj,A->rf,S,X,*M);
    if( (uc==5) || (uc==6) )
      MCP(pri,fmoj,X,*M);
    if( (uc==3) || (uc==5) )
      MCP(pri,fmoj,A->K,*K);
    if( (uc==1) || (uc==2) || (uc==4) || (uc==6) )
      MAD(pri,fmoj,A->K,V,W,*K);
}

static void UR0(int pri, MOJ fmoj, STA * A, MOJ * B, MOJ * C)
{
    MOJ V,W,X;
    REX(pri, fmoj, A->rs, *C, V, W);
    MUL(pri, fmoj, A->M, V, X);
    MCP(pri, fmoj, X, *B);
    MAD(pri, fmoj, A->K, V, W, *C);
}

static void RL(int pri, MOJ fmoj, MOJ M1, STE * E1, STE * E2, MOJ * M2)
{
    MOJ rif;
    MKR(pri,E1->rs,E2->rs,rif);
    CRZ(pri,fmoj,rif,M1,*M2);
}

uint64_t mech(const char *m1, int s1, const char *b2, int s2, 
         const char *b3, int s3, const char *m4, int s4,
         const char *m5, int s5, const char *m6, int s6)
{
    FIELD *f;
    DSPACE ds,ds1;
    EFIL * e;
    uint64_t hdr[5];
    uint64_t fdef, noc,nor,shift,maxrows;
    uint64_t xc,xr,chsz;
    uint64_t cha;   //  rows of blocks
    uint64_t chb;   //  cols of blocks
    size_t cslen,rslen;
    uint64_t *cs,*rs;
    uint64_t *bscs,*bsrs;
    uint64_t i, h;       // rows of blocks
    uint64_t j, k, l;    // cols of blocks
    MOJ fmoj;
    int urtcase;
    int rank = 0;
    STA A;
    STD D[MAXCHOP];
    STE E[MAXCHOP][MAXCHOP];
/* 2 d mojes */
    MOJ X;
    MOJ B[MAXCHOP][MAXCHOP];
    MOJ C[MAXCHOP][MAXCHOP];
    MOJ R[MAXCHOP][MAXCHOP];
    MOJ K[MAXCHOP][MAXCHOP];
    MOJ M[MAXCHOP][MAXCHOP];
    M3 CM;
    long *chp,*chpcol,*colstart;
    long *chq,*chqcol,*coqstart;
    long *chr,*chrcol,*corstart;
    Dfmt * buf;
    Dfmt * mx;
    uint64_t *mm;
    int tick;      // priority shift for phases

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
    while( (xr>10000) && (xc>10000))
    {
        chsz=chsz+1+chsz/5;
        xr=xr/2;
        xc=xc/2;
    }
    while((chsz*(chsz-1))<3*THREADS) chsz++;
    if(chsz>MAXCHOP) chsz=MAXCHOP;

    cha=chsz;
    chb=chsz;
//printf("Chopping %d\n",(int)chsz);
    tick=cha+chb+1;

    CM->r=cha;
    CM->c=chb;
    M3EvenChop(CM,f->entbyte,f->entbyte,1,1);
    M3MOJs(CM);
    M3Read(CM);

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
                CD0(i+j,fmoj,C[i][j],&D[j],&A);
                if(i==(cha-1)) TFGetReadRef(D[j].cs);

            } 
            else 
            {
                CD(i+j,fmoj,C[i][j],&D[j],&A);
                if(i==(cha-1)) TFGetReadRef(D[j].cs);

            }
/*  E = Extend(A,E) */
            if(j==0)
                PC0(i+j,A.rs,E[i][j].rs,E[i][j].rf);              
            else
                PVC(i+j,E[i][j-1].rs,A.rs,E[i][j].rs,E[i][j].rf);
            for (k = j + 1; k < chb; k++)
            {
/* (C,B) = UpdateRow(A,C,B)  */
                if (i==0)
                    UR0(i+k,fmoj,&A,&B[j][k],&C[i][k]);
                else
                     UR(i+k,fmoj,&A,&B[j][k],&C[i][k]);
            }
            for(h=0;h<=i;h++)
            {
/*  (K,M)=UpdateRowTrafo (A,K,E) */
                urtcase=1;
                if(h==i) urtcase+=2;
                if(i==0) urtcase+=2;
                if(j!=0) urtcase+=1;
                URT(tick+i+j,fmoj,&A,&E[h][j],&M[j][h],
                        &K[i][h],urtcase);
            }
        }
    }
/* step 2 */
    for(j=0;j<chb;j++)
    {
        for(h=0;h<cha;h++)
        {
            RL(2*tick,fmoj, M[j][h],&E[h][j],&E[h][chb-1],&M[j][h]); 
                                  // what is correct priority?
        }
    }

/* step 3 - the back-clean  */
    for(k=0;k<chb;k++)
/* R = Copy(D) */
        MCP(3*tick, fmoj, D[k].R, R[k][k]);

    for(k=chb-1;k>0;k--)   // not done for k=0?
    {
        for(j=0;j<k;j++) 
        {
/* (X,R) = PreClearUp(B,D)  */
            CEX(6*tick-j-k, fmoj, D[k].cs, B[j][k], X, R[j][k]);
            for(l=k;l<chb;l++)
/* R = ClearUp(R,X,R)  */
                MAD(6*tick-j-k, fmoj, X, R[k][l], R[j][l], R[j][l]);

            for(h=0;h<cha;h++)
/* M = ClearUp(M,X,M)  */
                MAD(6*tick-j-k, fmoj, X, M[k][h], M[j][h], M[j][h]);
        }
    }
/* Keep the bits we need for the remnant */
    for(i=0;i<chb;i++)
        for(j=i;j<chb;j++)
            TFGetReadRef(R[i][j]);
/* and for row-select output */
    for(h=0;h<cha;h++)
        TFGetReadRef(E[h][chb-1].rs);
    for(j=0;j<chb;j++)
        for(h=0;h<cha;h++)
            TFGetReadRef(M[j][h]);
    for(i=0;i<cha;i++)
        for(h=0;h<=i;h++)
            TFGetReadRef(K[i][h]);

    TFStartMOJFree();

/* Now wait for the results and compute the answer */
    for (j = 0; j < chb; j++)
    {
        TFWait(D[j].cs);
        cs = TFPointer(D[j].cs);
        rank += cs[1];    /* Add in number of set bits */
    }
/* compute the column-select bit string */
    cslen=16+((noc+63)/64)*8;
    bscs=malloc(cslen);
    memset(bscs,0,cslen);
    bscs[0]=noc;
    bscs[1]=rank;
    shift=0;
    for(j=0;j<chb;j++)
    {
        cs = TFPointer(D[j].cs);
        BSShiftOr(cs,shift,bscs);
        shift+=cs[0];
    }
    hdr[0]=2;
    hdr[1]=1;
    hdr[2]=noc;
    hdr[3]=rank;
    hdr[4]=0;
    e=EWHdr(b3,hdr);
    EWData(e,cslen,(uint8_t *)bscs);
    EWClose1(e,s3);
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
    e=EWHdr(m6,hdr);
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
        EWData(e,chp[j]*ds.nob,(uint8_t *)buf);
    }
    EWClose1(e,s6);

/* now write out multiplier  */
    chq=malloc(chb*sizeof(long));
    chqcol=malloc(cha*sizeof(long));
    coqstart=malloc(cha*sizeof(long));
    for(j=0;j<chb;j++)
        for(h=0;h<cha;h++)
            TFWait(M[j][h]);

    maxrows=0;
    for(j=0;j<chb;j++)
    {
        mm=TFPointer(M[j][0]);
        chq[j]=mm[0];
        if(maxrows<chq[j]) maxrows=chq[j];
    }
    k=0;
    for(h=0;h<cha;h++)
    {
        mm=TFPointer(M[0][h]);
        chqcol[h]=mm[1];
        coqstart[h]=k;
        k+=chqcol[h]; 
    }
    DSSet(f,k,&ds);
    free(buf);
    buf=malloc(ds.nob*maxrows);
    hdr[0]=1;
    hdr[1]=f->fdef;
    hdr[2]=rank;
    hdr[3]=rank;
    hdr[4]=0;

    e=EWHdr(m4,hdr);
    for(j=0;j<chb;j++)
    {
        memset(buf,0,chq[j]*ds.nob);
        for(h=0;h<cha;h++)
        {
            mx=TFPointer(M[j][h]);
            DSSet(f,chqcol[h],&ds1);
            DPaste(&ds1,mx+16,chq[j],coqstart[h],&ds,buf);
        }
        EWData(e,chq[j]*ds.nob,(uint8_t *)buf);
    }
    EWClose1(e,s4);
    free(chq);
    free(chqcol);
    free(coqstart);

/* now write out cleaner  */
    chr=malloc(cha*sizeof(long));
    chrcol=malloc(cha*sizeof(long));
    corstart=malloc(cha*sizeof(long));
    for(i=0;i<cha;i++)
        for(h=0;h<=i;h++)
            TFWait(K[i][h]);

    maxrows=0;
    for(i=0;i<cha;i++)
    {
        mm=TFPointer(K[i][0]);
        chr[i]=mm[0];
        if(maxrows<chr[i]) maxrows=chr[i];
    }
    k=0;
    for(h=0;h<cha;h++)
    {
        mm=TFPointer(K[cha-1][h]);
        chrcol[h]=mm[1];
        corstart[h]=k;
        k+=chrcol[h]; 
    }
    DSSet(f,k,&ds);
    free(buf);
    buf=malloc(ds.nob*maxrows);
    hdr[0]=1;
    hdr[1]=f->fdef;
    hdr[2]=nor-rank;
    hdr[3]=rank;
    hdr[4]=0;

    e=EWHdr(m5,hdr);
    for(i=0;i<cha;i++)
    {
        memset(buf,0,chr[i]*ds.nob);
        for(h=0;h<=i;h++)
        {
            mx=TFPointer(K[i][h]);
            DSSet(f,chrcol[h],&ds1);
            DPaste(&ds1,mx+16,chr[i],corstart[h],&ds,buf);
        }
        EWData(e,chr[i]*ds.nob,(uint8_t *)buf);
    }
    EWClose1(e,s5);
    free(chr);
    free(chrcol);
    free(corstart);
    for(i=0;i<cha;i++)
        for(h=0;h<=i;h++)
            TFRelease(K[i][h]);

/* compute the row-select bit string */
    rslen=16+((nor+63)/64)*8;
    bsrs=malloc(rslen);
    memset(bsrs,0,rslen);
    bsrs[0]=nor;
    bsrs[1]=rank;
    shift=0;
    for(h=0;h<cha;h++)
    {
        rs = TFPointer(E[h][chb-1].rs);
        BSShiftOr(rs,shift,bsrs);
        shift+=rs[0];
    }
    hdr[0]=2;
    hdr[1]=1;
    hdr[2]=nor;
    hdr[3]=rank;
    hdr[4]=0;
    e=EWHdr(b2,hdr);
    EWData(e,rslen,(uint8_t *)bsrs);
    EWClose1(e,s2);
    free(bsrs);
/* Free the bits we needed for output */
    for(j=0;j<chb;j++)      // remnant
        for(k=j;k<chb;k++)
            TFRelease(R[j][k]);
    for(h=0;h<cha;h++)      // row-select bitstring
        TFRelease(E[h][chb-1].rs);
    for(j=0;j<chb;j++)      // multiplier
        for(h=0;h<cha;h++)
            TFRelease(M[j][h]);
    TFWaitEnd();
    
    TFRelease(fmoj);
    TFClose();
    M3Dest(CM);
    free(bscs);
    free(chp);
    free(chpcol);
    free(colstart);
    free(buf);
    return rank;
}

/* end of mech.c  */
