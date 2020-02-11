extern "C" {
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "fphpmi.h"
}

template<typename IT, typename OT>
static inline uint64_t normalize(const IT *in, OT *__restrict__ out, uint64_t num_entries, OT p, IT p2) {
    IT nonsparse = 0;
    for (uint64_t i = 0; i < num_entries; i++) {
        IT x = in[i];
        nonsparse |= x;
        // next line needs to do unsigned comparison
        // then signed subtraction -- care is needed
        out[i] = (x > p2) ? ((OT)x - p) : (OT)x;
    }
    return nonsparse ? 1 : 0;
}

uint64_t normalize_8_24(const uint8_t *in, int16_t * out, int16_t p, uint8_t p2) {
    return normalize<uint8_t, int16_t>(in, out, 24, p, p2);
}

uint64_t normalize_8_160(const uint8_t *in, int16_t * out, int16_t p, uint8_t p2) {
    return normalize<uint8_t, int16_t>(in, out, 160, p, p2);
}

uint64_t normalize_8(const uint8_t *in, int16_t * out, uint64_t num_entries, int16_t p, uint8_t p2) {
    return normalize<uint8_t, int16_t>(in, out, num_entries, p, p2);
}

uint64_t normalize_16_24(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2) {
    return normalize<uint16_t, int16_t>(in, out, 24, p, p2);
}

uint64_t normalize_16_80(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2) {
    return normalize<uint16_t, int16_t>(in, out, 80, p, p2);
}

uint64_t normalize_16_160(const uint16_t *in, int16_t * out, int16_t p, uint16_t p2) {
    return normalize<uint16_t, int16_t>(in, out, 160, p, p2);
}

uint64_t normalize_16(const uint16_t *in, int16_t * out, uint64_t num_entries, int16_t p, uint16_t p2) {
    return normalize<uint16_t, int16_t>(in, out, num_entries, p, p2);
}

uint64_t normalize_32_24(const uint32_t *in, int32_t * out, int32_t p, uint32_t p2) {
    return normalize<uint32_t, int32_t>(in, out, 24, p, p2);
}

uint64_t normalize_32_80(const uint32_t *in, int32_t * out, int32_t p, uint32_t p2) {
    return normalize<uint32_t, int32_t>(in, out, 80, p, p2);
}


uint64_t normalize_32(const uint32_t *in, int32_t * out, uint64_t num_entries, int32_t p, uint32_t p2) {
    return normalize<uint32_t, int32_t>(in, out, num_entries, p, p2);
}
