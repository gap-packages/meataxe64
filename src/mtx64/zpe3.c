/*
    zpe3.c meataxe-64 proggy based "Produce Echelon Form"
    ====== J G Thackray 13.08.15  R A Parker 10.8.16  ======
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

#define PRI1 ((i)+(j))
#define PRI2 ((i)+(k))

int main(int argc, char **argv)
{
    FIELD *f;
    DSPACE ds,ds1;
    EFIL * e;
    uint64 hdr[5];
    uint64 fdef, noc,nor,shift,maxrows;
    uint64 xc,xr,chsz;
    size_t rslen;
    uint64 *cs;
    uint64 * bsrs;
    uint64_t i, j, k, m;
    char st[200];
    MOJ fmoj;
    int rank = 0;
/* 2 d mojes */
    MOJ Aij,D,Eij,G,H,Kij,L,Mij,N,Q,Uij,S,Tij,V,W,X,Z;
    MOJ P[MAXCHOP];
    MOJ R[MAXCHOP];
    MOJ B[MAXCHOP][MAXCHOP];
    MOJ C[MAXCHOP][MAXCHOP];
    M3 CM;
    long *chp,*chpcol,*colstart;
    Dfmt * buf;
    Dfmt * mx;
    LogCmd(argc,argv);
    if (argc != 4)
    {
        LogString(80,"usage zpe <m1> <cs> <remnant>");
        exit(14);
    }

    TFInit(THREADS); /*  Start Thread farm */
    TFStopMOJFree();

    CM=M3Cons(argv[1]);
    M3Peek(CM);
    fdef = CM->fdef;
    nor = CM->nor;
    noc = CM->noc;
    fmoj=M3FieldMOJ(fdef);
    CM->fmoj=fmoj;

// determine chsz
    xr=nor;
    xc=noc;
    chsz=1;
    if((fdef%2)!=0)
    {
        xr=xr*3;
        xc=xc*3;
    }
    while( (xr>120) && (xc>120))
    {
        chsz=chsz+1+chsz/3;
        xr=xr/2;
        xc=xc/2;
    }
    if(chsz>MAXCHOP) chsz=MAXCHOP;


    CM->r=chsz;
    CM->c=chsz;
    M3EvenChop(CM);
    M3MOJs(CM);
    M3Read(CM);

/* Now get the field */
    f = (FIELD *) TFPointer(fmoj);

    for(i=0;i<chsz;i++)
        for(j=0;j<chsz;j++)
            C[i][j]=CM->m[i][j];
    for (i = 0; i < chsz; i++) /* Row echelise count in i */
    {
        for (j = 0; j < chsz; j++) /* Column echelise count in j */
        {
/* Special handling for i = 0 */
            if (0 == i)
            {
                H = C[i][j];
                ECH(PRI1, fmoj, H, Tij, P[j], Mij, Kij, R[j]);
/* Ensure we don't throw away the answer before use */
                if(i==(chsz-1)) TFGetReadRef(P[j]);
            } 
            else 
            {
                CEX(PRI1, fmoj, P[j], C[i][j], Aij, G);
                MAD(PRI1, fmoj, Aij, R[j], G, H);
                ECH(PRI1, fmoj, H, Tij, Q, Mij, Kij, N);
                CEX(PRI1, fmoj, Q, R[j], Eij, D);
                MAD(PRI1, fmoj, Eij, N, D, L);
                PVC(PRI1, P[j], Q, P[j], Uij);
                RRF(PRI1, fmoj, Uij, L, N, R[j]);
/* Ensure we don't throw away the answer before use */
                if(i==(chsz-1)) TFGetReadRef(P[j]);
            }
/*
* Now we propagate these results right and down from above
* These are the cubic parts.
* They don't happen on the final loo,
* as there's nothing to do.
*/
            for (k = j + 1; k < chsz; k++)
            {
                m = j + 1;
/* Special handling for first row */
                if (0 == i)
                {
/* Z1jk = C1jk = Y1j, hence use of C below */
                    REX(PRI2, fmoj, Tij, C[i][k], V, W);
                    MUL(PRI2, fmoj, Mij, V, X);
                    MCP(PRI2, fmoj, X, B[j][k]);
                } 
                else
                {
                    MAD(PRI2, fmoj, Aij, B[j][k], C[i][k], Z);
                    REX(PRI2, fmoj, Tij, Z, V, W);
                    MUL(PRI2, fmoj, Mij, V, X);
                    MAD(PRI2, fmoj, Eij, X, B[j][k], S);
                    RRF(PRI2, fmoj, Uij, S, X, B[j][k]);
                }
                if(m<chsz)
                    MAD(PRI2, fmoj, Kij, V, W, C[i][k]);
            }
        }
    }

/* Now do the back-cleaning  */
#define PRI3 (chsz-k)
    for(k=0;k<chsz;k++)
        MCP(PRI3, fmoj, R[k], C[k][k]);
    for(k=chsz-1;k>0;k--)
    {
        for(j=0;j<k;j++)
        {
            CEX(PRI3, fmoj, P[k], B[j][k], X, C[j][k]);
            for(m=k;m<chsz;m++)
                MAD(PRI3, fmoj, X, C[k][m], C[j][m], C[j][m]);
        }
    }
/* Keep the bits we need for the remnant */
    for(i=0;i<chsz;i++)
        for(j=i;j<chsz;j++)
            TFGetReadRef(C[i][j]);

    TFStartMOJFree();

/* Now wait for the results and compute the answer */
    i=chsz-1;
    for (j = 0; j < chsz; j++)
    {
        TFWait(P[j]);
        cs = TFPointer(P[j]);
        rank += cs[1];/* Add in number of set bits */
    }
/* compute the column-select bit string */
    rslen=16+((noc+63)/64)*8;
    bsrs=malloc(rslen);
    memset(bsrs,0,rslen);
    bsrs[0]=noc;
    bsrs[1]=rank;
    shift=0;
    for(j=0;j<chsz;j++)
    {
        cs = TFPointer(P[j]);
        BSShiftOr(cs,shift,bsrs);
        shift+=cs[0];
    }
    hdr[0]=2;
    hdr[1]=1;
    hdr[2]=noc;
    hdr[3]=rank;
    hdr[4]=0;
    e=EWHdr(argv[2],hdr);
    EWData(e,rslen,(uint8 *)bsrs);
    EWClose(e);
/* now write out remnant  */
    chp=malloc(chsz*sizeof(long));
    chpcol=malloc(chsz*sizeof(long));
    colstart=malloc(chsz*sizeof(long));
    k=0;
    maxrows=0;
    for(i=0;i<chsz;i++)   // amalgamate with above loop?
    {
        cs=TFPointer(P[i]);
        chp[i]=cs[1];
        if(maxrows<cs[1]) maxrows=cs[1];
        chpcol[i]=cs[0]-cs[1];
        colstart[i]=k;
        k+=(cs[0]-cs[1]);
        TFRelease(P[i]);
    }
    DSSet(f,noc-rank,&ds);
    buf=malloc(ds.nob*maxrows);
    hdr[0]=1;
    hdr[1]=f->fdef;
    hdr[2]=rank;
    hdr[3]=noc-rank;
    hdr[4]=0;
    e=EWHdr(argv[3],hdr);
    for(i=0;i<chsz;i++)
    {
        memset(buf,0,chp[i]*ds.nob);
        for(j=i;j<chsz;j++)
        {
            TFWait(C[i][j]);
            mx=TFPointer(C[i][j]);
            DSSet(f,chpcol[j],&ds1);
            DPaste(&ds1,mx+16,chp[i],colstart[j],&ds,buf);
        }
        EWData(e,chp[i]*ds.nob,(uint8 *)buf);
    }
    EWClose(e);
/* Free the bits we needed for the remnant */
    for(i=0;i<chsz;i++)
        for(j=i;j<chsz;j++)
            TFRelease(C[i][j]);
    sprintf(st,"Rank of %s is %d", argv[1], rank);
    printf("%s\n", st);
    LogString(20, st);
    TFWaitEnd();
    
    TFRelease(fmoj);
    TFClose();
    M3Dest(CM);
    free(bsrs);
    free(chp);
    free(chpcol);
    free(colstart);
    free(buf);
    return 0;
}

/* end of zpe3.c  */
