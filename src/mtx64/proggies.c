/*
      proggies.c     All the proggies
      ==========     R. A. Parker    10.8.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "field.h"
#include "tfarm.h"
#include "psn1.h"
#include "io.h"
#include "proggies.h"
#include "slab.h"
#include "bitstring.h"
#include "tuning.h"

M3  M3Cons(const char * fn)         // construct and set filename
{
    M3 a;
    a = malloc(sizeof(M3S));
    a->fn=malloc(strlen(fn)+1);
    strcpy(a->fn,fn);
    a->fl=NULL;
    a->silent=0;
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

void M3Same (M3 a, M3 b)            // get field fmoj nor noc
{
    a->fmoj=b->fmoj;
    a->fdef = b->fdef;
    a->nor = b->nor;
    a->noc = b->noc;
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

void M3EvenChop (M3 a)            // Implement chopping strategy
{
    uint64_t def,rem,i;
/*  Then allocate the sizes lists  */
    a->rnor=malloc(a->r*sizeof(uint64_t));
    a->cnoc=malloc(a->c*sizeof(uint64_t));
/*  Then share out nor equally  */
    def=a->nor/a->r;
    rem=a->nor-(def*a->r);
    for(i=0;i<a->r;i++)
    {
        a->rnor[i]=def;
        if(rem==0) continue;
        rem--;
        a->rnor[i]++;
    }
    def=a->noc/a->c;
    rem=a->noc-(def*a->c);
    for(i=0;i<a->c;i++)
    {
        a->cnoc[i]=def;
        if(rem==0) continue;
        rem--;
        a->cnoc[i]++;
    } 
}

void M3CopyChop (M3 a, M3 b)  // copy chopping from b to a
{
    uint64_t i;
    a->r=b->r;
    a->c=b->c;
    a->rnor=malloc(a->r*sizeof(uint64_t));
    a->cnoc=malloc(a->c*sizeof(uint64_t));
    for(i=0;i<a->r;i++) a->rnor[i]=b->rnor[i];
    for(i=0;i<a->c;i++) a->cnoc[i]=b->cnoc[i];
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
    uint64_t i,j,colsf,maxnor;
    EFIL * ef;

    a=(M3) pp;
    f=TFPointer(a->fmoj);
    ef=ERHdr(a->fn,hdr);
    DSSet(f,a->noc,&dsin);
    if(a->c==1)
    {
        for(i=0;i<a->r;i++)
        {
            if(a->fl!=NULL) TFFlowWait(a->fl);
            bufout=TFAllocate( a->m[i][0] , a->rnor[i]*dsin.nob );
            stt=(uint64_t *) bufout;
            stt[0]=a->rnor[i];
            stt[1]=a->noc;
            ERData(ef,a->rnor[i]*dsin.nob,bufout+16);
            TFStable(a->m[i][0]);
            TFRelease(a->m[i][0]);
        }
    }
    else
    {
/* ought to do sanity checks */
        maxnor=0;
        for(i=0;i<a->r;i++) if(a->rnor[i]>maxnor) maxnor=a->rnor[i];
        bufin=malloc(maxnor*dsin.nob);

        for(i=0;i<a->r;i++)
        {
            if(a->fl!=NULL) TFFlowWait(a->fl);
            ERData(ef,a->rnor[i]*dsin.nob,bufin);
            colsf=0;
            for(j=0;j<a->c;j++)
            {
                DSSet(f,a->cnoc[j],&dsout);
                bufout=TFAllocate( a->m[i][j] , a->rnor[i]*dsout.nob );
                stt=(uint64_t *) bufout;
                stt[0]=a->rnor[i];
                stt[1]=a->cnoc[j];
                DCut(&dsin, a->rnor[i], colsf, bufin, &dsout, bufout+16);
                colsf+=a->cnoc[j];
                TFStable(a->m[i][j]);
                TFRelease(a->m[i][j]);
            }
        }
        free(bufin);
    }
    ERClose1(ef,a->silent);
    TFDownJobs();
    return pp;    // never used!
}

void * M3WriteThd(void *pp)
{
    uint64_t hdr[5];
    FIELD * f;
    uint64_t i,j,maxnor,colsf;
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
        memset(bufout,0,dsout.nob*maxnor);
        colsf=0;
        for(j=0;j<a->c;j++)
        {

            TFWait(a->m[i][j]);
            bufin=TFPointer(a->m[i][j]);
            DSSet(f,a->cnoc[j],&dsin);
            DPaste(&dsin,bufin+16,a->rnor[i],colsf,&dsout,bufout);
            colsf+=a->cnoc[j];
            TFRelease(a->m[i][j]);
        }
        EWData(ef,dsout.nob*a->rnor[i],bufout);
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
    uint64 *a,*b,*c;
    Dfmt *da, *db, *dc;
    uint64 nor,noc;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    b=(uint64 *) TFPointer(BMOJ);
    if( (a[0]!=b[0]) || (a[1]!=b[1]) )
    {
        printf("Add with incompatible matrices %lu %lu + %lu %lu\n",
                          a[0],a[1],b[0],b[1]);
        exit(22);
    }
    nor=a[0];
    noc=a[1];
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
    uint64 *a,*b,*c;
    Dfmt *da, *db, *dc;
    uint64 nor1,noc1,noc2;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    b=(uint64 *) TFPointer(BMOJ);
    if(a[1]!=b[0])
    {
        printf("Mul with incompatible matrices %lu %lu x %lu %lu\n",
                          a[0],a[1],b[0],b[1]);
        exit(22);
    }
    nor1=a[0];
    noc1=a[1];
    noc2=b[1];
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

/* moj adder 2 operands */
/* cmoj += amoj         */
/* then Stable and Release cmoj  */

void genadd(MOJ FMOJ,  MOJ AMOJ, MOJ CMOJ)
{
    FIELD * f;
    uint64 *a,*c;
    Dfmt *da, *dc;
    uint64 nor,noc;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    c=(uint64 *) TFPointer(CMOJ);
    if( (a[0]!=c[0]) || (a[1]!=c[1]) )
    {
        printf("Add with incompatible matrices %lu %lu + %lu %lu\n",
                          a[0],a[1],c[0],c[1]);
        exit(22);
    }
    nor=a[0];
    noc=a[1];
    da=(Dfmt *) a;
    dc=(Dfmt *) c;
    DSSet(f,noc,&ds);
    da+=16;
    dc+=16;
    DAdd(&ds,nor,da,dc,dc);
}

void gencpy(MOJ FMOJ,  MOJ AMOJ, MOJ CMOJ)
{
    FIELD * f;
    uint64 *a,*c;
    uint64 nor,noc;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    nor=a[0];
    noc=a[1];
    DSSet(f,noc,&ds);
    c=TFAllocate(CMOJ,nor*ds.nob+16);
    memcpy(c,a,16+nor*ds.nob);
}

void pgmul(MOJ FMOJ, MOJ AMOJ, MOJ BMOJ, MOJ CMOJ)
{
    genmul(FMOJ,AMOJ,BMOJ,CMOJ);
    TFStable(CMOJ);
}



void pgmad(MOJ FMOJ,  MOJ AMOJ, MOJ BMOJ, MOJ CMOJ, MOJ SMOJ)
{
    genmul(FMOJ,AMOJ,BMOJ,SMOJ);
    genadd(FMOJ,CMOJ,SMOJ);
    TFStable(SMOJ);
}

static void gentra(MOJ FMOJ,  MOJ AMOJ, MOJ BMOJ)
{
    FIELD * f;
    uint64 *a,*b;
    Dfmt *da, *db;
    uint64 nor,noc;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    b=(uint64 *) TFPointer(BMOJ);
    if( (a[0]!=b[1]) || (a[1]!=b[0]) )
    {
        printf("Tra with incompatible matrices %lu %lu %lu %lu\n",
               a[0],a[1],b[0],b[1]);
        exit(22);
    }
    nor=a[0];
    noc=a[1];
    da=(Dfmt *) a;
    db=(Dfmt *) b;
    da+=16;
    db+=16;

    SLTra(f, da, db, nor, noc); /* Transpose a slab */
    TFStable(BMOJ);
}

static void pgtra(MOJ FMOJ, MOJ AMOJ, MOJ BMOJ)
{
    FIELD * f;
    uint64 *a,*b;
    DSPACE ds;

    f=(FIELD *)  TFPointer(FMOJ);
    a=(uint64 *) TFPointer(AMOJ);
    DSSet(f,a[0],&ds); /* a[0] == b[1] columns */
    b=TFAllocate(BMOJ,a[1]*ds.nob+16); /* nor(b)*nob(b)+header */
    b[0]=a[1]; /* nor(b) = noc(a) */
    b[1]=a[0]; /* noc(b) = nor(a) */
    memset(b+2,0,a[1]*ds.nob); /* Clear the matrix of a[1] = b[0] rows */
    gentra(FMOJ,AMOJ,BMOJ);
}

static void pgcex(MOJ FMOJ, MOJ BSMOJ, MOJ MATMOJ, MOJ SELMOJ, MOJ NONSELMOJ)
{
  FIELD *f;
  uint64 *sel, *nonsel;
  const uint64 *bs, *mat;
  DSPACE ds_mat, ds_sel, ds_nonsel;
  uint64 nor, noc;
  Dfmt *d_mat, *d_sel, *d_nonsel;

  f = (FIELD *)TFPointer(FMOJ);
  bs = (uint64 *)TFPointer(BSMOJ);
  mat = (uint64 *)TFPointer(MATMOJ);
  nor = mat[0];
  noc = mat[1];
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
  memset(sel + 2, 0, nor * ds_sel.nob);
  memset(nonsel + 2, 0, nor * ds_nonsel.nob);
  d_mat = (Dfmt *)(mat + 2);
  d_sel = (Dfmt *)(sel + 2);
  d_nonsel = (Dfmt *)(nonsel + 2);
  BSColSelect(f, bs, nor, d_mat, d_sel, d_nonsel);
  TFStable(SELMOJ);
  TFStable(NONSELMOJ);
}

static void pgrex(MOJ FMOJ, MOJ BSMOJ, MOJ MATMOJ, MOJ SELMOJ, MOJ NONSELMOJ)
{
  FIELD *f;
  uint64 *sel, *nonsel;
  const uint64 *bs, *mat;
  DSPACE ds_mat;
  uint64 nor, noc, nor1, nor2, r;
  Dfmt *d_mat, *d_sel, *d_nonsel;

  f = (FIELD *)TFPointer(FMOJ);
  bs = (uint64 *)TFPointer(BSMOJ);
  mat = (uint64 *)TFPointer(MATMOJ);
  nor = mat[0];
  noc = mat[1];
  DSSet(f, noc, &ds_mat);
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
  memset(sel + 2, 0, nor1 * ds_mat.nob);
  memset(nonsel + 2, 0, nor2 * ds_mat.nob);
  d_mat = (Dfmt *)(mat + 2);
  d_sel = (Dfmt *)(sel + 2);
  d_nonsel = (Dfmt *)(nonsel + 2);
  for (r = 0; r < nor; r++) {
    int set = BSBitRead (bs, r);
    if (set) {
      /* Copy row r to sel */
      memcpy(d_sel, d_mat, ds_mat.nob);
      d_sel += ds_mat.nob;
    } else {
      /* Copy row r to nonsel */
      memcpy(d_nonsel, d_mat, ds_mat.nob);
      d_nonsel += ds_mat.nob;
    }
    d_mat += ds_mat.nob;
  }
  TFStable(SELMOJ);
  TFStable(NONSELMOJ);
}

/* Like REX, but combine rather than separate */
static void pgrrf(MOJ FMOJ, MOJ BSMOJ, MOJ SELMOJ, MOJ UMOJ, MOJ MATMOJ)
{
  FIELD *f;
  const uint64  *bs, *sel, *u;
  uint64 *mat;
  DSPACE ds_mat;
  uint64 nor, noc, nor1, nor2, r;
  Dfmt *d_mat, *d_sel, *d_u;

  f = (FIELD *)TFPointer(FMOJ);
  bs = (uint64 *)TFPointer(BSMOJ);
  sel = (uint64 *)TFPointer(SELMOJ);
  u = (uint64 *)TFPointer(UMOJ);
  nor1 = sel[0];
  nor2 = u[0];
  nor = nor1 + nor2;
  noc = sel[1];
  DSSet(f, noc, &ds_mat);
  /* Output has full set of columns */
  mat = TFAllocate(MATMOJ, nor * ds_mat.nob+16);
  mat[0] = nor;
  mat[1] = noc;
  memset(mat + 2, 0, nor * ds_mat.nob);
  d_mat = (Dfmt *)(mat + 2);
  d_sel = (Dfmt *)(sel + 2);
  d_u = (Dfmt *)(u + 2);
  for (r = 0; r < nor; r++) {
    int set = BSBitRead(bs, r);
    if (set) {
      /* Copy row r from sel */
      memcpy(d_mat, d_sel, ds_mat.nob);
      d_sel += ds_mat.nob;
    } else {
      /* Copy row r from u */
      memcpy(d_mat, d_u, ds_mat.nob);
      d_u += ds_mat.nob;
    }
    d_mat += ds_mat.nob;
  }
  TFStable(MATMOJ);
}

/* PVC, a bit of an oddball */
static void pgpvc(MOJ BSPMOJ, MOJ BSQMOJ, MOJ BSP1MOJ, MOJ BSUMOJ)
{
  const uint64 *bsp, *bsq;
  uint64 *bsp1, *bsu;
  uint64 nor, noc, noc1, noc2;
  uint64 bsp1_size, bsu_size;

  /* Get pointers to the inputs */
  bsp = (uint64 *)TFPointer(BSPMOJ);
  bsq = (uint64 *)TFPointer(BSQMOJ);
  /* Compute the parameters */
  nor = bsp[0]; /* Total bits in p */
  noc1 = bsp[1]; /* Bits set in p */
  noc2 = bsq[1]; /* Bits set in q */
  noc = noc1 + noc2; /* Bits set in p1, also total bits in u */
  /* Compute sizes */
  bsp1_size = (sizeof(uint64)) * (2 + (nor + 63) / 64);
  bsu_size = (sizeof(uint64)) * (2 + (noc + 63) / 64);
  /* Allocate p1 and u */
  bsp1 = TFAllocate(BSP1MOJ, bsp1_size);
  bsu = TFAllocate(BSUMOJ, bsu_size);
  memset(bsp1, 0, bsp1_size);
  memset(bsu, 0, bsu_size);
  /* Now combine the pivots */
  BSCombine(bsp, bsq, bsp1, bsu);
  TFStable(BSP1MOJ);
  TFStable(BSUMOJ);
}

static void pgech(MOJ FMOJ, MOJ AMOJ, MOJ RSMOJ, MOJ CSMOJ,
                   MOJ MMOJ, MOJ CMOJ, MOJ RMOJ)
{
    FIELD *f;
    uint64 nor,noc,rank;
    uint64 *ah,*mh,*ch,*rh, *rsp,*csp;
    Dfmt *am,*mm,*cm,*rm;
    DSPACE dsa;
    size_t z;

    f  = (FIELD *) TFPointer(FMOJ);
    am = (Dfmt *)  TFPointer(AMOJ);
    ah=(uint64 *) am;
    nor=ah[0];
    noc=ah[1];
    DSSet(f,noc,&dsa);
    z=16+8*((nor+63)/64);
    rsp=TFAllocate(RSMOJ,z);
    z=16+8*((noc+63)/64);
    csp=TFAllocate(CSMOJ,z);
    mm=TFAllocate(MMOJ,16+nor*dsa.nob);   // don't need all this
    cm=TFAllocate(CMOJ,16+nor*dsa.nob);   // don't need all this either
    rm=TFAllocate(RMOJ,16+nor*dsa.nob);
    
    rank=SLEch(f,am+16,rsp,csp,mm+16,cm+16,rm+16,nor,noc);

    mh=(uint64 *) mm;
    ch=(uint64 *) cm;
    rh=(uint64 *) rm;
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
  gencpy(FMOJ, AMOJ, BMOJ);
  TFStable(BMOJ);
}

void tfdo(int proggyno, MOJ *p)
{
  switch (proggyno) {
  case MULPROG:
    pgmul(p[0],p[1],p[2],p[3]);        // multiply
    break;

  case MADPROG:
    pgmad(p[0],p[1],p[2],p[3],p[4]);   // multiply and add
    break;

  case TRAPROG:
    pgtra(p[0],p[1],p[2]);   // transpose
    break;

  case CEXPROG:
    pgcex(p[0], p[1], p[2], p[3], p[4]);   /* pivot extract */
    break;

  case REXPROG:
    pgrex(p[0], p[1], p[2], p[3], p[4]);   /* row extract */
    break;

  case RRFPROG:
    pgrrf(p[0], p[1], p[2], p[3], p[4]);   /* row riffle */
    break;

  case PVCPROG:
    pgpvc(p[0], p[1], p[2], p[3]);   /* pivot combine */
    break;

  case ADDPROG:
    pgadd(p[0], p[1], p[2], p[3]);   /* matrix add */
    break;

  case ECHPROG:
    pgech(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);   /* echelise */
    break;

  case MCPPROG:
    pgcpy(p[0], p[1], p[2]);   /* copy */
    break;

  case FMVPROG:
    pgfmv(p[0], p[1], p[2]);   /* flow controlled "copy" */
    break;

  default:
    fprintf(stderr, "Unknown proggy number %d, terminating\n", proggyno);
    exit(1);
  }
}

/******  end of proggies.c    ******/
