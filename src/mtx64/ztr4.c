/*
      ztr4.c     meataxe-64 big (IV) transpose
      ======     J. G. Thackray 28.8.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "tuning.h"
#include "tfarm.h"
#include "funs.h"
#include "util.h"

int main(int argc,  char **argv)
{
  const char *in, *out;
  uint64 hdr[5];
  uint64 nor, noc, fdef;
  unsigned int rchops, cchops, chops;
  EFIL *e1;
  const char *tmp_in;
  char *tmp_out;
  char ***tmpnames_in, ***tmpnames_out;
  unsigned int i, j;
  DSPACE ds;
  FIELD *f;
  uint64 mem1,mem2;

  LogCmd(argc, argv);
  /******  First check the number of input arguments  */
  if (3 != argc)  {
    LogString(80,"usage ztr m1 ans");
    exit(21);
  }
  in = argv[1];
  out = argv[2];
  /*
   * TBD: work out how to split stuff up
   * We want effectively to split rows : columns = file rows : file columns
   * But to cope with bigger primes, instead of noc we use ds.nob * CHAR_BIT
   * For GF(2) this produces the same answer, whereas for bigger primes we
   * correspondingly reduce the number of columns
   */
  EPeek(in, hdr);
  nor = hdr[2];
  noc = hdr[3];
  /*
   * Handle the following cases specially
   * 1. nor == 0 || noc == 0
   * 2. chops = 1ftr
   */
  if (0 == noc || 0 == nor) {
    /* No point, we just want a noc by nor empty matrix */
    hdr[2] = noc;
    hdr[3] = nor; /* Swap parameters */
    /* Open the output, then finish it */
    e1 = EWHdr(out, hdr);
    EWClose(e1);
    return 0;
  }
  /* If A plus one row of B fits in memory, don't bother to chop */
  fdef = hdr[1];
  f = malloc(FIELDLEN);
  if (NULL == f) {
    LogString(81,"Can't malloc field structftrure");
    exit(24);
  }
  FieldSet(fdef, f);
  DSSet(f, noc, &ds);
  mem1 = ds.nob * nor; /* Space for A */
  DSSet(f, nor, &ds);
  mem2 = ds.nob * noc; /* Space for B */
  free(f);
/* allowed to use 10% of available memory only  */
  if (((mem1+mem2)/100000) <= MEGABYTES) {
    ftra(in, 0, out, 0);
    return 0;
  }
  chops=2;
  while( (mem1+mem2)/(chops*chops*100000)>MEGABYTES) chops++;

  rchops = chops;
  cchops = chops;
  tmp_in = tmp_name();
  tmp_out = malloc(strlen(tmp_in) + 5); /* +1 for EOS, +4 for ".tra" */
  strcpy(tmp_out, tmp_in);
  strcat(tmp_out, ".tra");
  tmpnames_in = fchp(in, tmp_in, rchops, cchops);
  tmpnames_out = mk_tmps(tmp_out, rchops, cchops);
  for (i = 0; i < rchops; i++) {
    for (j = 0; j < cchops; j++) {
      ftra(tmpnames_in[i][j], 1, tmpnames_out[j][i], 1);
    }
  }
  fasp(out, tmpnames_out, rchops, cchops);
  /* Now delete the temporary files */
  for (i = 0; i < rchops; i++) {
    for (j = 0; j < cchops; j++) {
      remove(tmpnames_in[i][j]);
      remove(tmpnames_out[i][j]);
    }
  }
  free_tmpnames(tmpnames_in, rchops, cchops);
  free_tmpnames(tmpnames_out, rchops, cchops);
  free(tmp_out);
  return 0;
}
