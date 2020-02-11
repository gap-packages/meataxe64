// Copyright (C) Steve Linton 2019
//  fphpmi.cc Meataxe64 floating point based HPMI

extern "C" {
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include "field.h"
#include "hpmi.h"
#include "fphpmi.h"
}

#define ALCOVE 24
#define CAULDRON_bytes 640
#define CAULDRON_sp (CAULDRON_bytes / sizeof(float))
#define CAULDRON_dp (CAULDRON_bytes / sizeof(double))

/************************************************************************************
 *          Utility
 ***********************************************************************************/

// convert unsigned integers of type IT to signed integers of type OT in range -p/2..p/2
//  with sparseness tracking
// these may merit some assembler for some of the cases.

// rather ugly workaround for a bug in gcc up to about version 7.4
// if we allow this code to be inlined gcc generates an invalid aligned move instruction
// which causes a GPF.

// It is expected that, on the fast-paths all of the conditionals in this function will be
// resolved at compile-time, so there will just be one call to function from fphpmi1.cc

template<typename OT, typename IT>
static inline uint64_t normalize(const IT *in, OT *restrict out, uint64_t num_entries, OT p, IT p2) {
    if (sizeof(IT) == 1) {
        if (num_entries == 24)
            return normalize_8_24((const uint8_t *)in, (int16_t *)out, p, p2);
        if (num_entries == 160)
            return normalize_8_160((const uint8_t *)in, (int16_t *)out, p, p2);
        return normalize_8((const uint8_t *)in, (int16_t *)out, num_entries, p, p2);
    }
    if (sizeof(IT) == 2) {
        if (num_entries == 24)
            return normalize_16_24((const uint16_t *)in, (int16_t *)out, p, p2);
        if (num_entries == 80)
            return normalize_16_80((const uint16_t *)in, (int16_t *)out, p, p2);
        if (num_entries == 160)
            return normalize_16_160((const uint16_t *)in, (int16_t *)out, p, p2);
        return normalize_16((const uint16_t *)in, (int16_t *)out, num_entries, p, p2);
    }
    if (num_entries == 24)
        return normalize_32_24((const uint32_t *)in, (int32_t *)out, p, p2);
    if (num_entries == 80)
        return normalize_32_80((const uint32_t *)in, (int32_t *)out, p, p2);
    return normalize_32((const uint32_t *)in, (int32_t *)out, num_entries, p, p2);      
}

/************************************************************************************
 *          DtoA
 ***********************************************************************************/

// Access macros for the A format
// access the skip byte in alcove j
#define aix_skip(j) (a[ix[(j)]])
// access the k,l entry in the current boxlet x alcove piece of alcove j
#define aix_entry(j, k, l)                                                     \
  (*(ABT *)(&(a[ix[(j)] + 1 + sizeof(ABT) * (ALCOVE * (l) + (k))])))
// Step down to next block of A format.
#define aix_step (1 + sizeof(ABT) * BOXLET * ALCOVE)

// Inner part of DtoA, converts a boxlet x alcove piece of alcove j
// width and height indicate how much "real" matrix there is, the rest will be
// padding This only exists as a separate routine so that calls with
// compile-time constant width and/or height can be inlined and benefit from
// constant propagation

template <typename DT, typename ABT, int BOXLET>
void DtoA1(Afmt *restrict a, uint64_t *restrict ix, uint64_t j, uint64_t width,
           uint64_t height, const Dfmt *restrict d, uint64_t stride, uint64_t p,
           uint64_t p2) {
  DT nonsparse = 0;
  ABT *aa = (ABT *)(a + 1 + ix[j]);
  for (uint64_t l = 0; l < height; l++) {
      nonsparse |= normalize<ABT, DT>((DT *)d, aa + l*ALCOVE, width, (DT)p, (ABT)p2);
    for (uint64_t k = width; k < ALCOVE; k++) {
      aix_entry(j, k, l) = 0;
    }
    d += stride;
  }
  for (uint64_t l = height; l < BOXLET; l++)
    for (uint64_t k = 0; k < ALCOVE; k++)
      aix_entry(j, k, l) = 0;
  if (nonsparse || (aix_skip(j) == 250)) {
    ix[j] += aix_step;
    aix_skip(j) = 0;
  } else
    aix_skip(j)++;
}

// The main DtoA template. fastpaths the full boxlet x alcove pieces

template <typename DT, typename ABT, int BOXLET>
void DtoA_fp_template(const DSPACE *ds, uint64_t *ix, const Dfmt *d,
                      Afmt *restrict a, uint64_t nora, uint64_t stride) {
  const FIELD *f = ds->f;

  uint64_t mainboxlets = nora / BOXLET;
  uint64_t lastboxlet = (nora % BOXLET) > 0;
  uint64_t mainalcoves = ds->noc / ALCOVE;
  uint64_t lastalcove = (ds->noc % ALCOVE) > 0;
  uint64_t p2 = f->charc / 2;
  uint64_t p = f->charc;

  for (uint64_t i = 0; i < mainboxlets; i++) {
    const Dfmt *d0 = d;
    for (uint64_t j = 0; j < mainalcoves; j++) {
      DtoA1<DT, ABT, BOXLET>(a, ix, j, ALCOVE, BOXLET, d, stride, p,
                             p2); // fastpath
      d += sizeof(DT) * ALCOVE;
    }
    if (lastalcove) {
      DtoA1<DT, ABT, BOXLET>(a, ix, mainalcoves, ds->noc % ALCOVE, BOXLET, d,
                             stride, p, p2);
    }
    d0 += BOXLET * stride;
    d = d0;
  }
  if (lastboxlet) {
    for (uint64_t j = 0; j < mainalcoves; j++) {
      DtoA1<DT, ABT, BOXLET>(a, ix, j, ALCOVE, nora % BOXLET, d, stride, p, p2);
      d += sizeof(DT) * ALCOVE;
    }
    if (lastalcove) {
      DtoA1<DT, ABT, BOXLET>(a, ix, mainalcoves, ds->noc % ALCOVE,
                             nora % BOXLET, d, stride, p, p2);
    }
  }
}

// DtoA dispatch -- instantiate the template based on field and machine and call
// it
void DtoA_fp(const DSPACE *ds, uint64_t *ix, const Dfmt *d, Afmt *restrict a,
             uint64_t nora, uint64_t stride) {
  const FIELD *f = ds->f;
  switch (f->pbytesper) {
  case 1:
    if (f->mact[0] >= 'm')
      DtoA_fp_template<uint8_t, int16_t, 5>(ds, ix, d, a, nora, stride);
    else
      DtoA_fp_template<uint8_t, int16_t, 3>(ds, ix, d, a, nora, stride);
    break;
  case 2:
    if (f->mact[0] >= 'm')
      DtoA_fp_template<uint16_t, int16_t, 5>(ds, ix, d, a, nora, stride);
    else
      DtoA_fp_template<uint16_t, int16_t, 3>(ds, ix, d, a, nora, stride);
    break;
  case 4:
    if (f->mact[0] >= 'm')
      DtoA_fp_template<uint32_t, int32_t, 5>(ds, ix, d, a, nora, stride);
    else
      DtoA_fp_template<uint32_t, int32_t, 3>(ds, ix, d, a, nora, stride);
    break;
  }
}

/************************************************************************************
 *          DtoB
 ***********************************************************************************/

template <typename DT, typename ABT, int CAULDRON>
uint64_t DtoB_fp_template(const DSPACE *ds, const Dfmt *d, Bfmt *restrict b,
                          uint64_t nor, uint64_t stride) {
  const FIELD *f = ds->f;
  DT p = f->charc;
  DT p2 = p / 2;
  DT nonsparse = 0;
  DT *d0 =  (DT *)d;
  ABT *b0 = (ABT *)(b + 1);
  if (ds->noc == CAULDRON && nor == ALCOVE) {
    // fast-path compile time known loop lengths
    for (uint64_t i = 0; i < ALCOVE; i++) {
        nonsparse |= normalize<ABT, DT>(d0, b0, CAULDRON, p, (ABT)p2);
        d0 += stride / sizeof(DT);
        b0 += CAULDRON;
    }
  } else {
    // slow path for bricks on the right or bottom of the matrix
    for (uint64_t i = 0; i < nor; i++) {
        nonsparse |= normalize<ABT, DT>(d0, b0, ds->noc, p, (ABT)p2);
        memset(b0+ds->noc, 0, sizeof(ABT) * (CAULDRON - ds->noc));
        b0 += CAULDRON;
        d0 += stride / sizeof(DT);
    }
    memset(b0, 0, sizeof(ABT) * (ALCOVE - nor) * CAULDRON);
  }
  if (nonsparse) {
    *b = 1;
    return 1 + ALCOVE * CAULDRON * sizeof(ABT);
  } else {
    *b = 0;
    return 1;
  }
}

uint64_t DtoB_fp(const DSPACE *ds, const Dfmt *d, Bfmt *restrict b,
                 uint64_t nor, uint64_t stride) {
  const FIELD *f = ds->f;
  if (f->pbytesper == 1)
    return DtoB_fp_template<uint8_t, int16_t, CAULDRON_sp>(ds, d, b, nor,
                                                          stride);
  if (f->pbytesper == 2) {
    if (f->BfmtMagic == 10)
      return DtoB_fp_template<uint16_t, int16_t, CAULDRON_sp>(ds, d, b, nor,
                                                              stride);
    else
      return DtoB_fp_template<uint16_t, int16_t, CAULDRON_dp>(ds, d, b, nor,
                                                              stride);
  }
  return DtoB_fp_template<uint32_t, int32_t, CAULDRON_dp>(ds, d, b, nor,
                                                          stride);
}

/************************************************************************************
 *          CtoD
 ***********************************************************************************/

// Convert one row from C to D format.
// This is a separate function so that it can be called from the fastpath with
// noc a compile-time constant and inlined and unrolled by the compiler for that case.
template <typename DT, typename CT>
static inline void CtoD1(const CT *restrict c, DT *restrict d, uint64_t noc, int32_t p) {
    for (uint64_t k = 0; k < noc; k++) {
        int32_t x = c[k]; // needs to be a big enough type to hold -p..p
        x = (x < 0) ? (x + p) : x;
        d[k] = x;
    }
}
    

template <typename DT, typename CT>
void CtoD_fp_template(const DSPACE *ds, const Cfmt *c, Dfmt *restrict d,
                      uint64_t nor, uint64_t stride) {
  const FIELD *f = ds->f;
  const uint64_t CAULDRON = CAULDRON_bytes / sizeof(CT);
  uint64_t maincauldron = ds->noc / CAULDRON;
  uint64_t lastcauldron = ds->noc % CAULDRON;
  Dfmt *d0 = d;
  for (uint64_t i = 0; i < maincauldron; i++) {
    for (uint64_t j = 0; j < nor; j++) { //fastpath
        CtoD1<DT,  CT>((const CT *)c, (DT *)d, CAULDRON, (int32_t)f->charc);
        d += stride;
        c += CAULDRON_bytes;
    }
    d0 += f->dfmtcauld;
    d = d0;
  }
  if (lastcauldron) {
    for (uint64_t j = 0; j < nor; j++) {
        CtoD1<DT,  CT>((const CT *)c, (DT *)d, ds->noc % CAULDRON, (int32_t)f->charc);
        d += stride;
        c += CAULDRON_bytes;
    }
  }
}

void CtoD_fp(const DSPACE *ds, const Cfmt *c, Dfmt *restrict d, uint64_t nor,
             uint64_t stride) {
  const FIELD *f = ds->f;
  if (f->pbytesper == 1)
    CtoD_fp_template<uint8_t, float>(ds, c, d, nor, stride);
  else if (f->pbytesper == 2) {
    if (f->CfmtMagic == 11)
      CtoD_fp_template<uint16_t, double>(ds, c, d, nor, stride);
    else
      CtoD_fp_template<uint16_t, float>(ds, c, d, nor, stride);
  } else
    CtoD_fp_template<uint32_t,  double>(ds, c, d, nor, stride);
}

/************************************************************************************
 *          BwaMad
 ***********************************************************************************/

// AVX 512

static inline void __attribute__((target("avx512f")))
reduce_and_store_sp_512(__m512 c, __m512 rs, __m512 ss, __m512 mps,
                        __m512 *dest) {
  __m512 a = _mm512_fmadd_ps(c, rs, ss);
  __m512 b = _mm512_sub_ps(a, ss);
  *dest = _mm512_fmadd_ps(b, mps, c);
}

static inline void __attribute__((target("avx512f")))
reduce_and_store_dp_512(__m512d c, __m512d rs, __m512d ss, __m512d mps,
                        __m512d *dest) {
  __m512d a = _mm512_fmadd_pd(c, rs, ss);
  __m512d b = _mm512_sub_pd(a, ss);
  *dest = _mm512_fmadd_pd(b, mps, c);
}

template <typename CT, typename VT, VT BCAST(const CT), VT FMA(VT, VT, VT),
          void RAS(VT, VT, VT, VT, VT *)>
static inline void __attribute__((target("avx512f")))
inner_512(const CT *a, const uint8_t *b, Cfmt *restrict c, CT r, CT s, CT mp) {
  Cfmt *c1 = c;
  const uint8_t *b1 = b;
  const CT *a0 = a;
  const uint64_t CUP_bytes = 5 * sizeof(VT);
  for (uint64_t bcol = 0; bcol < CAULDRON_bytes / CUP_bytes; bcol++) {
    VT c00, c01, c02, c03, c04, c10, c11, c12, c13, c14, c20, c21, c22, c23,
        c24, c30, c31, c32, c33, c34, c40, c41, c42, c43, c44;
    c = c1;
    Cfmt *c0 = c;
    b = b1;
    const uint8_t *b0 = b;
    a = a0;
    c00 = *(VT *)c;
    c += sizeof(VT);
    c01 = *(VT *)c;
    c += sizeof(VT);
    c02 = *(VT *)c;
    c += sizeof(VT);
    c03 = *(VT *)c;
    c += sizeof(VT);
    c04 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c10 = *(VT *)c;
    c += sizeof(VT);
    c11 = *(VT *)c;
    c += sizeof(VT);
    c12 = *(VT *)c;
    c += sizeof(VT);
    c13 = *(VT *)c;
    c += sizeof(VT);
    c14 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c20 = *(VT *)c;
    c += sizeof(VT);
    c21 = *(VT *)c;
    c += sizeof(VT);
    c22 = *(VT *)c;
    c += sizeof(VT);
    c23 = *(VT *)c;
    c += sizeof(VT);
    c24 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c30 = *(VT *)c;
    c += sizeof(VT);
    c31 = *(VT *)c;
    c += sizeof(VT);
    c32 = *(VT *)c;
    c += sizeof(VT);
    c33 = *(VT *)c;
    c += sizeof(VT);
    c34 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c40 = *(VT *)c;
    c += sizeof(VT);
    c41 = *(VT *)c;
    c += sizeof(VT);
    c42 = *(VT *)c;
    c += sizeof(VT);
    c43 = *(VT *)c;
    c += sizeof(VT);
    c44 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;

    for (uint64_t acol = 0; acol < ALCOVE; acol++) {
      VT as0, as1, as2, as3, as4;
      VT bs;
      as0 = BCAST(a[acol]);
      as1 = BCAST(a[acol + ALCOVE]);
      as2 = BCAST(a[acol + 2 * ALCOVE]);
      as3 = BCAST(a[acol + 3 * ALCOVE]);
      as4 = BCAST(a[acol + 4 * ALCOVE]);
      bs = *(VT *)b;
      b += sizeof(VT);
      c00 = FMA(as0, bs, c00);
      c10 = FMA(as1, bs, c10);
      c20 = FMA(as2, bs, c20);
      c30 = FMA(as3, bs, c30);
      c40 = FMA(as4, bs, c40);
      bs = *(VT *)b;
      b += sizeof(VT);
      c01 = FMA(as0, bs, c01);
      c11 = FMA(as1, bs, c11);
      c21 = FMA(as2, bs, c21);
      c31 = FMA(as3, bs, c31);
      c41 = FMA(as4, bs, c41);
      bs = *(VT *)b;
      b += sizeof(VT);
      c02 = FMA(as0, bs, c02);
      c12 = FMA(as1, bs, c12);
      c22 = FMA(as2, bs, c22);
      c32 = FMA(as3, bs, c32);
      c42 = FMA(as4, bs, c42);
      bs = *(VT *)b;
      b += sizeof(VT);
      c03 = FMA(as0, bs, c03);
      c13 = FMA(as1, bs, c13);
      c23 = FMA(as2, bs, c23);
      c33 = FMA(as3, bs, c33);
      c43 = FMA(as4, bs, c43);
      bs = *(VT *)b;
      c04 = FMA(as0, bs, c04);
      c14 = FMA(as1, bs, c14);
      c24 = FMA(as2, bs, c24);
      c34 = FMA(as3, bs, c34);
      c44 = FMA(as4, bs, c44);
      b0 += CAULDRON_bytes;
      b = b0;
    }
    VT rs, ss, mps;
    rs = BCAST(r);
    ss = BCAST(s);
    mps = BCAST(mp);
    c = c1;
    c0 = c;
    RAS(c00, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c01, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c02, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c03, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c04, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c10, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c11, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c12, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c13, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c14, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c20, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c21, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c22, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c23, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c24, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c30, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c31, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c32, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c33, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c34, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c40, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c41, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c42, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c43, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c44, rs, ss, mps, (VT *)c);
    c1 += CUP_bytes;
    b1 += CUP_bytes;
  }
}

static inline void __attribute__((target("avx512f")))
inner_sp_512(const float *a, const uint8_t *b, Cfmt *restrict c, float r,
             float s, float mp) {
  inner_512<float, __m512, _mm512_set1_ps, _mm512_fmadd_ps,
            reduce_and_store_sp_512>(a, b, c, r, s, mp);
}

static inline void __attribute__((target("avx512f")))
inner_dp_512(const double *a, const uint8_t *b, Cfmt *restrict c, double r,
             double s, double mp) {
  inner_512<double, __m512d, _mm512_set1_pd, _mm512_fmadd_pd,
            reduce_and_store_dp_512>(a, b, c, r, s, mp);
}

// AVX versions (classes 'g' and 'k')

// for class 'g'
static inline __attribute__((target("avx"))) __m256
avx_fma_ps(__m256 x, __m256 y, __m256 z) {
  return _mm256_add_ps(z, _mm256_mul_ps(x, y));
}

static inline __attribute__((target("avx"))) __m256d
avx_fma_pd(__m256d x, __m256d y, __m256d z) {
  return _mm256_add_pd(z, _mm256_mul_pd(x, y));
}

template <typename VT, VT FMA(VT, VT, VT), VT SUB(VT, VT)>
static inline void __attribute__((target("fma")))
reduce_and_store_256(VT c, VT rs, VT ss, VT mps, VT *restrict dest) {
  VT a = FMA(c, rs, ss);
  VT b = SUB(a, ss);
  *dest = FMA(b, mps, c);
}

template <typename CT, typename VT, VT BCAST(const CT), VT FMA(VT, VT, VT),
          void RAS(VT, VT, VT, VT, VT *)>
static inline void __attribute__((target("fma")))
inner_256(const CT *a, const uint8_t *b, Cfmt *restrict c, CT r, CT s, CT mp) {
  Cfmt *c1 = c;
  const uint8_t *b1 = b;
  const CT *a0 = a;
  const uint64_t CUP_bytes = 4 * sizeof(VT);
  for (uint64_t bcol = 0; bcol < CAULDRON_bytes / CUP_bytes; bcol++) {
    VT c00, c01, c02, c03, c10, c11, c12, c13, c20, c21, c22, c23;
    Cfmt *c0 = c1;
    const uint8_t *b0 = b1;
    c = c1;
    b = b1;
    a = a0;
    c00 = *(VT *)c;
    c += sizeof(VT);
    c01 = *(VT *)c;
    c += sizeof(VT);
    c02 = *(VT *)c;
    c += sizeof(VT);
    c03 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c10 = *(VT *)c;
    c += sizeof(VT);
    c11 = *(VT *)c;
    c += sizeof(VT);
    c12 = *(VT *)c;
    c += sizeof(VT);
    c13 = *(VT *)c;
    c0 += CAULDRON_bytes;
    c = c0;
    c20 = *(VT *)c;
    c += sizeof(VT);
    c21 = *(VT *)c;
    c += sizeof(VT);
    c22 = *(VT *)c;
    c += sizeof(VT);
    c23 = *(VT *)c;

    for (uint64_t acol = 0; acol < ALCOVE; acol++) {
      VT as0, as1, as2;
      VT bs;
      as0 = BCAST(a[acol]);
      as1 = BCAST(a[acol + ALCOVE]);
      as2 = BCAST(a[acol + 2 * ALCOVE]);
      bs = *(VT *)b;
      b += sizeof(VT);
      c00 = FMA(as0, bs, c00);
      c10 = FMA(as1, bs, c10);
      c20 = FMA(as2, bs, c20);
      bs = *(VT *)b;
      b += sizeof(VT);
      c01 = FMA(as0, bs, c01);
      c11 = FMA(as1, bs, c11);
      c21 = FMA(as2, bs, c21);
      bs = *(VT *)b;
      b += sizeof(VT);
      c02 = FMA(as0, bs, c02);
      c12 = FMA(as1, bs, c12);
      c22 = FMA(as2, bs, c22);
      bs = *(VT *)b;
      c03 = FMA(as0, bs, c03);
      c13 = FMA(as1, bs, c13);
      c23 = FMA(as2, bs, c23);
      b0 += CAULDRON_bytes;
      b = b0;
    }
    c = c1;
    c0 = c;
    VT rs, ss, mps;
    rs = BCAST(r);
    ss = BCAST(s);
    mps = BCAST(mp);
    RAS(c00, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c01, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c02, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c03, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c10, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c11, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c12, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c13, rs, ss, mps, (VT *)c);
    c0 += CAULDRON_bytes;
    c = c0;
    RAS(c20, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c21, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c22, rs, ss, mps, (VT *)c);
    c += sizeof(VT);
    RAS(c23, rs, ss, mps, (VT *)c);
    b1 += CUP_bytes;
    c1 += CUP_bytes;
  }
}

static void __attribute__((target("fma")))
inner_sp_256(const float *a, const uint8_t *b, Cfmt *restrict c, float r,
             float s, float mp) {
  inner_256<float, __m256, _mm256_set1_ps, _mm256_fmadd_ps,
            reduce_and_store_256<__m256, _mm256_fmadd_ps, _mm256_sub_ps>>(
      a, b, c, r, s, mp);
}

static void __attribute__((target("fma")))
inner_dp_256(const double *a, const uint8_t *b, Cfmt *restrict c, double r,
             double s, double mp) {
  inner_256<double, __m256d, _mm256_set1_pd, _mm256_fmadd_pd,
            reduce_and_store_256<__m256d, _mm256_fmadd_pd, _mm256_sub_pd>>(
      a, b, c, r, s, mp);
}

static void __attribute__((target("avx")))
inner_sp_256_nofma(const float *a, const uint8_t *b, Cfmt *restrict c, float r,
                   float s, float mp) {
  inner_256<float, __m256, _mm256_set1_ps, avx_fma_ps,
            reduce_and_store_256<__m256, avx_fma_ps, _mm256_sub_ps>>(a, b, c, r,
                                                                     s, mp);
}

static void __attribute__((target("avx")))
inner_dp_256_nofma(const double *a, const uint8_t *b, Cfmt *restrict c,
                   double r, double s, double mp) {
  inner_256<double, __m256d, _mm256_set1_pd, avx_fma_pd,
            reduce_and_store_256<__m256d, avx_fma_pd, _mm256_sub_pd>>(a, b, c,
                                                                      r, s, mp);
}

//Now the generic template and dispatcher

template <typename ABT, typename CT, uint64_t BOXLET,
          void INNER(const CT *, const uint8_t *, Cfmt *restrict, CT, CT, CT)>
void BwaMad_fp_template(const Afmt *a, uint8_t *restrict bwa, Cfmt *restrict c,
                        uint64_t p) {
  const CT S = (sizeof(CT) == 8) ? (CT)(3L << 51) : (CT)(3 << 22);
  CT r = 1.0 / p;
  CT mp = -1.0 * (int64_t)p;
  CT *awa = (CT *)(bwa + ALCOVE * CAULDRON_bytes);
  c += BOXLET * CAULDRON_bytes * (*a); // initial skip
  while (*a != 255) {
    a++;
    // ASeed
    ABT *as = (ABT *)a;
    for (uint64_t i = 0; i < ALCOVE * BOXLET; i++)
      awa[i] = as[i];
    INNER(awa, bwa, c, r, S, mp);
    a += sizeof(ABT) * BOXLET * ALCOVE;
    c += BOXLET * CAULDRON_bytes * (*a + 1);
  }
}

void BwaMad_fp(const FIELD *f, const Afmt *a, uint8_t *restrict bwa,
               Cfmt *restrict c) {
  const char mactype = f->mact[0];
  // const char mactype = 'g'; // for testing
  if (f->pbytesper <= 2)
    if (f->BwaMagic == 10)
      if (mactype >= 'm')
        BwaMad_fp_template<int16_t, float, 5, inner_sp_512>(a, bwa, c,
                                                            f->charc);
      else if (mactype >= 'k')
        BwaMad_fp_template<int16_t, float, 3, inner_sp_256>(a, bwa, c,
                                                            f->charc);
      else
        BwaMad_fp_template<int16_t, float, 3, inner_sp_256_nofma>(a, bwa, c,
                                                                  f->charc);
    else if (mactype >= 'm')
      BwaMad_fp_template<int16_t, double, 5, inner_dp_512>(a, bwa, c, f->charc);
    else if (mactype >= 'k')
      BwaMad_fp_template<int16_t, double, 3, inner_dp_256>(a, bwa, c, f->charc);
    else
      BwaMad_fp_template<int16_t, double, 3, inner_dp_256_nofma>(a, bwa, c,
                                                                 f->charc);

  else if (mactype >= 'm')
    BwaMad_fp_template<int32_t, double, 5, inner_dp_512>(a, bwa, c, f->charc);
  else if (mactype >= 'k')
    BwaMad_fp_template<int32_t, double, 3, inner_dp_256>(a, bwa, c, f->charc);
  else
    BwaMad_fp_template<int32_t, double, 3, inner_dp_256_nofma>(a, bwa, c,
                                                               f->charc);
}
