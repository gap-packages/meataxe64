/*
         bitstring.h  -   Bit String Operations header
         ===========      R. A. Parker 7.7.2014
*/

#define ENDGPC 0xffffffff

extern int  BSBitRead (const uint64_t *bs, uint64_t bitno);
extern void BSBitSet (uint64_t * bs, uint64_t bitno);
extern uint32_t * BSGpc(const uint64_t * bs); // caller must free result
extern uint32_t * BSLixUn(const uint64_t * bs); // caller frees
extern void BSColSelect (const FIELD * f, const uint64_t * bs, uint64_t nor,
                  const Dfmt * d, Dfmt * sel, Dfmt * nonsel);
extern void BSColRifZ (const FIELD * f, const uint64_t * bs, uint64 nor,
                  const Dfmt * mtxin, Dfmt * mtxout);
extern void BSColPutS (const FIELD * f, const uint64_t * bs, uint64 nor,
                       FELT scalar, Dfmt * dfmt);   // in situ
extern void BSCombine (const uint64_t * bs1, const uint64_t * bs2,
                  uint64_t * comb, uint64_t * rif);
extern void BSShiftOr (const uint64_t * bs1, uint64_t shift, uint64_t * bs2);
extern void BSMkr(const uint64_t * lit, const uint64_t * big, uint64_t * rif);

/* end of bitstring.h */
