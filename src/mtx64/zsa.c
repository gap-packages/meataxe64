/*
      zsa.c     meataxe-64 subspace action
      =====     J. G. Thackray   12.11.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "mfuns.h"
#include "io.h"
#include "slab.h"
#include "funs.h"
#include "bitstring.h"
#include "util.h"

#define jgt 1

static const char prog_name[] = "zsa";

static char *fun_tmp;

#define FUN_TMP "_funs"

/* Negate: effectively a copy from zng.c */

static void fNegate(const char *in,  const char *out)
{
    EFIL *e1, *e2;
    FIELD *f;
    header hdr;
    uint64_t fdef, nor, noc;
    FELT min1;
    DSPACE ds;
    Dfmt *v1;
    uint64_t i;

    e1 = ERHdr(in, hdr.hdr);
    fdef = hdr.named.fdef;
    nor = hdr.named.nor;
    noc = hdr.named.noc;

    f = malloc(FIELDLEN);
    if (f == NULL) {
      LogString(81,"Can't malloc field structure");
      exit(22);
    }
    FieldASet(fdef, f);
    min1 = FieldNeg(f, 1);
    e2 = EWHdr(out, hdr.hdr);
    DSSet(f, noc, &ds);
    v1 = malloc(ds.nob);
    for(i = 0; i < nor; i++) {
      ERData(e1,ds.nob, v1);
      DSMul(&ds, min1, 1, v1);
      EWData(e2, ds.nob, v1);
    }
    free(v1);
    free(f);
    ERClose(e1);
    EWClose(e2);
}

#if !jgt
/* Get the selected and non selected rows from a generator */
static void fRowExtract(const char *bs, const char *in, const char *sel, const char *nsel)
{
  /*
   * Open the file, open the bs
   * check file has rows = total bits in bitstring
   * Loop read a row, if bit clear output else ignore
   * Pool
   */
  EFIL *ebs, *ei, *eos, *eon; /* bitstring, in, out selected, out non selected */
  header hdrbs, hdrio;
  FIELD *f;
  DSPACE ds; /* Matrix in */
  Dfmt *mi; /* Matrix out */
  uint64_t nor, noc, j, fdef, size;
  uint64_t *bst;
  ebs = ERHdr(bs, hdrbs.hdr);
  ei = ERHdr(in, hdrio.hdr);
  nor = hdrbs.named.nor; /* Total bits */
  noc = hdrio.named.noc; /* Elements per row */
  fdef = hdrio.named.fdef;
  if (nor != hdrio.named.nor) {
    /* Should be same as for generator */
    LogString(90, "fRowExtract: different number of rows or columns");
    fprintf(stderr, "%s: %s, %s different number of rows or columns, exiting\n", prog_name, bs, in);
    exit(1);
  }
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  /* Create the output header  for selected*/
  hdrio.named.nor = noc; /* Only selected rows */
  eos = EWHdr(sel, hdrio.hdr);
  /* Create the output header  for non selected*/
  hdrio.named.nor = nor - noc; /* Only non selected rows */
  eon = EWHdr(nsel, hdrio.hdr);
  DSSet(f, noc, &ds); /* input/output space */
  mi = malloc(ds.nob);
  /* Read the bitstring */
  size = 8 * (2 + (nor + 63) / 64);
  bst = malloc(size);
  ERData(ebs, size, (uint8_t *)bst);
  for (j = 0; j < nor; j++) {
    ERData(ei, ds.nob, mi);
    if (BSBitRead(bst, j)) {
      EWData(eos, ds.nob, mi);
    } else {
      EWData(eon, ds.nob, mi);
    }
  }
  EWClose1(eos, 0);
  EWClose1(eon, 0);
  ERClose1(ei, 0);
  ERClose1(ebs, 0);
  free(mi);
  free(bst);
  free(f);
}
#endif

int main(int argc, const char *argv[])
{
  /* Set up tmp stem tmp_pid*/
  const char *tmp_root = tmp_name();
  /* How long the temporary filename root is */
  size_t tmp_len = strlen(tmp_root);
  /* The non selected rows */
  char *seln = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Selected columns of non selected rows */
  char *selc = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Non selected columns of non selected rows */
  char *selr = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Negated multiply result */
  char *selmn = mk_tmp(prog_name, tmp_root, tmp_len);
  /* Multiply result */
  char *selm = mk_tmp(prog_name, tmp_root, tmp_len);
  /* junk */
  char *junk = mk_tmp(prog_name, tmp_root, tmp_len);
  /* The root of the subspace */
  const char *sub_root;
  unsigned int sub_root_len;
  char *sub_bs;
  char *sub_rem;
#if jgt
  header hdr;
#endif

  CLogCmd(argc, argv);
  /* Check command line <vecs> <output stem> [<gen>*] */
  /* Must be exactly 3 args */
  if (argc != 4) {
    LogString(80,"usage zsa <subspace stem> <generator> <output>");
    exit(21);
  }
  sub_root = argv[1];
  sub_root_len = strlen(sub_root);
  sub_bs = malloc(sub_root_len + 4);
  sub_rem = malloc(sub_root_len + 5);
  strcpy(sub_bs, sub_root);
  strcat(sub_bs, ".bs");
  strcpy(sub_rem, sub_root);
  strcat(sub_rem, ".rem");
#if !jgt
  /*
   * Algorithm (Parker original)
   * zcx sub.bs gen selc junk
   * zrx sub.bs selc selr selcn
   * zmu selc sub.rem selm
   * zng selm slemn
   * zad selmn selr out
   */
  /* Row select on gen (argv[2]) */
  fColumnExtract(sub_bs, 0, argv[2], 0, selc, 0, junk, 0);
  fRowExtract(sub_bs, selc, selr, seln);
  fMultiply(fun_tmp, sub_rem, 0, seln, 0, selm, 0);
  fNegate(selm, selmn);
  fAdd(selm, 0, selr, 0, argv[3], 0);
#else
  /*
   * Algorithm jgt
   * make_plain bs rem selc
   * zcx bs gen seln junk
   * zmu selc seln selr
   * zng seln out
   */
  EPeek(sub_rem, hdr.hdr);
  make_plain(NULL, sub_bs, sub_rem, selc, hdr.named.fdef);
  fColumnExtract(sub_bs, 0, argv[2], 0, seln, 0, junk, 0);
  fMultiply(fun_tmp, selc, 0, seln, 0, selr, 0);
  fNegate(selr, argv[3]);
#endif
  remove(junk);
  remove(seln);
  remove(selc);
  remove(selr);
  remove(selm);
  remove(selmn);
  free(junk);
  free(seln);
  free(selc);
  free(selr);
  free(selm);
  free(selmn);
  return 0;
}
