/*
         fasp.c  -   meataxe64 Level III function - assemble
         ======      J. G. Thackray 25.08.2016
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"

void fasp(const char *fname, char ***tmpnames, unsigned int rchops, unsigned int cchops)
{
  uint64 hdr[5];
  uint64 fdef, nor, noc, nor1, noc1;
  FIELD *f;
  DSPACE ds, dsv;
  Dfmt *v1, *v2;
  uint64 i, j, k;
  EFIL **e2;
  EFIL *e;
  uint64 *noc_seg; 

  EPeek(tmpnames[0][0], hdr);
  fdef = hdr[1];
  nor = hdr[2];
  noc = hdr[3];
  f = malloc(FIELDLEN);
  FieldASet(fdef, f);
  DSSet(f, noc, &ds);
  /* Work out full nor and noc. Note we've already got the (0,0) value */
  for (i = 1; i < rchops; i++) {
    EPeek(tmpnames[i][0], hdr);
    nor += hdr[2];
  }
  for (j = 1; j < cchops; j++) {
    EPeek(tmpnames[0][j], hdr);
    noc += hdr[3];
  }
  DSSet(f, noc, &ds);
  v1 = malloc(ds.nob);
  v2 = malloc(ds.nob);
  hdr[2] = nor;
  hdr[3] = noc;
  e = EWHdr(fname, hdr);
  e2 = malloc(sizeof(*e2) * cchops);
  nor1 = 0; /* stop compiler warnings */
  noc_seg = malloc(sizeof(*noc_seg) * cchops);
  for (i = 0; i < rchops; i++) {
    noc1 = 0;
    for (j = 0; j < cchops; j++) {
      e2[j] = ERHdr(tmpnames[i][j], hdr);
      if (j == 0) {
        nor1 = hdr[2];
      } else {
        if (hdr[2] != nor1) {
          LogString(80, "matrices incompatible");
          exit(21);
        }
      }
      noc_seg[j] = hdr[3]; /* Record the number of columns of each file */
      noc1 += noc_seg[j]; /* Record output columns */
    }
    /* Check same number of columns throughout */
    if (noc1 != noc) {
      LogString(80, "matrices incompatible");
      exit(21);
    }
    for (k = 0; k < nor1; k++) {
      memset(v2, 0, ds.nob);
      noc1 = 0;
      for (j = 0; j < cchops; j++) {
        DSSet(f, noc_seg[j], &dsv);
        ERData(e2[j], dsv.nob, v2); /* Read one row of file j */
        DPaste(&dsv, v2, 1, noc1, &ds, v1); /* Paste it on the end of the output */
        noc1 += noc_seg[j];
      }
      EWData(e, ds.nob, v1);
    }
    for (j = 0; j < rchops; j++) {
      ERClose1(e2[j],1);
    }
  }
  EWClose(e);
  free(e2);
  free(noc_seg);
  return;
}

/* end of fasp.c  */
