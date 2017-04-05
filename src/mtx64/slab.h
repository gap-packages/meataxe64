/*
         slab.h  -   Slab Routines Header
         ======      R. A. Parker 12.5.2016
*/

extern void SLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
                  Dfmt * c, uint64 nora, uint64 noca, uint64 nocb);

extern void SLTra(const FIELD *f, const Dfmt *a, Dfmt *b,
                  uint64 nora, uint64 noca);

extern uint64 SLEch(const FIELD *f, Dfmt *a, uint64 *rs, uint64 *cs,
                    Dfmt *m, Dfmt *c, Dfmt *r, uint64 nor, uint64 noc);

/* internal interface - called from linf */

extern void PLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
                  Dfmt * c, uint64 nora, uint64 noca, uint64 nocb);

/* end of slab.h  */
