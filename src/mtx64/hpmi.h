/*
         hpmi.h  -   High Performance Meataxe Interface Header
         ======      R. A. Parker     18.8.2017
*/

typedef uint8_t Afmt;
typedef uint8_t Bfmt;
typedef uint8_t Cfmt;

extern void DtoA(DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt * a,
                 uint64_t nora, uint64_t stride);
extern uint64_t DtoB(DSPACE * ds, const Dfmt * d, Bfmt * b,
                     uint64_t nor, uint64_t stride);
extern void CZer(DSPACE * ds, Cfmt * c, uint64_t nor);
extern void CtoD(DSPACE * ds, Cfmt * c, Dfmt * d, 
                 uint64_t nor, uint64_t stride);
extern void BwaInit(const FIELD *f, uint8_t *bwa);
extern void BrickMad(const FIELD *f, uint8_t * bwa, 
                     Afmt *a, Bfmt *b, Cfmt *c);
extern  int BSeed(const FIELD * f, uint8_t * bwa, Bfmt * b);
extern void BGrease(const FIELD * f, uint8_t * bwa, int sparsity);
extern void BwaMad(const FIELD *f, uint8_t * bwa, int sparsity,
                   Afmt *af, Cfmt *c);

/* end of hpmi.h  */
