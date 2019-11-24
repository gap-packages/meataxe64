/* memfuns.h - header files for the "memory functions" layer. */
/*  S. A. Linton, R. A. Parker July 2019  */

// All parameter data structures must be allocated/freed by caller
// responsible for knowing how big they are, or might be.

extern void mfMultiply(uint64_t threads, const FIELD * f, 
          const Dfmt * a, const Dfmt * b, Dfmt * c,
          uint64_t nora, uint64_t noca, uint64_t nocb);

//  Compute the maximum sizes for the Gaussian results
//  M(ultiplier), C(leaner), R(emnant)

extern uint64_t mfSizeM(const FIELD * f, uint64_t nor, uint64_t noc);
extern uint64_t mfSizeC(const FIELD * f, uint64_t nor, uint64_t noc);
extern uint64_t mfSizeR(const FIELD * f, uint64_t nor, uint64_t noc);

//  Put into NREF (Negative Reduced Echelon Form) with 
//  Transformation matrix

extern uint64_t mfFullEchelize(uint64_t threads, const DSPACE * ds,
              const Dfmt *a, uint64_t *rs, uint64_t *cs, 
              FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor);

//  As above, but no transformation matrix (faster)

extern uint64_t mfProduceNREF(uint64_t threads, const DSPACE * ds,
              const Dfmt *a, uint64_t *rs, uint64_t *cs, 
              FELT * det, Dfmt *r, uint64_t nor);

//  As above, but no back-cleaning (faster still)

extern uint64_t mfRank(uint64_t threads, const DSPACE * ds, const Dfmt *a,
              uint64_t *rs, uint64_t *cs, FELT * det, uint64_t nor);

//  Write data out in meataxe64 format

extern int mfWrite( const void *data, const char *path, 
                  const uint64_t nob, const uint64_t *header);

//  Read header of meataxe64 standard file

extern int mfReadHeader(const char *path, uint64_t *header);

//  Read a meataxe64 standard file

extern int mfReadData(const char *path, uint64_t nob, void *data);

// Section 2 -- quadratic things. 

// Things lifted from D-format layer for isolation

extern void mfAdd(const DSPACE *ds, const Dfmt *a, const Dfmt *b,
               Dfmt *c, uint64_t nor);
extern void mfSub(const DSPACE *ds, const Dfmt *a, const Dfmt *b,
               Dfmt *c, uint64_t nor);
extern void mfSMul(const DSPACE *ds, const Dfmt *a, FELT x, uint64_t nor);
extern void mfSMad(const DSPACE *ds, Dfmt *a, const Dfmt *b, FELT x,
               uint64_t nor);

// Multi-field functions, not provided at D format layer

extern void mfFrobenius(const DSPACE * ds,
               const Dfmt *m1, Dfmt *m2, uint64_t nor);

extern int  mfFieldContract(DSPACE *ds1, const Dfmt *m1,
               const DSPACE *ds2, Dfmt *m2, uint64_t nor);

extern void mfFieldExtend(const DSPACE *ds1, const Dfmt *m1, 
               const DSPACE *ds2, Dfmt *m2,  uint64_t nor);

extern void mfBloat(const DSPACE *ds1, const Dfmt *m1,
               const DSPACE *ds2, Dfmt *m2,  uint64_t nor);

// Tensor and related

extern void mfTensor( const DSPACE *ds1, const Dfmt *m1, uint64_t nor1,
               const DSPACE *ds2, const Dfmt *m2, uint64_t nor2,  Dfmt *m3);

extern void mfExteriorSquare(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2);

extern void mfExteriorCube(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2);

extern void mfSymmetricSquare(const DSPACE *ds1, const Dfmt *m1,
               uint64_t nor1, Dfmt *m2);


// spinning

extern uint64_t mfInvariantSubspace(uint64_t threads, const DSPACE *ds,
               const Dfmt *seeds, uint64_t numSeeds, const Dfmt **gens,
               uint64_t numGens, Dfmt *space);

extern uint64_t mfStandardBase(uint64_t threads, const DSPACE *ds,
               const Dfmt * seed, const Dfmt **gens, uint64_t numGens,
               Dfmt *base);

// characteristic polynomial routines

extern uint64_t mfCharPoly(uint64_t threads, const DSPACE *ds,
               const Dfmt *m, uint64_t nor, Dfmt *polys,
               uint64_t *polydegs);

extern uint64_t mfMinPoly(uint64_t threads, const DSPACE *ds,
               const Dfmt *m, uint64_t nor, Dfmt *polys,
               uint64_t *polydegs);

/* end of memfuns.h  */
