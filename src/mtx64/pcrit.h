/*    Meataxe-64    pcrit.h     */
/*    ==========    ========     */

/*    R. A. Parker      13.Feb.2017 */


extern void pchal(FIELD * f);
extern uint64_t pcstride(uint64_t s);
extern uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
extern void pcxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void pcbif(Dfmt * d, const Dfmt * s1, const Dfmt * s2,
                   uint64_t nob, const uint8_t * t2);
extern void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2);
extern void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1);
extern void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1);

void pcbm2(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pcad3(const uint8_t *a, const uint8_t *b, uint8 * c);
void pcbm3(const uint8_t *a, uint8_t * bv, uint8_t *c);

/* end of pcrit.h  */
