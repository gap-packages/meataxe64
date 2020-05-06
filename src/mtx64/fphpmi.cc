extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <x86intrin.h>
#include "field.h"
#include "hpmi.h"
#include "fphpmi.h"
}



#define ALCOVE 24
#define CAULDRON_bytes 640
#define CAULDRON_sp (CAULDRON_bytes/sizeof(float))
#define CAULDRON_dp (CAULDRON_bytes/sizeof(double))

// Access macros for the A format
#define aix_skip(j) (a[ix[(j)]])
// access the k,l entry in the current boxlet x alcove piece of alcove j
#define aix_entry(j,k,l) (*(ABT *)(&(a[ix[(j)] + 1 + sizeof(ABT)*(ALCOVE*(l) + (k))])))
// Step down to next block of A format.
#define aix_step (1 + sizeof(ABT)*BOXLET*ALCOVE) 

// Inner part of DtoA, converts a boxlet x alcove piece of alcove j
// width and height indicate how much "real" matrix there is, the rest will be padding
// This only exists as a separate routine so that calls with compile-time
// constant width and/or height can be inlined and benefit from constant propagation


template <typename DT, typename ABT, int BOXLET>
void DtoA1 (Afmt *restrict a, uint64_t *restrict ix,
                   uint64_t j, uint64_t width, uint64_t height,
                   const Dfmt *restrict d, uint64_t stride, uint64_t p, uint64_t p2) {
    DT nonsparse = 0;
    for (uint64_t l = 0; l < height; l++)  {
        DT *dp = (DT *)d;
        for (uint64_t k = 0; k < width; k++) {
            DT x = *dp++;
            nonsparse |= x;
            aix_entry(j,k,l) = (ABT)(x > p2 ? (ABT)x- p : x);
        }
        for (uint64_t k=width; k < ALCOVE; k++) {
            aix_entry(j,k,l) = 0;                
        }
        d += stride;
    }
    for (uint64_t l = height; l < BOXLET; l++) 
        for (uint64_t k = 0; k < ALCOVE; k++) 
            aix_entry(j,k,l) = 0;
    if (nonsparse || (aix_skip(j) == 250)) {
        ix[j] += aix_step;
        aix_skip(j) = 0;                
    } else 
        aix_skip(j)++;
}


// This is pretty much DtoA.


template <typename DT, typename ABT, int BOXLET>
void DtoA_fp_template(const DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt *restrict a,
                           uint64_t nora, uint64_t stride) {
    const FIELD *f = ds->f;
    
    uint64_t mainboxlets = nora/BOXLET;
    uint64_t lastboxlet = (nora % BOXLET) > 0;
    uint64_t mainalcoves = ds->noc/ALCOVE;
    uint64_t lastalcove = (ds->noc % ALCOVE) > 0;
    uint64_t p2 = f->charc/2;
    uint64_t p = f->charc;

    // reserve a byte in each alcove for reduction flag
    for (uint64_t j = 0; j < mainalcoves + lastalcove; j++) {
        ix[j] ++;
        aix_skip(j) = 0;
    }

    for (uint64_t i = 0; i < mainboxlets; i++) {
        const Dfmt *d0 = d;
        for (uint64_t j = 0; j < mainalcoves; j++) {
            DtoA1<DT,ABT,BOXLET>(a, ix, j, ALCOVE, BOXLET, d, stride, p, p2); //fastpath
            d += sizeof(DT)*ALCOVE;
        }
        if (lastalcove) {
             DtoA1<DT,ABT,BOXLET>(a,ix,mainalcoves, ds->noc % ALCOVE, BOXLET, d, stride, p, p2);
        }
        d0 += BOXLET*stride;
        d = d0;
    }
    if (lastboxlet) {
        for (uint64_t j = 0; j < mainalcoves; j++) {
            DtoA1<DT,ABT,BOXLET>(a,ix,j, ALCOVE, nora % BOXLET, d, stride, p, p2);
            d += sizeof(DT)*ALCOVE;
        }
        if (lastalcove) {
            DtoA1<DT,ABT,BOXLET>(a,ix,mainalcoves, ds->noc % ALCOVE, nora % BOXLET, d, stride, p, p2);                       
        }        
    }                
}


void DtoA_fp(const DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt *restrict a,
             uint64_t nora, uint64_t stride) {
    const FIELD *f = ds->f;
    switch(f->pbytesper) {
    case 1:
        if (f->mact[0] >= 'm')
            DtoA_fp_template<uint8_t, int8_t,  5>(ds, ix, d, a, nora, stride);
        else
            DtoA_fp_template<uint8_t, int8_t, 3>(ds, ix, d, a, nora, stride);
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

template <typename DT, typename ABT, int CAULDRON>
uint64_t DtoB_fp_template(const DSPACE * ds, const Dfmt * d, Bfmt *restrict b, 
              uint64_t nor, uint64_t stride) {
    const FIELD *f = ds->f;
    DT p = f->charc;
    DT p2 = p/2;
    DT nonsparse = 0;
    const Dfmt * d0 = d;
    ABT *b0 = (ABT *)(b+1);
    if (ds->noc == CAULDRON && nor == ALCOVE) {
        // fast-path compile time known loop lengths
        for (uint64_t i = 0; i < ALCOVE; i++) {
            for(uint64_t j = 0; j < CAULDRON; j++) {
                DT x = *(DT *)d;
                nonsparse |= x;
                ABT sx = (ABT)x;
                sx = x > p2 ? sx-p : sx;
                *b0++ = sx; 
                d += sizeof(DT);
            }
            d0 += stride;
            d = d0;
        }
    } else {
        // slow path for bricks on the right or bottom of the matrix
        for (uint64_t i = 0; i < nor; i++) {
            for (uint64_t j = 0; j < ds->noc; j++) {
                DT x = *(DT *)d;
                nonsparse |= x;
                ABT sx = (ABT)x;
                sx = x > p2 ? sx-p : sx;
                *b0++ =sx; 
                d += sizeof(DT);
            }
            memset(b0, 0, sizeof(ABT)*(CAULDRON-ds->noc));
            b0 += CAULDRON - ds->noc;
            d0 += stride;
            d = d0;
        }
        memset(b0,  0, sizeof(ABT)*(ALCOVE - nor)* CAULDRON);
    }
    if (nonsparse) {
        *b = 1;
        return 1 + ALCOVE*CAULDRON*sizeof(ABT);
    } else {
        *b = 0;
        return 1;
    }
}


    
uint64_t DtoB_fp(const DSPACE * ds, const Dfmt * d, Bfmt *restrict b, 
                    uint64_t nor, uint64_t stride) {
    const FIELD *f = ds->f;
    if (f->pbytesper == 1) 
        return DtoB_fp_template<uint8_t, int8_t, CAULDRON_sp>(ds, d, b, nor, stride);
    if (f->pbytesper == 2) {
        if (f->BfmtMagic == 10)
            return DtoB_fp_template<uint16_t, int16_t, CAULDRON_sp>(ds, d, b, nor, stride);
        else
            return DtoB_fp_template<uint16_t, int16_t, CAULDRON_dp>(ds, d, b, nor, stride);
    }
    return DtoB_fp_template<uint32_t, int32_t, CAULDRON_dp>(ds, d, b, nor, stride);
}

            
        

template<typename DT, typename sDT, typename CT>
void CtoD_fp_template(const DSPACE * ds, const Cfmt * c, Dfmt *restrict d, uint64_t nor, uint64_t stride)
{
    const CT S = (sizeof(CT) == 8) ? (CT)(3L << 51) : (CT)(3 << 22);
    const FIELD *f = ds->f;
    const uint64_t cauldron = CAULDRON_bytes/sizeof(CT);
    uint64_t maincauldron = ds->noc/cauldron;
    uint64_t lastcauldron = ds->noc % cauldron;
    Dfmt *d0 = d;
    CT p = f->charc;
    CT pi = 1/p;
    for (uint64_t i = 0; i < maincauldron; i++) {
        Dfmt *d1 = d0;
        d = d1;
        for (uint64_t j = 0; j < nor; j++) {
            for (uint64_t k = 0; k < cauldron; k++) {
                CT x = *(CT *)c;
                //                CT x0 = x;
                x -= ((x*pi + S)-S)*p;
                sDT y = x;
                y = (y < 0) ? y+f->charc : y;
                //           if (y > f->charc)
                //  fprintf(stderr, "Panic %i %i %i %lf %lf %i\n",i,j,k,(double)x0,(double)x,y);
                *(DT *)d = y;
                d += sizeof(DT);
                c += sizeof(CT);
            }
            d1 += stride;
            d = d1;
        }
        d0 += f->dfmtcauld;
    }
    if (lastcauldron) {
        Dfmt *d1 = d0;
        d = d1;
        for (uint64_t j = 0; j < nor; j++) {
            for (uint64_t k = 0; k < ds->noc % cauldron; k++) {
                CT x = *(CT *)c;
                x -= ((x*pi + S)-S)*p;
                sDT y = x;
                y = (y < 0) ? y+f->charc : y;
                *(DT *)d = y;
                d += sizeof(DT);
                c += sizeof(CT);
            }
            c += sizeof(CT)*(cauldron - ds->noc % cauldron);
            d1 += stride;
            d = d1;
        }
        d0 += f->dfmtcauld;
        
    }
}


void CtoD_fp(const DSPACE * ds, const Cfmt * c, Dfmt *restrict d, uint64_t nor, uint64_t stride) {
    const FIELD *f = ds->f;
    if (f->pbytesper == 1)
        CtoD_fp_template<uint8_t, int8_t, float>(ds,c,d,nor,stride);
    else if (f->pbytesper == 2) {
        if (f->CfmtMagic == 11)
            CtoD_fp_template<uint16_t, int16_t, double>(ds,c,d,nor,stride);
        else
            CtoD_fp_template<uint16_t, int16_t, float>(ds,c,d,nor,stride);
    } else 
        CtoD_fp_template<uint32_t, int32_t, double>(ds,c,d,nor,stride);
}    
            
            
        
    




static inline void __attribute__((target("avx512f")))
reduce_and_store_sp_512 (__m512 c, __m512 rs, __m512 ss, __m512 mps, __m512 *dest) {
        __m512 a = _mm512_fmadd_ps (c,  rs, ss);
        __m512 b = _mm512_sub_ps (a,  ss);
        *dest = _mm512_fmadd_ps (b,  mps, c);
}

static inline void __attribute__((target("avx512f")))
reduce_and_store_dp_512 (__m512d c, __m512d rs, __m512d ss, __m512d mps, __m512d *dest) {
        __m512d a = _mm512_fmadd_pd (c,  rs, ss);
        __m512d b = _mm512_sub_pd (a,  ss);
        *dest = _mm512_fmadd_pd (b,  mps, c);
}


#define CUP_bytes_512 320

template<typename CT, typename VT, VT BCAST(const CT), VT FMA(VT, VT, VT), void RAS(VT, VT, VT, VT, VT *)> 
static inline void __attribute__((target("avx512f"))) inner_512( const CT *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          CT r, CT s, CT mp) {
    Cfmt *c1 = c;
    const uint8_t *b1 = b;
    const CT *a0 = a;
    for (uint64_t bcol = 0; bcol < CAULDRON_bytes/CUP_bytes_512; bcol++) {
        VT c00, c01, c02, c03, c04,
            c10, c11, c12, c13, c14,
            c20, c21, c22, c23, c24,
            c30, c31, c32, c33, c34,
            c40, c41, c42, c43, c44;
        c = c1;
        Cfmt *c0 = c;
        b = b1;
        const uint8_t *b0 = b;
        a = a0;
        c00 = *(VT *)c; c+= sizeof(VT);
        c01 = *(VT *)c; c+= sizeof(VT);
        c02 = *(VT *)c; c+= sizeof(VT);
        c03 = *(VT *)c; c+= sizeof(VT);
        c04 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c10 = *(VT *)c; c+= sizeof(VT);
        c11 = *(VT *)c; c+= sizeof(VT);
        c12 = *(VT *)c; c+= sizeof(VT);
        c13 = *(VT *)c; c+= sizeof(VT);
        c14 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c20 = *(VT *)c; c+= sizeof(VT);
        c21 = *(VT *)c; c+= sizeof(VT);
        c22 = *(VT *)c; c+= sizeof(VT);
        c23 = *(VT *)c; c+= sizeof(VT);
        c24 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c30 = *(VT *)c; c+= sizeof(VT);
        c31 = *(VT *)c; c+= sizeof(VT);
        c32 = *(VT *)c; c+= sizeof(VT);
        c33 = *(VT *)c; c+= sizeof(VT);
        c34 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c40 = *(VT *)c; c+= sizeof(VT);
        c41 = *(VT *)c; c+= sizeof(VT);
        c42 = *(VT *)c; c+= sizeof(VT);
        c43 = *(VT *)c; c+= sizeof(VT);
        c44 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        
        for (uint64_t acol = 0; acol < ALCOVE; acol ++) {
            VT as0, as1, as2, as3, as4;
            VT bs;
            as0 = BCAST(a[acol]); 
            as1 = BCAST(a[acol+ALCOVE]);
            as2 = BCAST(a[acol+2*ALCOVE]);
            as3 = BCAST(a[acol+3*ALCOVE]);
            as4 = BCAST(a[acol+4*ALCOVE]);
            bs = *(VT *)b; b += sizeof(VT);
            c00 = FMA(as0, bs, c00);
            c10 = FMA(as1, bs, c10);
            c20 = FMA(as2, bs, c20);
            c30 = FMA(as3, bs, c30);
            c40 = FMA(as4, bs, c40);
            bs = *(VT *)b; b += sizeof(VT);
            c01 = FMA(as0, bs, c01);
            c11 = FMA(as1, bs, c11);
            c21 = FMA(as2, bs, c21);
            c31 = FMA(as3, bs, c31);
            c41 = FMA(as4, bs, c41);
            bs = *(VT *)b; b += sizeof(VT);
            c02 = FMA(as0, bs, c02);
            c12 = FMA(as1, bs, c12);
            c22 = FMA(as2, bs, c22);
            c32 = FMA(as3, bs, c32);
            c42 = FMA(as4, bs, c42);
            bs = *(VT *)b; b += sizeof(VT);
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
         if (!reduce) {
             c = c1;
             c0 = c;
             *(VT *)c = c00; c+= sizeof(VT);
             *(VT *)c = c01; c+= sizeof(VT);
             *(VT *)c = c02; c+= sizeof(VT);
             *(VT *)c = c03; c+= sizeof(VT);
             *(VT *)c = c04; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c10; c+= sizeof(VT);
             *(VT *)c = c11; c+= sizeof(VT);
             *(VT *)c = c12; c+= sizeof(VT);
             *(VT *)c = c13; c+= sizeof(VT);
             *(VT *)c = c14; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c20; c+= sizeof(VT);
             *(VT *)c = c21; c+= sizeof(VT);
             *(VT *)c = c22; c+= sizeof(VT);
             *(VT *)c = c23; c+= sizeof(VT);
             *(VT *)c = c24; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c30; c+= sizeof(VT);
             *(VT *)c = c31; c+= sizeof(VT);
             *(VT *)c = c32; c+= sizeof(VT);
             *(VT *)c = c33; c+= sizeof(VT);
             *(VT *)c = c34; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c40; c+= sizeof(VT);
             *(VT *)c = c41; c+= sizeof(VT);
             *(VT *)c = c42; c+= sizeof(VT);
             *(VT *)c = c43; c+= sizeof(VT);
             *(VT *)c = c44; 
         } else {
             VT rs, ss, mps;
             rs = BCAST(r);
             ss = BCAST(s);
             mps = BCAST(mp);
             c = c1;
             c0 = c;
             RAS(c00, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c01, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c02, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c03, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c04, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c10, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c11, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c12, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c13, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c14, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c20, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c21, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c22, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c23, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c24, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c30, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c31, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c32, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c33, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c34, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c40, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c41, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c42, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c43, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c44, rs, ss, mps, (VT *)c); 
         }
         c1 += CUP_bytes_512;
         b1 += CUP_bytes_512;
    }
}



static inline void __attribute__((target("avx512f"))) inner_sp_512( const float *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          float r, float s, float mp) {
    inner_512<float, __m512, _mm512_set1_ps, _mm512_fmadd_ps, reduce_and_store_sp_512>(a,b,c,reduce,r,s,mp);
}

static inline void __attribute__((target("avx512f"))) inner_dp_512( const double *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          double r, double s, double mp) {
    inner_512<double, __m512d, _mm512_set1_pd, _mm512_fmadd_pd, reduce_and_store_dp_512>(a,b,c,reduce,r,s,mp);
}




#define CUP_bytes_256 128


static inline void  __attribute__((target("fma"))) reduce_and_store_sp_256 (__m256 c, __m256 rs, __m256 ss, __m256 mps, __m256 *restrict dest) {
        __m256 a = _mm256_fmadd_ps (c,  rs, ss);
        __m256 b = _mm256_sub_ps (a,  ss);
        *dest = _mm256_fmadd_ps (b,  mps, c);
}

static inline void  __attribute__((target("fma"))) reduce_and_store_dp_256 (__m256d c, __m256d rs, __m256d ss, __m256d mps, __m256d *restrict dest) {
        __m256d a = _mm256_fmadd_pd (c,  rs, ss);
        __m256d b = _mm256_sub_pd (a,  ss);
        *dest = _mm256_fmadd_pd(b,  mps, c);
}

template<typename CT, typename VT, VT BCAST(const CT), VT FMA(VT, VT, VT), void RAS(VT, VT, VT, VT, VT *)> 
static inline void  __attribute__((target("fma"))) inner_256( const  CT *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          CT r, CT s, CT mp) {
    Cfmt *c1 = c;
    const uint8_t *b1 = b;
    const CT *a0 = a;
    for (uint64_t bcol = 0; bcol < CAULDRON_bytes/CUP_bytes_256; bcol++) {
        VT c00, c01, c02, c03, c10, c11, c12, c13, c20, c21, c22, c23;
        Cfmt *c0 = c1;
        const uint8_t *b0 = b1;
        c = c1;
        b = b1;
        a = a0;
        c00 = *(VT *)c; c+= sizeof(VT);
        c01 = *(VT *)c; c+= sizeof(VT);
        c02 = *(VT *)c; c+= sizeof(VT);
        c03 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c10 = *(VT *)c; c+= sizeof(VT);
        c11 = *(VT *)c; c+= sizeof(VT);
        c12 = *(VT *)c; c+= sizeof(VT);
        c13 = *(VT *)c; c0 += CAULDRON_bytes; c = c0;
        c20 = *(VT *)c; c+= sizeof(VT);
        c21 = *(VT *)c; c+= sizeof(VT);
        c22 = *(VT *)c; c+= sizeof(VT);
        c23 = *(VT *)c;
        
        for (uint64_t acol = 0; acol < ALCOVE; acol ++) {
            VT as0, as1, as2;
            VT bs;
            as0 = BCAST(a[acol]); 
            as1 = BCAST(a[acol+ALCOVE]);
            as2 = BCAST(a[acol+2*ALCOVE]);
            bs = *(VT *)b; b += sizeof(VT);
            c00 = FMA(as0, bs, c00);
            c10 = FMA(as1, bs, c10);
            c20 = FMA(as2, bs, c20);
            bs = *(VT *)b; b += sizeof(VT);
            c01 = FMA(as0, bs, c01);
            c11 = FMA(as1, bs, c11);
            c21 = FMA(as2, bs, c21);
            bs = *(VT *)b; b += sizeof(VT);
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
         if (!reduce) {
             *(VT *)c = c00; c+= sizeof(VT);
             *(VT *)c = c01; c+= sizeof(VT);
             *(VT *)c = c02; c+= sizeof(VT);
             *(VT *)c = c03; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c10; c+= sizeof(VT);
             *(VT *)c = c11; c+= sizeof(VT);
             *(VT *)c = c12; c+= sizeof(VT);
             *(VT *)c = c13; c0 += CAULDRON_bytes; c = c0;
             *(VT *)c = c20; c+= sizeof(VT);
             *(VT *)c = c21; c+= sizeof(VT);
             *(VT *)c = c22; c+= sizeof(VT);
             *(VT *)c = c23; 
         } else {
             VT rs, ss, mps;
             rs = BCAST(r);
             ss = BCAST(s);
             mps = BCAST(mp);
             RAS(c00, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c01, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c02, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c03, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c10, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c11, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c12, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c13, rs, ss, mps, (VT *)c); c0 += CAULDRON_bytes; c = c0;
             RAS(c20, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c21, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c22, rs, ss, mps, (VT *)c); c += sizeof(VT);
             RAS(c23, rs, ss, mps, (VT *)c); 
         }
         b1 += CUP_bytes_256;
         c1 += CUP_bytes_256;
    }
}




static inline void __attribute__((target("fma"))) inner_sp_256( const float *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          float r, float s, float mp) {
    inner_256<float, __m256, _mm256_set1_ps, _mm256_fmadd_ps, reduce_and_store_sp_256>(a,b,c,reduce,r,s,mp);
}

static inline void __attribute__((target("fma"))) inner_dp_256( const double *a, const uint8_t *b,  Cfmt *restrict c, int reduce,
                          double r, double s, double mp) {
    inner_256<double, __m256d, _mm256_set1_pd, _mm256_fmadd_pd, reduce_and_store_dp_256>(a,b,c,reduce,r,s,mp);
}


        

template<typename sDT, typename CT, uint64_t BOXLET>
void  BwaMad_fp_template(const Afmt *a, const uint8_t *b, Cfmt  *restrict c, uint64_t p, char machine) {
    const CT S = (sizeof(CT) == 8) ? (CT)(3L<<51) : (CT)(3 << 22);
    CT r = 1.0/p;
    CT mp = -1.0*(int64_t)p;
    CT ad[ALCOVE][BOXLET];
    float *adf = (float *)&(ad[0][0]);
    double *add= (double *)&(ad[0][0]);
    const uint64_t reduce = *a++; // get the reduction flag
    c += BOXLET*CAULDRON_bytes * (*a); // initial skip
    if (reduce) {
        while (*a != 255) {
            a++;
            for (uint64_t i = 0; i < ALCOVE; i++)
                for (uint64_t j = 0; j < BOXLET; j++) {
                    ad[i][j] = *(sDT *)a;
                    a += sizeof(sDT);
                }
            if (sizeof(CT) == 8) // doubles
                if (machine >= 'm')
                    inner_dp_512(add,b,c,1,r,S,mp);
                else
                    inner_dp_256(add,b,c,1,r,S,mp);
            else
                if (machine >= 'm')
                    inner_sp_512(adf,b,c,1,r,S,mp);
                else
                    inner_sp_256(adf,b,c,1,r,S,mp);
            c += BOXLET*CAULDRON_bytes* (*a +1);
        }
    } else  {
        while (*a != 255) {
            a++;
            for (uint64_t i = 0; i < ALCOVE; i++)
                for (uint64_t j = 0; j < BOXLET; j++) {
                    ad[i][j] = *(sDT *)a;
                    a += sizeof(sDT);
                }
            if (sizeof(CT) == 8) // doubles
                if (machine >= 'm')
                    inner_dp_512(add,b,c,0,r,S,mp);
                else
                    inner_dp_256(add,b,c,0,r,S,mp);
            else
                if (machine >= 'm')
                    inner_sp_512(adf,b,c,0,r,S,mp);
                else
                    inner_sp_256(adf,b,c,0,r,S,mp);
            c += BOXLET*CAULDRON_bytes* (*a +1);
        } 
    } 
}

void BwaMad_fp(const FIELD *f, const Afmt *a, const uint8_t *b, Cfmt  *restrict c) {
    if (f->pbytesper == 1) 
        if (f->mact[0] >= 'm')
            BwaMad_fp_template<int8_t, float,5>(a,b,c,f->charc,f->mact[0]);
        else
            BwaMad_fp_template<int8_t, float,3>(a,b,c,f->charc,f->mact[0]);
    else if (f->pbytesper == 2)
        if (f->BwaMagic == 10)
            if (f->mact[0] >= 'm')
                BwaMad_fp_template<int16_t, float,5>(a,b,c,f->charc,f->mact[0]);
            else
                BwaMad_fp_template<int16_t, float,3>(a,b,c,f->charc,f->mact[0]);
        else
            if (f->mact[0] >= 'm')
                BwaMad_fp_template<int16_t, double,5>(a,b,c,f->charc,f->mact[0]);
            else
                BwaMad_fp_template<int16_t, double,3>(a,b,c,f->charc,f->mact[0]);
    else
        if (f->mact[0] >= 'm')
            BwaMad_fp_template<int32_t, double,5>(a,b,c,f->charc,f->mact[0]);
        else
            BwaMad_fp_template<int32_t, double,3>(a,b,c,f->charc,f->mact[0]);
}


