/*
         mfuns.h  -   matrix functions prototypes
         =======      J.G.Thackray 13.10.2017
*/

/* This file contains utility maths functions until better places can be found for them */

/*
 * Flatten into a row something specified as some zeroes,
 * then some -1s and then a remnant.
 * Used by theings like invariant subspace and subspace action
 */

#ifndef MFUNS_H
#define MFUNS_H

typedef union header {
  struct named_header {
    uint64_t rnd1;
    uint64_t fdef;
    uint64_t nor;
    uint64_t noc;
    uint64_t rnd2;
  } named;
  uint64_t hdr[5];
} header;

extern void make_plain(const char *zero_bs, const char *nref_bs, const char *in, const char *out, uint64_t fdef);

#endif /* MFUNS_H */
