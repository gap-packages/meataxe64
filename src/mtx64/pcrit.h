/*    Meataxe-64    pcrit.h     */
/*    ==========    ========     */

/*    R. A. Parker      2.11.2017 */

extern void mactype(char * mact);
extern void hpmiset(FIELD * f);
extern uint64_t pcstride(uint64_t s);
extern uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
extern void pcaxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void pcjxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void pcbif(Dfmt * d, const Dfmt * s1, const Dfmt * s2,
                   uint64_t nob, const uint8_t * t2);
extern void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2);
extern void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1);
extern void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1);

void pcab2(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pcjb2(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pcad3(const uint8_t *a, const uint8_t *b, uint8_t * c);
void pcab3(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pcjb3(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pcaas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pcdas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pcjas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pcjat(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pcdasc(const uint8_t *prog, uint8_t * bv, const uint64_t * parms);
void pccl32 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint32_t * d1, uint32_t * d2);
void pccl64 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint64_t * d1, uint64_t * d2);
// void pcbmdq(const uint8_t *a, uint8_t * bw, uint8_t *c,
//             const uint64_t * parms);

/* end of pcrit.h  */
