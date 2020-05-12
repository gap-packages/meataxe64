/*   Memory functions.c  */
/*   RAP 4.7.18 */
// very draft implementation


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "io1.h"
#include "tfarm.h"
#include "bitstring.h"
#include "proggies.h"
#include "memfuns.h"
#include "slab.h"
#include "tuning.h"
#include "funs.h"
#include "funs1.h"

#define DEBUG 0


void mfMultiply(uint64_t threads, const FIELD * f, 
          const Dfmt * a, const Dfmt * b, Dfmt * c,
          uint64_t nora, uint64_t noca, uint64_t nocb)
{
       M4 A,B,C;
       MOJ fmoj,X,Y,Z;
       uint64_t i,j,k;
       uint64_t caul,base;
       int concur;
       uint64_t D1,D2;
       TFPM * tfpm;
       tfpm=TFParms();
       tfpm->threads = threads;
       TFInit(tfpm); /*  Start Thread farm */
       TFStopMOJFree();

       A=M4Cons((Dfmt *)a,f,nora, noca);
       B=M4Cons((Dfmt *)b,f,noca, nocb);   
       C=M4Cons(c,f,nora, nocb);   
       fmoj = A->fmoj;

       // Now we have to decide how to chop.
       // This should maybe be in a functions shared with mmul
       // How much concurrency do we want
       concur=20;
       if(concur<2*(threads+3)) concur=2*(threads+3);
       // What's the smallest size we are willing to have 
       D1=800;
       if(f->fdef>65536) D1=1400/(f->pow+1);
       if( (f->fdef<=16) && (f->charc<5) ) D1=8000;

       D2=D1;
       // try chopping into pieces of size D2
       while(1)
           {
               A->r=(A->nor+D2-1)/D2;
               A->c=(A->noc+D2-1)/D2;
               B->c=(B->noc+D2-1)/D2;
               if((A->r*A->c*B->c) < 3*concur) break;
               D2=D2+D2/5;
           }

       // Tidy up extreme cases
       if(A->c > MAXCHOP) A->c=MAXCHOP;
       if(A->r > MAXCHOP) A->r=MAXCHOP;
       if(B->c > MAXCHOP) B->c=MAXCHOP;
       if(A->c==0) A->c=1;
       if(A->r==0) A->r=1;
       if(B->c==0) B->c=1;
       
       B->r=A->c;
       C->r=A->r;
       C->c=B->c;

       // figure out the actual chop points
       caul=f->entbyte*8;
       if( (f->cauldron!=0) && ((f->pow==1) || (f->linfscheme!=0)) )  //HPMI
           {
               caul=f->cauldron;
               base=C->noc/(B->c * caul);
               if(base>=2) caul=caul*2;
               if(base>=6) caul=caul*2;
               if(base>=16) caul=caul*2;
           }
       M4EvenChop(A,8,f->entbyte*8,1,1); 
       M4EvenChop(B,f->entbyte*8,caul,1,1);
       M4EvenChop(C,8,caul,1,1);


       #if DEBUG
printf("%lu %ld  chops %lu %lu %lu\n",
              A->f->charc, A->nor, A->c,A->r,C->c);
printf(" Arow");
for(i=0;i<A->r;i++) printf(" %ld",A->rnor[i]);
printf(" Acol");
for(i=0;i<A->c;i++) printf(" %ld",A->cnoc[i]);
printf(" Brow");
for(i=0;i<B->r;i++) printf(" %ld",B->rnor[i]);
printf(" Bcol");
for(i=0;i<B->c;i++) printf(" %ld",B->cnoc[i]);
printf(" Crow");
for(i=0;i<C->r;i++) printf(" %ld",C->rnor[i]);
printf(" Ccol");
for(i=0;i<C->c;i++) printf(" %ld",C->cnoc[i]);
printf("\n");
#endif

       // Create all the tile reference MOJs
       M4MOJsStable(A);
       M4MOJsStable(B);
       M4MOJsStable(C);

       // submit all the jobs
       for(i=0;i<A->r;i++)
           for(k=0;k<B->c;k++)
               {
                   TML(i,fmoj,A->m[i][0],B->m[0][k],X);
                   if (A->c > 1) {
                       for(j=1;j<A->c-1;j++)
                           {
                               TML(i,fmoj,A->m[i][j],B->m[j][k],Y);
                               ADD(i,fmoj,X,Y,Z);
                               X=Z;
                           }
                       TML(i, fmoj, A->m[i][j], B->m[j][k], Y);
                       ADT(i, fmoj, X, Y , C->m[i][k]);
                   } else {
                       CPT(i, fmoj, X, C->m[i][k]);
                   }
               }

       // Now do the work 
       TFStartMOJFree();
       TFWaitEnd();
       
       // and clean up
       M4Dest(A);
       M4Dest(B);
       M4Dest(C);
       TFClose();
}

// caller is responsible for making sure shapes are compatible

// direct lifts from the slab layer
uint64_t mfSizeM(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return SLSizeM(f, nor, noc);
}

uint64_t mfSizeC(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return SLSizeC(f, nor, noc);
}

uint64_t mfSizeR(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return SLSizeR(f, nor, noc);
}

//  Put into NREF (Negative Reduced Echelon Form) with 
//  Transformation matrix

uint64_t mfFullEchelize(uint64_t threads, const DSPACE * ds,
              const Dfmt *a, uint64_t *rs, uint64_t *cs, 
              FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    return 0;
}

//  As above, but no transformation matrix (faster)

uint64_t mfProduceNREF(uint64_t threads, const DSPACE * ds,
              const Dfmt *a, uint64_t *rs, uint64_t *cs, 
              FELT * det, Dfmt *r, uint64_t nor)
{
    return 0;
}

//  As above, but no back-cleaning (faster still)

uint64_t mfRank(uint64_t threads, const DSPACE * ds, const Dfmt *a,
              uint64_t *rs, uint64_t *cs, FELT * det, uint64_t nor)
{
    return 0;
}

//  Write data out in meataxe64 format

int mfWrite( const void *data, const char *path, 
                  const uint64_t nob, const uint64_t *header)
{
    EFIL *ef = EWHdr(path, header);
    EWData(ef, nob, data);
    EWClose(ef);
    return 0;
}

//  Read header of meataxe64 standard file

int mfReadHeader(const char *path, uint64_t *header)
{
    EPeek(path, header);
    return 0;
}

//  Read a meataxe64 standard file

int mfReadData(const char *path, uint64_t nob, void *data)
{
    uint64_t hdr[6];
    EFIL *ef = ERHdr(path, hdr);
    ERData(ef, nob, data);
    ERClose(ef);
    return 0;
}

// Things lifted from D-format layer for isolation

// May be worth parallelising over some fields and on some hardware
void mfAdd(const DSPACE *ds, const Dfmt *a, const Dfmt *b, 
               Dfmt *c, uint64_t nor)
{
    DAdd(ds, nor, a, b, c);
}

void mfSub(const DSPACE *ds, const Dfmt *a, const Dfmt *b,
               Dfmt *c, uint64_t nor)
{
    DSub(ds, nor, a, b, c);
}

void mfSMul(const DSPACE *ds, const Dfmt *a, FELT x, uint64_t nor, Dfmt *b)
{
    if (a != b)
        memmove(b, a, ds->nob*nor);
    DSMul( ds, x, nor, b);
}

void mfSMad(const DSPACE *ds, Dfmt *a, const Dfmt *b,
             FELT x, uint64_t nor)
{
    DSMad(ds, x, nor, b, a);
}

// Multi-field functions, not provided at D format layer

void mfFrobenius(const DSPACE * ds, const Dfmt *m1, Dfmt *m2, uint64_t nor)
{
    EFIL *e1, *e2;
    uint64_t header[6];
    e1 = ERHdrD(ds, m1, nor, header);
    e2 = EWHdrD(m2, header);
    fFrobenius1(e1, e2, ds->f->fdef, nor, ds->noc, 0, 0);
}

int  mfFieldContract(DSPACE *ds1, const Dfmt *m1,
               const DSPACE *ds2, Dfmt *m2, uint64_t nor)
{
    EFIL *e1, *e2;
    uint64_t header[6];
    e1 = ERHdrD(ds1, m1, nor, header);
    header[1] = ds2->f->fdef;
    e2 = EWHdrD(m2, header);
    return fFieldContract1(e1, e2, ds1->f->fdef, ds2->f->fdef, nor, ds1->noc, 0, 0);
}

void mfFieldExtend(const DSPACE *ds1, const Dfmt *m1,
            const DSPACE *ds2, Dfmt *m2, uint64_t nor)
{
    EFIL *e1, *e2;
    uint64_t header[6];
    e1 = ERHdrD(ds1, m1, nor, header);
    header[1] = ds2->f->fdef;
    e2 = EWHdrD(m2, header);
    fFieldExtend1(e1, e2, ds1->f->fdef, ds2->f->fdef, nor, ds1->noc, 0, 0);
}

void mfBloat(const DSPACE *ds1, const Dfmt *m1, const DSPACE *ds2,
            Dfmt *m2, uint64_t nor)
{
}

// Tensor and related

void mfTensor( const DSPACE *ds1, const Dfmt *m1, uint64_t nor1,
       const DSPACE *ds2, const Dfmt *m2, uint64_t nor2,  Dfmt *m3)
{
}

void mfExteriorSquare(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2)
{
}

void mfExteriorCube(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2)
{
}

void mfSymmetricSquare(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2)
{
}

// spinning

uint64_t mfInvariantSubspace(uint64_t threads, const DSPACE *ds,
               const Dfmt *seeds, uint64_t numSeeds, const Dfmt **gens,
               uint64_t numGens, Dfmt *space)
{
    return 0;
}

uint64_t mfStandardBase(uint64_t threads, const DSPACE *ds,
               const Dfmt * seed, const Dfmt **gens, uint64_t numGens,
               Dfmt *base)
{
    return 0;
}

// characteristic polynomial routines

uint64_t mfCharPoly(uint64_t threads, const DSPACE *ds,
               const Dfmt *m, uint64_t nor, Dfmt *polys,
               uint64_t *polydegs)
{
    return 0;
}

uint64_t mfMinPoly(uint64_t threads, const DSPACE *ds,
               const Dfmt *m, uint64_t nor, Dfmt *polys,
               uint64_t *polydegs)
{
    return 0;
}

/* end of memfuns.c  */


