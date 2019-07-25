/*    Meataxe-64    pcrit.h     */
/*    ==========    ========     */

/*    R. A. Parker      20.3.2019 */

// pc1.s general functions
extern void mactype(char * mact);
void pccl32 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint32_t * d1, uint32_t * d2);
void pccl64 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint64_t * d1, uint64_t * d2);
extern void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2);
extern void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1);
extern void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1);
extern uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c);
extern uint64_t pcrem(uint64_t p,uint64_t a,uint64_t b);
extern void pc1xora(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void pc1xorj(Dfmt * d, const Dfmt * s1, const Dfmt * s2, uint64_t nob);
extern void pcbif(Dfmt * d, const Dfmt * s1, const Dfmt * s2,
                   uint64_t nob, const uint8_t * t2);
void pcbarprp(int inp, int oup, long base, int digits, 
              uint64_t maxval, uint64_t * barpar);
void pcbarrett(const uint64_t * barpar, const Dfmt * input, Dfmt * output,
                   uint64_t entries, uint64_t stride);

// pc2.s HPMI in characteristic 2
void pc2aca(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc2acj(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc2acm(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc2bma(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pc2bmj(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pc2bmm(const uint8_t *a, uint8_t * bv, uint8_t *c);

// pc3.s HPMI in characteristic 3
void pc3aca(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc3acj(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc3acm(const uint8_t *prog, uint8_t * bv, uint64_t stride);
void pc3bma(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pc3bmj(const uint8_t *a, uint8_t * bv, uint8_t *c);
void pc3bmm(const uint8_t *a, uint8_t * bv, uint8_t *c);

// pc5.s AS primes for characteristic 5-193
void pc5aca(const uint8_t *prog, uint8_t * bv, const uint64_t * parms);
void pc5bmwa(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pc5bmdd(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pc5bmwj(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pc5bmdj(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);
void pc5bmdm(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms);

// pc6.s scalar multiplication HPMI
void pc6bma(const uint8_t *a, uint8_t * bwa, uint8_t * c,
            uint64_t p90);

// in hpmi.c
extern void hpmiset(FIELD * f);
// in pcrit.c
extern uint64_t pcstride(uint64_t s);








/* end of pcrit.h  */
