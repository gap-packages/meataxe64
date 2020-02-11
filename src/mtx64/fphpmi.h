#define restrict __restrict__


extern  void DtoA_fp(const DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt * a,
                     uint64_t nora, uint64_t stride);

extern uint64_t DtoB_fp(const DSPACE * ds, const Dfmt * d, Bfmt *restrict b, 
                 uint64_t nor, uint64_t stride);

extern  void CtoD_fp(const DSPACE * ds, const Cfmt * c, Dfmt *restrict d, uint64_t nor, uint64_t stride);

extern  void  BwaMad_fp(const FIELD *f, const Afmt *a, uint8_t *restrict b, Cfmt  *restrict c);


// functions in fphpmi1.cc -- workaround for C compiler bug

extern uint64_t normalize_8_24(const uint8_t *in, int16_t * out, int16_t p, uint8_t p2);

extern uint64_t normalize_8_160(const uint8_t *in, int16_t * out, int16_t p, uint8_t p2);

extern uint64_t normalize_8(const uint8_t *in, int16_t * out, uint64_t num_entries, int16_t p, uint8_t p2);

extern uint64_t normalize_16_24(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2);


extern uint64_t normalize_16_80(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2);

extern uint64_t normalize_16_160(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2);

extern uint64_t normalize_16(const uint16_t *in, int16_t * out, uint64_t num_entries, int16_t p, uint16_t p2);

extern uint64_t normalize_32_24(const uint32_t *in, int32_t * out, int32_t p, uint32_t p2);

extern uint64_t normalize_32_80(const uint32_t *in, int32_t * out, int32_t p, uint32_t p2);


extern uint64_t normalize_32(const uint32_t *in, int32_t * out, uint64_t num_entries, int32_t p, uint32_t p2);
