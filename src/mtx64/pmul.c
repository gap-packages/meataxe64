/*
         pmul.c  -   Ground field slab-scale multiply
         ======      R. A. Parker 27.2.2018
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "hpmi.h"
#include "pmul.h"
#include "dfmtf.h"

//#define MAKTAB2

void PLMul(DSPACE *dsa, DSPACE *dsbc, uint64_t nora, 
          const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
          Dfmt * c)
{
    uint64_t alca,cauls;         // shape of the Bfmt
    uint64_t rowstodo, rowsleft, colstodo, colsleft;
    uint64_t box,boxes;
    uint64_t * ix;
    const Dfmt *ptb;
    DSPACE dsbf;
    Afmt *aa;
    Bfmt *bb;
    Cfmt *cc;
    uint8_t * bwa;
    uint64_t j,k;
    const FIELD *f;

    f=dsa->f;

/* check out the *do* *nothing* cases where C is zero bytes  */
    if(dsbc->noc==0) return;
    if(nora==0) return;
    if(dsa->noc==0)
    {
        memset(c,0,dsbc->nob*nora);
        return;
    }

    if(f->cauldron != 0)
    {
        // how many alcoves are there to do
        alca=(dsa->noc+f->alcove-1)/f->alcove;
        // how many cauldrons are there to do
        cauls=(dsbc->noc+f->cauldron-1)/f->cauldron;

/* Zeroize C */
        cc=AlignMalloc(f->cfmtcauld*cauls*nora);
        CZer(dsbc,cc,nora);

/* Convert B to Bfmt */
        bb=malloc(f->bbrickbytes*alca*cauls);
        rowsleft=dsa->noc;
        for(j=0;j<alca;j++)
        {
            rowstodo=rowsleft;
            if(rowstodo>f->alcove) rowstodo=f->alcove;
            ptb=b+j*bstride*f->alcove;
            colsleft=dsbc->noc;
            for(k=0;k<cauls;k++)
            {
                colstodo=colsleft;
                if(colstodo>f->cauldron) colstodo=f->cauldron;
                PSSet(f,colstodo,&dsbf);
                DtoB(&dsbf,
                     ptb+k*f->dfmtcauld,
                     bb+(k*alca+j)*f->bbrickbytes,
                     rowstodo,
                     bstride);
                colsleft-=colstodo;
            }
            rowsleft-=rowstodo;
        }

        boxes=(nora+f->recbox-1)/f->recbox;
        ix=malloc(alca*sizeof(uint64_t));
        aa=malloc(8+alca*(f->abase+f->recbox*f->alcovebytes));
        bwa=AlignMalloc(f->bwasize);
        BwaInit(f,bwa);
        for(box=0;box<boxes;box++)
        {

/* convert box of rows to Afmt */
            rowstodo=nora-box*f->recbox;
            if(rowstodo>f->recbox) rowstodo=f->recbox;
            DtoA(dsa,
                 ix,
                 a+box*f->recbox*astride,
                 aa,
                 rowstodo,
                 astride);

/*  Do the matrix multiplication  */
            for(k=0;k<cauls;k++)
            {
                for(j=0;j<alca;j++)
                {
                    BrickMad(f,
                             bwa,
                             aa+j*(f->abase+rowstodo*f->alcovebytes),
                             bb+(k*alca+j)*f->bbrickbytes,
                             cc+(box*f->recbox+k*nora)*f->cfmtcauld
                            );
#ifdef DEBUG
for(rowstodo=0;rowstodo<256;rowstodo++)
{
    uint64_t k1;
    int i1,i4,i25;
    unsigned int i2,i3;
    k1=*((uint64_t *)(bwa+128*rowstodo));
    i1=rowstodo;
    i2=k1&1023;
    i25=(k1>>10)&1;
    i3=(k1>>11)&1023;
    i4=(k1>>21)&2047;
    printf("%3d %03x %d %03x %04d\n",i1, i2, i25, i3, i4);
    if((rowstodo%8)==7) printf("\n");
}
#endif
#ifdef MAKTAB
uint8_t p1[1024],p2[1024];
uint32_t q1,q2,r1;
for(q1=0;q1<1024;q1++)
{
    p1[q1]=255;
    p2[q1]=255;
}
for(q1=0;q1<256;q1++)
{
  for(q2=0;q2<=q1;q2++)
  {
    r1=*((uint32_t *)(bwa+128*q1));
    r1^=*((uint32_t *)(bwa+128*q2));
    r1=r1&1023;
    if(p1[r1]!=255) continue;
    p1[r1]=q1;
    p2[r1]=q2;
  }
}
for(q1=0;q1<1024;q1++)
{
    printf(" 0x%02x, 0x%02x,",p1[q1],p2[q1]);
    if((q1%4)==3) printf("\n");
}
#endif
#ifdef MAKTAB2
uint32_t q1,r1;
for(q1=0;q1<256;q1++)
{
    r1=*((uint32_t *)(bwa+128*q1));
    printf(" 0x%08x,",r1);
    if((q1%4)==3) printf("\n");
}
#endif
                }
            }
        }
        AlignFree(bwa);
        free(aa);
        free(bb);
        free(ix);
        CtoD(dsbc,cc,c,nora,dsbc->nob);
        AlignFree(cc);
        return;
    }

/* if there is no HPMI, do it by steam over ground field */
    DFMul(dsa,dsbc,nora,a,astride,b,bstride,c);
}

/* end of pmul.c  */
