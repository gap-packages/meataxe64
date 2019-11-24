/*   Memory functions.c  */
/*   RAP 4.7.18 */
// very draft implementation


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "bitstring.h"
#include "memfuns.h"

void mfMultiply(uint64_t threads, const FIELD * f, 
          const Dfmt * a, const Dfmt * b, Dfmt * c,
          uint64_t nora, uint64_t noca, uint64_t nocb)
{
}

// caller is responsible for making sure shapes are compatible

// direct lifts from the slab layer
uint64_t mfSizeM(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return 0;
}

uint64_t mfSizeC(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return 0;
}

uint64_t mfSizeR(const FIELD * f, uint64_t nor, uint64_t noc)
{
    return 0;
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
    return 0;
}

//  Read header of meataxe64 standard file

int mfReadHeader(const char *path, uint64_t *header)
{
    return 0;
}

//  Read a meataxe64 standard file

int mfReadData(const char *path, uint64_t nob, void *data)
{
    return 0;
}

// Things lifted from D-format layer for isolation

void mfAdd(const DSPACE *ds, const Dfmt *a, const Dfmt *b, 
               Dfmt *c, uint64_t nor)
{
}

void mfSub(const DSPACE *ds, const Dfmt *a, const Dfmt *b,
               Dfmt *c, uint64_t nor)
{
}

void mfSMul(const DSPACE *ds, const Dfmt *a, FELT x, uint64_t nor)
{
}

void mfSMad(const DSPACE *ds, Dfmt *a, const Dfmt *b,
             FELT x, uint64_t nor)
{
}

// Multi-field functions, not provided at D format layer

void mfFrobenius(const DSPACE * ds, const Dfmt *m1, Dfmt *m2, uint64_t nor)
{
}

int  mfFieldContract(DSPACE *ds1, const Dfmt *m1,
               const DSPACE *ds2, Dfmt *m2, uint64_t nor)
{
    return 0;
}

void mfFieldExtend(const DSPACE *ds1, const Dfmt *m1,
            const DSPACE *ds2, Dfmt *m2, uint64_t nor)
{
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
