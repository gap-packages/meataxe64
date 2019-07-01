/*
      proggies.c     All the proggies
      ==========     R. A. Parker    16.5.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "field.h"
#include "tfarm.h"
#include "io.h"
#include "proggies.h"
#include "slab.h"
#include "bitstring.h"
#include "tuning.h"
#include "pcrit.h"

// #define DEBUG 1
// #define DEBUG1 1

M3  M3Cons(const char * fn, int sil)  // construct and set filename
{
    M3 a;
    a = malloc(sizeof(M3S));
    a->fn=malloc(strlen(fn)+1);
    strcpy(a->fn,fn);
    a->fl=NULL;
    a->silent=sil;
    return a;
}

void M3Peek (M3 a)            // peek, get fdef nor,noc
{
    uint64_t hdr[5];
    EPeek(a->fn, hdr);
    a->fdef = hdr[1];
    a->nor = hdr[2];
    a->noc = hdr[3];
}

MOJ M3FieldMOJ (uint64_t fdef)          // make field moj
{
    FIELD * f;
    MOJ mj;
    mj=TFNewMOJ();
    f = (FIELD *) TFAllocate(mj, FIELDLEN);
    TFGetReadRef(mj);
    FieldSet(fdef,f);
    TFQuickReady(mj);
    return mj;
}

void M3EvenChop (M3 a, uint64_t divr, uint64_t divc, 
                 uint64_t padr, uint64_t padc)   
{
    uint64_t def,rem,i;
/*  Then allocate the sizes lists  */
    a->rnor=malloc(a->r*sizeof(uint64_t));
    a->cnoc=malloc(a->c*sizeof(uint64_t));
/*  Then share out nor equally  */
    def=a->nor/a->r;
    def=(def/divr)*divr;
    rem=a->nor-(def*a->r);
// so def is basic local size and rem is remainder to distribute
    for(i=0;i<a->r;i++)
    {
        a->rnor[i]=def;
        if(rem>=divr)
        {
            a->rnor[i]+=divr;
            rem-=divr;
        }
    }
    if((rem%padr)!=0) a->rextra=padr-(rem%padr);
          else        a->rextra=0;
    rem+=a->rextra;
    a->rnor[a->r-1]+=rem;
/*  similarly noc  */
    def=a->noc/a->c;
    def=(def/divc)*divc;
    rem=a->noc-(def*a->c);

    for(i=0;i<a->c;i++)
    {
        a->cnoc[i]=def;
        if(rem>=divc)
        {
            a->cnoc[i]+=divc;
            rem-=divc;
        }
    }
    if((rem%padc)!=0) a->cextra=padc-(rem%padc);
          else        a->cextra=0;
    rem+=a->cextra;
    a->cnoc[a->c-1]+=rem;
}

void M3MOJs  (M3 a)            // allocate MOJs
{
    int i,j;
    a->m=malloc(a->r*sizeof(MOJ*));
    for(i=0;i<a->r;i++)
    {
        *(a->m+i) = malloc(a->c*sizeof(MOJ));
        for(j=0;j<a->c;j++)
        {
            a->m[i][j]=TFNewMOJ();
            TFGetReadRef(a->m[i][j]);
        }
    }
}

void M3MOJArray  (M3 a)            // allocate MOJ array but not MOJs
{
    int i;
    a->m=malloc(a->r*sizeof(MOJ*));
    for(i=0;i<a->r;i++)
        *(a->m+i) = malloc(a->c*sizeof(MOJ));
}


void * M3ReadThd(void * pp)
{
    uint64_t hdr[5];
    FIELD * f;
    DSPACE dsin, dsout;
    Dfmt *bufin, *bufout;
    M3 a;
    uint64_t * stt;
    uint64_t i,j,colsf,maxnor,rowread,rowzero;
    EFIL * ef;

    a=(M3) pp;
    f=TFPointer(a->fmoj);
    ef=ERHdr(a->fn,hdr);
    DSSet(f,a->noc,&dsin);
// possible later optimization to avoid move if 1 column block and no padding
/* ought to do sanity checks */
    maxnor=0;
    for(i=0;i<a->r;i++) if(a->rnor[i]>maxnor) maxnor=a->rnor[i];
    bufin=malloc(maxnor*dsin.nob);

    for(i=0;i<a->r;i++)
    {
        if(a->fl!=NULL) TFFlowWait(a->fl);
        if((i+1)!=(a->r)) rowread=a->rnor[i];
             else         rowread=a->rnor[i]-a->rextra;
        rowzero=a->rnor[i]-rowread;
        ERData(ef,rowread*dsin.nob,bufin);
        colsf=0;
        for(j=0;j<a->c;j++)
        {
            DSSet(f,a->cnoc[j],&dsout);
            bufout=TFAllocate( a->m[i][j] , 16 + (a->rnor[i]*dsout.nob) );
            stt=(uint64_t *) bufout;
            stt[0]=a->rnor[i];
            stt[1]=a->cnoc[j];
            DCut(&dsin, rowread, colsf, bufin, &dsout, bufout+16);
            if(rowzero!=0)
                memset(bufout+16+rowread*dsout.nob,0,rowzero*dsout.nob);
            colsf+=a->cnoc[j];
            TFStable(a->m[i][j]);
            TFRelease(a->m[i][j]);
        }
    }
    free(bufin);
    ERClose1(ef,a->silent);
    TFDownJobs();
    return pp;    // never used!
}

void * M3WriteThd(void *pp)
{
    uint64_t hdr[5];
    FIELD * f;
    uint64_t i,j,maxnor,colsf,rowwrite;
    DSPACE dsin,dsout;
    M3 a;
    EFIL * ef;
    Dfmt *bufin, *bufout;    

    a=(M3) pp;
    f=TFPointer(a->fmoj);
    hdr[0]=1;
    hdr[1]=f->fdef;
    hdr[2]=a->nor;
    hdr[3]=a->noc;
    hdr[4]=0;
    ef=EWHdr(a->fn,hdr);
    maxnor=0;
    for(i=0;i<a->r;i++) if(a->rnor[i]>maxnor) maxnor=a->rnor[i];
    DSSet(f,a->noc,&dsout);
    bufout=malloc(maxnor*dsout.nob);
    for(i=0;i<a->r;i++)
    {
        if((i+1)!=(a->r)) rowwrite=a->rnor[i];
             else         rowwrite=a->rnor[i]-a->rextra;
        memset(bufout,0,dsout.nob*rowwrite);
        colsf=0;
        for(j=0;j<a->c;j++)
        {
            TFWait(a->m[i][j]);
            bufin=TFPointer(a->m[i][j]);
            DSSet(f,a->cnoc[j],&dsin);
            DPaste(&dsin,bufin+16,rowwrite,colsf,&dsout,bufout);
            colsf+=a->cnoc[j];
            TFRelease(a->m[i][j]);
        }
        EWData(ef,dsout.nob*rowwrite,bufout);
        if(a->fl!=NULL) TFFlowUp(a->fl);
    }
    EWClose1(ef,a->silent);
    TFDownJobs();
    free(bufout);
    return pp;    // never used
}

void M3Read (M3 a)          // kick off read thread
{
    pthread_t th;
/* really ought to copy a, I think  */
    TFUpJobs();
    pthread_create(&th,NULL,M3ReadThd,(void *)a);
    pthread_detach(th);    // fire and forget
}

void M3Write(M3 a)          // kick off write thread
{
    uint64_t i,j;
    pthread_t th;
    TFUpJobs();
    for(i=0;i<a->r;i++)
      for(j=0;j<a->c;j++)
        TFGetReadRef(a->m[i][j]);
/* really ought to copy a, I think  */
    pthread_create(&th,NULL,M3WriteThd,(void *)a);
    pthread_detach(th);    // fire and forget
}

void M3Dest(M3 a)
{
    int i;
    free(a->fn);
    free(a->rnor);
    free(a->cnoc);
    for(i=0;i<a->r;i++) free(*(a->m+i));
    free(a->m);
    free(a);
}

void pgadd(MOJ FMOJ,  MOJ AMOJ, MOJ BMOJ, MOJ CMOJ)
{
    FIELD * f;
    uint64_t *a,*b,*c;
    Dfmt *da, *db, *dc;
    uint64_t nor,noc;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64_t *) TFPointer(AMOJ);
    b=(uint64_t *) TFPointer(BMOJ);
    if( (a[0]!=b[0]) || (a[1]!=b[1]) )
    {
        printf("Add with incompatible matrices %lu %lu + %lu %lu\n",
                          a[0],a[1],b[0],b[1]);
        exit(22);
    }
    nor=a[0];
    noc=a[1];
#ifdef DEBUG
printf("ADD %lu x %lu\n",nor,noc);
#endif
    DSSet(f,noc,&ds);
    c=TFAllocate(CMOJ,(nor*ds.nob)+16);
    c[0]=a[0];
    c[1]=a[1];
    da=(Dfmt *) a;
    db=(Dfmt *) b;
    dc=(Dfmt *) c;
    DSSet(f,noc,&ds);
    DAdd(&ds,nor,da+16,db+16,dc+16);
    TFStable(CMOJ);
}

/*  ================= clean up to here  ==============*/

void genmul(MOJ FMOJ,  MOJ AMOJ, MOJ BMOJ, MOJ CMOJ)
{

    FIELD * f;
    uint64_t *a,*b,*c;
    Dfmt *da, *db, *dc;
    uint64_t nor1,noc1,noc2;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64_t *) TFPointer(AMOJ);
    b=(uint64_t *) TFPointer(BMOJ);
    if(a[1]!=b[0])
    {
        printf("Mul with incompatible matrices %lu %lu x %lu %lu\n",
                          a[0],a[1],b[0],b[1]);
        exit(22);
    }
    nor1=a[0];
    noc1=a[1];
    noc2=b[1];
#ifdef DEBUG
printf(" %lu x %lu  X  %lu x %lu",nor1,noc1,noc1,noc2);
#endif
    da=(Dfmt *) a;
    db=(Dfmt *) b;
    DSSet(f,b[1],&ds);
    c=TFAllocate(CMOJ,a[0]*ds.nob+16);
    c[0]=a[0];
    c[1]=b[1];
    dc=(Dfmt *) c;
    da+=16;
    db+=16;
    dc+=16;

    SLMul(f,da,db,dc,nor1,noc1,noc2);
}

void gencpy(MOJ FMOJ,  MOJ AMOJ, MOJ CMOJ)
{
    FIELD * f;
    uint64_t *a,*c;
    uint64_t nor,noc;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64_t *) TFPointer(AMOJ);
    nor=a[0];
    noc=a[1];
#ifdef DEBUG
printf(" %lu x %lu\n",nor,noc);
#endif
    DSSet(f,noc,&ds);
    c=TFAllocate(CMOJ,nor*ds.nob+16);
    memcpy(c,a,16+nor*ds.nob);
}

void pgmul(MOJ FMOJ, MOJ AMOJ, MOJ BMOJ, MOJ CMOJ)
{
#ifdef DEBUG
printf("MUL ");
#endif
    genmul(FMOJ,AMOJ,BMOJ,CMOJ);
#ifdef DEBUG
printf("\n");
#endif
    TFStable(CMOJ);
}

void pgmad(MOJ FMOJ,  MOJ AMOJ, MOJ BMOJ, MOJ CMOJ, MOJ SMOJ)
{

    Dfmt * temp;
    uint64_t i;
    const FIELD * f;
    uint64_t *ai,*ci;
    Dfmt *a,*b,*c;
#ifdef DEBUG
printf("MAD ");
#endif
    f=(FIELD *)  TFPointer(FMOJ);
    a=TFPointer(AMOJ);
    b=TFPointer(BMOJ);
    c=TFPointer(CMOJ);
    ai=(uint64_t *)a;
    ci=(uint64_t *)c;
    i=SLSize(f,ci[0],ci[1]);
    temp=malloc(i);
    TFSetPtr(SMOJ,c);
    TFSetPtr(CMOJ,NULL);
    SLMad(f,a+16,b+16,temp,c+16,ci[0],ai[1],ci[1]);
    free(temp);
    TFStable(SMOJ);
}

static void pgcex(MOJ FMOJ, MOJ BSMOJ, MOJ MATMOJ,
                  MOJ SELMOJ, MOJ NONSELMOJ)
{
    FIELD *f;
    uint64_t *sel, *nonsel;
    const uint64_t *bs, *mat;
    DSPACE ds_mat, ds_sel, ds_nonsel;
    uint64_t nor, noc;
    Dfmt *d_mat, *d_sel, *d_nonsel;
 
    f = (FIELD *)TFPointer(FMOJ);
    bs = (uint64_t *)TFPointer(BSMOJ);
    mat = (uint64_t *)TFPointer(MATMOJ);
    nor = mat[0];
    noc = mat[1];
    if(noc!=bs[0])
    {
        printf("Invalid column extract - cols not equal\n");
        exit(14);
    }
    DSSet(f, noc, &ds_mat);
    DSSet(f, bs[1], &ds_sel); /* Number of set bits */
    DSSet(f, bs[0] - bs[1], &ds_nonsel); /* Number of unset bits */
    sel = TFAllocate(SELMOJ, nor * ds_sel.nob+16);
    nonsel = TFAllocate(NONSELMOJ, nor * ds_nonsel.nob+16);
    /* Set output parameters */
    sel[0] = nor;
    sel[1] = bs[1];
    nonsel[0] = nor;
    nonsel[1] = bs[0] - bs[1];
    d_mat = (Dfmt *)(mat + 2);
    d_sel = (Dfmt *)(sel + 2);
    d_nonsel = (Dfmt *)(nonsel + 2);
#ifdef DEBUG
printf("CEX %lu x %lu -> %lu %lu\n",nor,noc,sel[1],nonsel[1]);
#endif
    BSColSelect(f, bs, nor, d_mat, d_sel, d_nonsel);
    TFStable(SELMOJ);
    TFStable(NONSELMOJ);
}

static void pgrex(MOJ FMOJ, MOJ BSMOJ, MOJ MATMOJ,
                  MOJ SELMOJ, MOJ NONSELMOJ)
{
    FIELD *f;
    uint64_t *sel, *nonsel;
    const uint64_t *bs, *mat;
    DSPACE ds_mat;
    uint64_t nor, noc, nor1, nor2, r;
    Dfmt *d_mat, *d_sel, *d_nonsel;
    int set;

    f = (FIELD *)TFPointer(FMOJ);
    bs = (uint64_t *)TFPointer(BSMOJ);
    mat = (uint64_t *)TFPointer(MATMOJ);
    nor = mat[0];
    noc = mat[1];
    DSSet(f, noc, &ds_mat);
    if(nor!=bs[0])
    {
        printf("row extract with wrong number of rows\n");
        exit(14);
    }
    /* Set output parameters */
    nor1 = bs[1];
    nor2 = nor - bs[1];
    /* Both outputs have full set of columns */
    sel = TFAllocate(SELMOJ, nor1 * ds_mat.nob+16);
    nonsel = TFAllocate(NONSELMOJ, nor2 * ds_mat.nob+16);
    sel[0] = nor1;
    sel[1] = noc;
    nonsel[0] = nor2;
    nonsel[1] = noc;
#ifdef DEBUG
printf("REX %lu -> %lu %lu X %lu\n",nor,nor1,nor2,noc);
#endif
    d_mat = (Dfmt *)(mat + 2);
    d_sel = (Dfmt *)(sel + 2);
    d_nonsel = (Dfmt *)(nonsel + 2);
    for (r = 0; r < nor; r++)
    {
        set = BSBitRead (bs, r);
        if (set)
        {
            /* Copy row r to sel */
            memcpy(d_sel, d_mat, ds_mat.nob);
            d_sel += ds_mat.nob;
        }
        else
        {
            /* Copy row r to nonsel */
            memcpy(d_nonsel, d_mat, ds_mat.nob);
            d_nonsel += ds_mat.nob;
        }
        d_mat += ds_mat.nob;
    }
    TFStable(SELMOJ);
    TFStable(NONSELMOJ);
}

/* RRF.  Like rex, but combine rather than separate */
static void pgrrf(MOJ FMOJ, MOJ BSMOJ, MOJ SELMOJ, MOJ UMOJ,
                  MOJ MATMOJ)
{
    FIELD *f;
    const uint64_t  *bs, *sel, *u;
    uint64_t *mat;
    DSPACE ds_mat;
    uint64_t nor, noc, nor1, nor2, r;
    Dfmt *d_mat, *d_sel, *d_u;
    int set;

    f = (FIELD *)TFPointer(FMOJ);
    bs = (uint64_t *)TFPointer(BSMOJ);
    sel = (uint64_t *)TFPointer(SELMOJ);
    u = (uint64_t *)TFPointer(UMOJ);
    nor1 = sel[0];
    nor2 = u[0];
    nor = nor1 + nor2;
    noc = sel[1];
    if( (nor1!=bs[1]) || (nor2!=(bs[0]-bs[1])) )
    {
        printf("Wrong number of rows in row extract\n");
        exit(14);
    }
    if(noc!=u[1])
    {
        printf("Incompatible matrices in row extract\n");
        exit(14);
    }
#ifdef DEBUG
printf("RRF %lu + %lu -> %lu X %lu\n",nor1,nor2,nor,noc);
#endif
    DSSet(f, noc, &ds_mat);
    /* Output has full set of columns */
    mat = TFAllocate(MATMOJ, nor * ds_mat.nob+16);
    mat[0] = nor;
    mat[1] = noc;
    memset(mat + 2, 0, nor * ds_mat.nob);
    d_mat = (Dfmt *)(mat + 2);
    d_sel = (Dfmt *)(sel + 2);
    d_u = (Dfmt *)(u + 2);
    for (r = 0; r < nor; r++)
    {
        set = BSBitRead(bs, r);
        if (set)
        {
            /* Copy row r from sel */
            memcpy(d_mat, d_sel, ds_mat.nob);
            d_sel += ds_mat.nob;
        }
        else
        {
            /* Copy row r from u */
            memcpy(d_mat, d_u, ds_mat.nob);
            d_u += ds_mat.nob;
        }
        d_mat += ds_mat.nob;
    }
    TFStable(MATMOJ);
}

/* Pivot combine P1,P2->P3,  riffle RF */
static void pgpvc(MOJ P1, MOJ P2, MOJ P3, MOJ RF)
{
    const uint64_t *p1, *p2;
    uint64_t *p3, *rf;
    uint64_t nor, noc1, noc2, noc3;
    uint64_t p3siz, rfsiz;

  /* Get pointers to the inputs */
    p1 = (uint64_t *)TFPointer(P1);
    p2 = (uint64_t *)TFPointer(P2);
  /* Compute the parameters */
    nor = p1[0]; /* Total bits in p1 */
    noc1 = p1[1]; /* Bits set in p1 */
    noc2 = p2[1]; /* Bits set in p2 */
    noc3 = noc1 + noc2; /* Bits set in p3, also total bits in rf */
    if(p2[0]!=(nor-noc1))
    {
        printf("Incompatible bit strings in PVC\n");
        exit(14);
    }
#ifdef DEBUG
printf("PVC %lu  %lu + %lu -> %lu\n",nor,noc1,noc2,noc3);
#endif
  /* Compute sizes */
    p3siz = (sizeof(uint64_t)) * (2 + (nor + 63) / 64);
    rfsiz = (sizeof(uint64_t)) * (2 + (noc3 + 63) / 64);
  /* Allocate p3 and rf */
    p3 = TFAllocate(P3, p3siz);
    rf = TFAllocate(RF, rfsiz);
  /* Now combine the pivots */
    BSCombine(p1, p2, p3, rf);
    TFStable(P3);
    TFStable(RF);
}

/* Pivot combine P2->P3,  riffle RF with no P1 start */
static void pgpc0(MOJ P2, MOJ P3, MOJ RF)
{
    const uint64_t *p2;
    uint64_t *p3, *rf;
    uint64_t nor, noc;
    uint64_t p3siz, rfsiz;

  /* Get pointers to the input */
    p2 = (uint64_t *)TFPointer(P2);
  /* Compute the parameters */
    nor = p2[0]; /* Total bits in p1=p2 */
    noc = p2[1]; /* Bits set in p2 */
#ifdef DEBUG
printf("PC0 %lu  %lu\n",nor,noc);
#endif
  /* Compute sizes */
    p3siz = (sizeof(uint64_t)) * (2 + (nor + 63) / 64);
    rfsiz = (sizeof(uint64_t)) * (2 + (noc + 63) / 64);
  /* Allocate p3 and rf */
    p3 = TFAllocate(P3, p3siz);
    rf = TFAllocate(RF, rfsiz);
    memcpy(p3, p2, p3siz);
    memset(rf, 0, rfsiz);
    rf[0]=noc;
    rf[1]=0;
    TFStable(P3);
    TFStable(RF);
}

static void pgech(MOJ FMOJ, MOJ AMOJ, MOJ RSMOJ, MOJ CSMOJ,
                   MOJ MMOJ, MOJ CMOJ, MOJ RMOJ)
{
    FIELD *f;
    uint64_t nor,noc,rank;
    uint64_t *ah,*mh,*ch,*rh, *rsp,*csp;
    Dfmt *am,*mm,*cm,*rm;
    size_t z;
    uint64_t det;
    DSPACE ds;
    f  = (FIELD *) TFPointer(FMOJ);
    am = (Dfmt *)  TFPointer(AMOJ);
    ah=(uint64_t *) am;
    nor=ah[0];
    noc=ah[1];
    z=16+8*((nor+63)/64);
    rsp=TFAllocate(RSMOJ,z);
    z=16+8*((noc+63)/64);
    csp=TFAllocate(CSMOJ,z);
    mm=TFAllocate(MMOJ,16+SLSizeM(f,nor,noc));
    cm=TFAllocate(CMOJ,16+SLSizeC(f,nor,noc));
    rm=TFAllocate(RMOJ,16+SLSizeR(f,nor,noc));
    DSSet(f,noc,&ds);
// determinant discarded at the moment
    rank=SLEch(&ds,am+16,rsp,csp,&det,mm+16,cm+16,rm+16,nor);
#ifdef DEBUG
printf("ECH %lu x %lu rank %lu\n",nor,noc,rank);
#endif

    mh=(uint64_t *) mm;
    ch=(uint64_t *) cm;
    rh=(uint64_t *) rm;
    mh[0]=rank;
    mh[1]=rank;
    ch[0]=nor-rank;
    ch[1]=rank;
    rh[0]=rank;
    rh[1]=noc-rank;
    TFStable(RSMOJ);
    TFStable(CSMOJ);
    TFStable(MMOJ);
    TFStable(CMOJ);
    TFStable(RMOJ);
}

void pgfmv(MOJ AMOJ, MOJ FMOJ, MOJ BMOJ)
{
    uint64_t * a;
    a=(uint64_t *) TFPointer(AMOJ);
    TFSetPtr(BMOJ,a);
    TFSetPtr(AMOJ,NULL);
    TFFlowUp(FMOJ);
    TFStable(BMOJ);
}

void pgcpy(MOJ FMOJ, MOJ AMOJ, MOJ BMOJ)
{
#ifdef DEBUG
printf("MCP ");
#endif
  gencpy(FMOJ, AMOJ, BMOJ);
  TFStable(BMOJ);
}

void pgcrz(MOJ FMOJ, MOJ BS, MOJ IN, MOJ OUT)
{
    uint64_t * bs;
    uint64_t nor,nocin,nocout;
    FIELD * f;
    DSPACE dsout;
    uint64_t *mxin,*mxout;

    f  = (FIELD *) TFPointer(FMOJ);
    bs   = (uint64_t *)TFPointer(BS);   // bit string in
    mxin = (uint64_t *)TFPointer(IN);   // matrix in
    nor=mxin[0];
    nocin=mxin[1];
    if(nocin!=bs[1])
    {
        printf("CRZ with with incompatible bitstring %lu %lu\n",
               nocin,bs[1]);
        exit(22);
    }
    nocout=bs[0];
    DSSet(f, nocout, &dsout);
    mxout = TFAllocate(OUT, (nor*dsout.nob)+16);
    mxout[0]=nor;
    mxout[1]=nocout;
#ifdef DEBUG
printf("CRZ %lu x %lu -> %lu\n",nor,nocin,nocout);
#endif
    BSColRifZ(f,bs,nor,(Dfmt *)(mxin+2),(Dfmt *)(mxout+2));
    TFStable(OUT);
}

void pgadi(MOJ FMOJ, MOJ RF, MOJ IN, MOJ OUT)
{
    uint64_t *rf;
    FIELD * f;
    DSPACE dsout;
    uint64_t nor,nocout;
    uint64_t *mxin,*mxout;

    f    = (FIELD *)   TFPointer(FMOJ);
    rf   = (uint64_t *)TFPointer(RF);   // Col Select bit string in
    mxin = (uint64_t *)TFPointer(IN);   // matrix in
    nor=mxin[0];
    nocout=rf[0];
    DSSet(f, nocout, &dsout);
    mxout = TFAllocate(OUT, (nor*dsout.nob)+16);
    mxout[0]=nor;    // not needed
    mxout[1]=nocout; // not needed
    memcpy(mxout,mxin,16+nor*dsout.nob);
#ifdef DEBUG
printf("ADI %lu x %lu, + %lu identity\n",nor,nocout,rf[0]-rf[1]);
#endif
    BSColPutS(f,rf,nor,1,(Dfmt *)(mxout+2));

    TFStable(OUT);
}

void pgmkr(MOJ LIT, MOJ BIG, MOJ RES)
{
    uint64_t *lit,*big,*res;

    lit=(uint64_t *)TFPointer(LIT);
    big=(uint64_t *)TFPointer(BIG);
    if( lit[0] != big[0] )
    {
        printf("MKR with bitstrings of different lengths %lu %lu\n",
                                                   lit[0],big[0]);
        exit(22);
    }
    res=TFAllocate(RES, ((((big[1]+63)/64)+2)*8) );
#ifdef DEBUG
printf("MKR %lu x %lu -> %lu\n",lit[0],lit[1],big[1]);
#endif
    BSMkr(lit,big,res);
    TFStable(RES);
}

void dump(MOJ *p, int ct)
{
    int i;
    printf("\n");
    for(i=0;i<=ct;i++)
    {
        printf("%3ld ",(((long)p[i])/8)&511);
    }
    printf("\n");
}

void tfdo(int proggyno, MOJ *p)
{
  switch (proggyno) {
  case MULPROG:
#ifdef DEBUG1
    dump(p,3);
#endif
    pgmul(p[0],p[1],p[2],p[3]);        // multiply
    break;

  case MADPROG:
#ifdef DEBUG1
    dump(p,4);
#endif
    pgmad(p[0],p[1],p[2],p[3],p[4]);   // multiply and add
    break;

  case CEXPROG:
#ifdef DEBUG1
    dump(p,4);
#endif
    pgcex(p[0], p[1], p[2], p[3], p[4]);   /* pivot extract */
    break;

  case REXPROG:
#ifdef DEBUG1
    dump(p,4);
#endif
    pgrex(p[0], p[1], p[2], p[3], p[4]);   /* row extract */
    break;

  case RRFPROG:
#ifdef DEBUG1
    dump(p,4);
#endif
    pgrrf(p[0], p[1], p[2], p[3], p[4]);   /* row riffle */
    break;

  case PVCPROG:
#ifdef DEBUG1
    dump(p,3);
#endif
    pgpvc(p[0], p[1], p[2], p[3]);   /* pivot combine */
    break;

  case ADDPROG:
#ifdef DEBUG1
    dump(p,3);
#endif
    pgadd(p[0], p[1], p[2], p[3]);   /* matrix add */
    break;

  case ECHPROG:
#ifdef DEBUG1
    dump(p,6);
#endif
    pgech(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);   /* echelise */
    break;

  case MCPPROG:
#ifdef DEBUG1
    dump(p,2);
#endif
    pgcpy(p[0], p[1], p[2]);   /* copy */
    break;

  case FMVPROG:
#ifdef DEBUG1
    dump(p,2);
#endif
    pgfmv(p[0], p[1], p[2]);   /* flow controlled "copy" */
    break;

  case CRZPROG:
#ifdef DEBUG1
    dump(p,3);
#endif
    pgcrz(p[0], p[1], p[2], p[3]);   /* Column Riffle with zero */
    break;

  case ADIPROG:
#ifdef DEBUG1
    dump(p,3);
#endif
    pgadi(p[0], p[1], p[2], p[3]);   /* Plonk in identity under zeros */
    break;

  case MKRPROG:
#ifdef DEBUG1
    dump(p,2);
#endif
    pgmkr(p[0], p[1], p[2]);   /* Make Riffle little big riffle */
    break;

  case PC0PROG:
#ifdef DEBUG1
    dump(p,2);
#endif
    pgpc0(p[0], p[1], p[2]);   /* pivot combine with nothing */
    break;

  default:
    fprintf(stderr, "Unknown proggy number %d, terminating\n", proggyno);
    exit(1);
  }
}

/******  end of proggies.c    ******/
