#define restrict __restrict__


extern  void DtoA_fp(const DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt * a,
                     uint64_t nora, uint64_t stride);

extern uint64_t DtoB_fp(const DSPACE * ds, const Dfmt * d, Bfmt *restrict b, 
                 uint64_t nor, uint64_t stride);

extern  void CtoD_fp(const DSPACE * ds, const Cfmt * c, Dfmt *restrict d, uint64_t nor, uint64_t stride);

extern  void  BwaMad_fp(const FIELD *f, const Afmt *a, uint8_t *restrict b, Cfmt  *restrict c);
