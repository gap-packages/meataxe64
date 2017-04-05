/*
      zmu4.c     meataxe-64 big (IV) multiply
      ======     J. G. Thackray 30.8.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "field.h"
#include "io.h"
#include "tuning.h"
#include "tfarm.h"
#include "funs.h"
#include "util.h"

int main(int argc, char ** argv)
{
  const char *ina, *inb, *out;
  uint64 hdra[5];
  uint64 hdrb[5];
  uint64 nora, noca, norb, nocb, mem1, mem2;
  unsigned int rchopsa, cchopsa, cchopsb, chops;
  const char *tmp;
  unsigned int len;
  char *tmp_a, *tmp_b, *tmp_c, *inter1, *inter2;
  char ***tmpnames_a, ***tmpnames_b, ***tmpnames_c;
  char *tt;
  unsigned int i, j, k;
  EFIL *e;
  FIELD *f;
  uint64 fdef;
  DSPACE ds;

  LogCmd(argc, argv);
  /******  First check the number of input arguments  */
  if (4 != argc)  {
    LogString(80,"usage zmu a b c");
    exit(21);
  }
  ina = argv[1];
  inb = argv[2];
  out = argv[3];
  /* Check compatibility of A and B, no point in proceeding if incompatible*/
  EPeek(ina, hdra);
  fdef = hdra[1];
  nora = hdra[2];
  noca = hdra[3];
  EPeek(inb, hdrb);
  norb = hdrb[2];
  nocb = hdrb[3];
  if(fdef != hdrb[1]) {
    LogString(80,"Matrices have different fields");
    exit(22);
  }
  if (norb != noca) {
    LogString(80,"Matrices have incompatible shapes");
    exit(23);
  }
  /*
   * First handle some special cases
   * 1. noca = 0, just want a nora x nocb zero matrix
   * 2. nora = 0, just want a 0 x nocb matrix
   * 3. nocb = 0, just want a nora x 0 matrix
   * 4. B fits in 70% memory, just call fmu directly
   * 5. A+C fits in 70% memory, just call fmu directly
   */
  if (0 == nora || 0 == nocb) {
    /* Cases 2 and 3 above */
    hdra[3] = nocb; /* Just change the number of columns */
    /* Open the output, then finish it */
    e = EWHdr(out, hdra);
    EWClose(e);
    return 0;
  }
  f = malloc(FIELDLEN);
  FieldSet(fdef, f);
  if (0 == noca) {
    /* Case 1 above*/
    Dfmt *v;

    hdra[3] = nocb;
    /* Open the output, fill with zero, then close it */
    e = EWHdr(out, hdra);
    DSSet(f, nocb, &ds);
    v = malloc(ds.nob);
    memset(v, 0, ds.nob);
    for (j= 0; j < nora; j++) {
      EWData(e, ds.nob, v);
    }
    EWClose(e);
    free(v);
    free(f);
    return 0;
  }
  /*
   * The next two cases handle all in memory options.
   * We don't need to beware of overflow in the computation of mem,
   * as we aren't going to have files of size 16 * 10^18!
   * Apart from that, the test is the same for each
   */
  /* Now try A+C in 70% memory case */
  DSSet(f, noca, &ds);
  mem1 = ds.nob * nora; /* Memory for A */
  DSSet(f, nocb, &ds);
  mem1 += ds.nob * nora; /* Memory for C */
  /* Now try B in 70% memory case */
  DSSet(f, nocb, &ds);
  mem2 = ds.nob * norb;
  if (mem1 < 7 * MEGABYTES * 100000 || mem2 < 7 * MEGABYTES * 100000) { /* 70% of total memory allowed */
    mzma(out,0,ina,0,inb,0,NULL,0,MEGABYTES);
    free(f);
    return 0;
  }

/* Chopping decisions.  Given that the matrix must be big,
   as otherwise it doesn't get here this isn't so silly  */
  chops = 3;
  rchopsa = chops;
  cchopsa = chops;
  cchopsb = chops;
/* end of chopping decisions */

  tmp = tmp_name();
  len = strlen(tmp) + 2; /* 1 for EOS, one for a/b/c/i */
  tmp_a = malloc(len);
  tmp_b = malloc(len);
  tmp_c = malloc(len);
  inter1 = malloc(len);
  inter2 = malloc(len);
  strcpy(tmp_a, tmp);
  strcat(tmp_a, "A");
  strcpy(tmp_b, tmp);
  strcat(tmp_b, "B");
  strcpy(tmp_c, tmp);
  strcat(tmp_c, "C");
  strcpy(inter1, tmp);
  strcat(inter1, "i");
  strcpy(inter2, tmp);
  strcat(inter2, "j");
  tmpnames_a = fchp(ina, tmp_a, rchopsa, cchopsa); /* l x m */
  tmpnames_b = fchp(inb, tmp_b, cchopsa, cchopsb); /* m x n*/
  tmpnames_c = fchp(out, tmp_c, rchopsa, cchopsb); /* l x n*/
  /* Time for a multiply loop */
  for (i = 0; i < rchopsa; i++) {
    for (j = 0; j < cchopsb; j++) {
      /* First result has nothing to add in, just mu((Ai,0),(B0,j) */
      mzma(inter1,1,tmpnames_a[i][0],1, tmpnames_b[0][j],1,NULL,0,MEGABYTES);
      /* Then multiply-and-add the remaining cases */
      for (k = 1; k < cchopsa; k++)
      {
          mzma(inter2,1,tmpnames_a[i][k],1,tmpnames_b[k][j],1,inter1,1,MEGABYTES);
          tt=inter1;
          inter1=inter2;
          inter2=tt;
      }
      rename(inter1, tmpnames_c[i][j]);
    }
  }
  /* Now put it all back together */
  fasp(out, tmpnames_c, rchopsa, cchopsb);
  /* Now delete the temporary files */
  for (i = 0; i < rchopsa; i++) {
    for (j = 0; j < cchopsa; j++) {
      remove(tmpnames_a[i][j]);
    }
  }
  for (i = 0; i < cchopsa; i++) {
    for (j = 0; j < cchopsb; j++) {
      remove(tmpnames_b[i][j]);
    }
  }
  for (i = 0; i < rchopsa; i++) {
    for (j = 0; j < cchopsb; j++) {
      remove(tmpnames_c[i][j]);
    }
  }
  free_tmpnames(tmpnames_a, rchopsa, cchopsa);
  free_tmpnames(tmpnames_b, cchopsa, cchopsb);
  free_tmpnames(tmpnames_c, rchopsa, cchopsb);
  free(inter1);
  free(inter2);
  free(tmp_a);
  free(tmp_b);
  free(tmp_c);
  return 0;
}
