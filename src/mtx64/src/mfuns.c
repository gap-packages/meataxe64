/*
         mfuns.c  -   matrix functions implementations
         =======      J.G.Thackray 13.10.2017
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"
#include "mfuns.h"
#include "io.h"
#include "bitstring.h"
#include "slab.h"

void make_plain(const char *zero_bs, const char *nref_bs, const char *in, const char *out, uint64_t fdef)
{
  /* A very naive implementation */
  EFIL *ezbs, *embs, *ei, *eo; /* zero bitstring, minus -1 bitstring, in, out */
  FELT min1;
  header hdrzbs, hdrmbs, hdrio;
  /*uint64_t hdrzbs[5], hdrmbs[5], hdrio[5];*/
  uint64_t nor, noci1, noci2, noci3, noco, sizz, sizm, i, j, k, l;
  FIELD *f;
  FELT felt;
  DSPACE dsi, dso; /* Matrix in */
  Dfmt *mo, *mi; /* Matrix out */
  uint64_t *bstz = NULL, *bstm;
  int use_ei;

#if 0
  /* Print the various parts */
  if (NULL != zero_bs) {
    printf("make_plain: parameter zero_bs %s\n", zero_bs);
    zut_fn(zero_bs);
  }
  printf("make_plain: parameter nref_bs %s\n", nref_bs);
  zut_fn(nref_bs);
#endif
  ezbs = (NULL != zero_bs) ? ERHdr(zero_bs, hdrzbs.hdr) : NULL;
  embs = ERHdr(nref_bs, hdrmbs.hdr);
  noci3 = hdrmbs.named.nor; /* Total bits in nref bitstring */
  noci1 = hdrmbs.named.noc; /* Set bits in nref bitstring */
  use_ei = noci1 != noci3;
  if (use_ei) {
    ei = ERHdr(in, hdrio.hdr);    // remnant   = 1 fdef nor noc 0
    nor = hdrio.named.nor; /* Rows of input */
  } else {
    ei = NULL;
    nor = noci1;
    hdrio.named.noc = 0;
  }
  if (NULL != ezbs) {
    /* Some leading zeros */
    noci2 = hdrzbs.named.noc; /* Set bits in zero bitstring */
    noco = hdrzbs.named.nor; /* Total bits in zero bitstring, ie amount to output */
  } else {
    /* No leading zeros */
    noci2 = 0;
    noco = hdrmbs.named.nor;
  }
  /* Compatibility check */
  if (noci1 != nor /* Number of pivots == number of rows */ || 
      hdrio.named.noc + noci1 + noci2 != noco /* Total columns check */) {
    LogString(80,"Inputs incompatible");
    exit(23);
  }
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  min1 = FieldNeg(f, 1);
  hdrio.named.noc = noco;
  /* Create the output header */
  eo = EWHdr(out, hdrio.hdr);  
  DSSet(f, noci3 - nor, &dsi); /* input space */
  DSSet(f, noco, &dso); /* output space */
  /* Allocate space for a row of input, and a row of output */
  mi = malloc(dsi.nob);
  mo = malloc(dso.nob);
  /* Read the bitstring(s) */
  if (NULL != ezbs) {
    sizz = 8 * (2 + (noco + 63) / 64);
    bstz = malloc(sizz);
    ERData(ezbs, sizz, (uint8_t *)bstz);
  }
  sizm = 8 * (2 + (hdrmbs.named.nor + 63) / 64);
  bstm = malloc(sizm);
  ERData(embs, sizm, (uint8_t *)bstm);
  /* Now read through in row by row, inserting minus -1s and zeros */
  for (i = 0; i < nor; i++) {
    ERData(ei, dsi.nob, mi);
    /* Clear output row */
    memset(mo, 0, dso.nob);
    /* TBD: put in -1s and contents of in. DCut and DPaste */
    k = 0; /* Bit position in riffle of -1s and input */
    l = 0; /* Bit position in -1s */
    for (j = 0; j < noco; j++) {
      if (NULL != ezbs && BSBitRead(bstz, j)) {
        /* No action, we're putting in a zero */
      } else {
        /* Putting in a -1 or something from the input */
        if (BSBitRead(bstm, k)) {
          /* Put in a -1 if we're at the correct row */
          if (l == i) {
            /* Correct row */
            DPak(&dso, j, mo, min1);
          }
          /* Else leave as zero */
          l++; /* Next column for -1s */
        } else {
          /* Pick the value out of input */
          felt = DUnpak(&dsi, k - l, mi);
          DPak(&dso, j, mo, felt);
        }
        k++; /* Next column in overall input */
      }
    }
    EWData(eo, dso.nob, mo);
  }
  /* Close files */
  if (use_ei) {
    ERClose1(ei, 1);
  }
  EWClose1(eo, 1);
  ERClose1(embs, 1);
  if (NULL != ezbs) {
    ERClose1(ezbs, 1);
  }
  /* Deallocate crap */
  free(mi);
  free(mo);
  free(f);
  free(bstm);
  if (NULL != ezbs) {
    free(bstz);
  }
}

int ident(uint64_t fdef, uint64_t nor, uint64_t noc, uint64_t elt,
          const char *out)
{
  uint64_t hdr[5];
  EFIL *e;
  FIELD *f;
  DSPACE ds;
  Dfmt *v1;
  uint64_t i;

  hdr[0] = 1;
  hdr[1] = fdef;
  hdr[2] = nor;
  hdr[3] = noc;
  hdr[4] = 0;
  e = EWHdr(out, hdr);
  f = malloc(FIELDLEN);
  if (f == NULL) {
    LogString(81, "Can't malloc field structure");
    exit(8);
  }
  FieldSet(fdef, f);
  DSSet(f, noc, &ds);
  v1 = malloc(ds.nob);
  if (v1 == NULL) {
    LogString(81,"Can't malloc a single vector");
    exit(9);
  }
  /******  for each row of the matrix  */
  for (i = 0; i < nor; i++) {
    memset(v1, 0, ds.nob);
    DPak(&ds, i, v1, elt);
    EWData(e, ds.nob, v1);
  }
  EWClose(e);
  free(v1);
  free(f);
  return 1;
}
