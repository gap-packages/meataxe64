/*
         bitstring.h  -   Bit String Operations header
         ===========      R. A. Parker 7.7.2014
*/

#define ENDGPC 0xffffffff

extern uint32_t * BSGpc(const uint64_t * bs); // caller must free result
extern uint32_t * BSLix(const uint64_t * bs, int flag); // caller frees
extern void BSColSelect (const FIELD * f, const uint64 * bs, uint64 nor,
                  const Dfmt * d, Dfmt * sel, Dfmt * nonsel);
extern void BSColRifZ (const FIELD * f, const uint64_t * bs, uint64 nor,
                  const Dfmt * mtxin, Dfmt * mtxout);
extern void BSColRifS (const FIELD * f, const uint64_t * bs, uint64 nor,
             int flag, FELT scalar, const Dfmt * mtxin, Dfmt * mtxout);
extern void BSCombine (const uint64 * bs1, const uint64 * bs2,
                  uint64 * comb, uint64 * rif);
extern void BSShiftOr (const uint64 * bs1, uint64 shift, uint64 * bs2);
extern void BSBitSet (uint64 * bs, uint64 bitno);
extern int  BSBitRead (const uint64 *bs, uint64 bitno);
/* to be obsoleted */
extern void BSColRiffle (const FIELD * f, const uint64 * bs, uint64 nor,
                  const Dfmt * set,  const Dfmt * unset, Dfmt * rif);

/* end of bitstring.h */
