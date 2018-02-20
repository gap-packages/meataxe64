/*
         dech.c  -   Dfmt Echelize routine
         =======     R. A. Parker 17.1.2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "bitstring.h"
#include "dech.h"

// ds, a, nor, flags set on input
// rs, cs, det allocated
// m, c, r allocated by DEch, must be free'd by caller

uint64_t DEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt **m, Dfmt **c, Dfmt **r, uint64_t nor,
             int flags)
{
    const FIELD * f;
    FELT deter,fel;
    uint64_t rank,poolsize;
    DSPACE dkt,dkts;   // keeptrack, keeptrack short
    Dfmt * kt;    // allocated
    Dfmt *va, *vk, *wa, *wk;
    Dfmt *mm;     // multiplier
    uint64_t i,j,k,col,orow;
/* a pool is limited here to at most 20 rows */
    uint64_t poolrows[20];
    uint64_t poolpivs[20];
    uint64_t *colrow;     // column -> row with pivot or ZEROROW
    uint64_t *pivs;       // row -> col with pivot or ZEROROW
    Dfmt *gra, *grk;      // greased batch

    f=ds->f;
    deter=1;   // determinant 1 so far
    rank=0;    // rank is zero so far

// Allocate keeptrack and set it to zero.  i cols.
    i=nor;
    if(i>ds->noc) i=ds->noc;
    DSSet(f,i,&dkt);
    kt=malloc(dkt.nob*nor);
    memset(kt,0,nor*dkt.nob);

// Set up the bitstrings
    i=8*(2+(nor+63)/64);
    memset(rs,0,i);
    rs[0]=nor;
    i=8*(2+(ds->noc+63)/64);
    memset(cs,0,i);
    cs[0]=ds->noc;

// Set up global arrays
    colrow=malloc(ds->noc*sizeof(uint64_t));
    pivs=malloc(nor*sizeof(uint64_t));
    for(i=0;i<nor;i++) colrow[i]=ZEROROW;

// Set up grease area - one row for now
    gra=malloc(ds->nob);
    grk=malloc(dkt.nob);

// Gaussian part.  Full Echelon in one pass.
    poolsize=0;
    DSSet(f,0,&dkts);   // just for compiler warnings
    for(i=0;i<nor;i++)   // processing the i'th row
    {
        va=a +i*ds->nob;
        vk=kt+i*dkt.nob;
        for(j=0;j<poolsize;j++)    // not executed yet
        {
            wa= a+poolrows[j]*ds->nob;
            wk=kt+poolrows[j]*dkt.nob;  // big stride
            col=poolpivs[j];
            fel=DUnpak(ds,col,va);
            DSMad(ds,fel,1,wa,va);
            DSMad(&dkt,fel,1,wk,vk); // should be small vector
        }
        col=DNzl(ds,va);              // is there a pivot?
        if(col==ZEROROW) continue;    // no - on with next row
        fel=DUnpak(ds,col,va);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(ds,fel,1,va);           // normalize vector
        rank++;
        DSSet(f,rank,&dkts);
        DPak(&dkt,rank,vk,1);
        DSMul(&dkts,fel,1,vk);         // and same normalize to keeptrack
        deter=FieldMul(f,fel,deter);  // update determinant
        BSBitSet(rs,i);               // i'th row is pivotal
        BSBitSet(cs,col);             // col'th column is pivotal
        pivs[i]=col;                  // update global arrays
        colrow[col]=i;
//  Back Clean the rest of the pool with that pivot
        for(j=0;j<poolsize;j++)
        {
            wa=a +poolrows[j]*ds->nob;
            wk=kt+poolrows[j]*dkt.nob;  // big stride
            fel=DUnpak(ds,col,wa);
            DSMad(ds,fel,1,va,wa);
            DSMad(&dkt,fel,1,vk,wk);   // should be small vector
        }
// put the new row into poolrows/cols keeping cols in order
        for(j=0;j<poolsize;j++) if(poolpivs[j]>col) break;
        for(k=poolsize;k>j;k--)
        {
            poolrows[k]=poolrows[k-1];
            poolpivs[k]=poolpivs[k-1];
        }
        poolrows[j]=i;
        poolpivs[j]=col;
        poolsize++;
        while(poolsize!=0)
        {
// choose a batch = first row = row 0
// grease up a batch = copy to gra, grk
            memcpy(gra,a +poolrows[0]*ds->nob,ds->nob);
            memcpy(grk,kt+poolrows[0]*dkt.nob,dkt.nob);
// prepare extraction
            col=poolpivs[0];
            for(j=0;j<nor;j++)
            {
                if(j==i) continue;   //don't clean pool itself
                fel=DUnpak(ds,col,a+j*ds->nob);
                DSMad(ds,fel,1,gra,a+j*ds->nob);
                DSMad(&dkts,fel,1,grk,kt+j*dkt.nob);
            }
        }
    }

    rs[1]=rank;    // complete bitstrings with number
    cs[1]=rank;    //     of set bits

// output multiplier
    mm=malloc(rank*dkts.nob);
    for(i=0;i<ds->noc;i++)
    {
        orow=0;
        if(colrow[i]==ZEROROW) continue;
        memcpy(mm+orow*dkts.nob,kt+colrow[i]*dkt.nob,dkts.nob);
    }
    *m=mm;
    free(kt);
    *det=deter;
    return rank;
}

/* end of dech.c */
