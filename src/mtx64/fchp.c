/*
         fchp.c  -   meataxe64 Level III function - chop
         ======      J. G. Thackray 25.08.2016
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "util.h"

char ***fchp(const char *fname, const char *tmp, unsigned int rchops, unsigned int cchops)
{
  uint64 hdr1[5], hdr2[5];
  FIELD *f;
  EFIL *e1;
  EFIL **e2;
  DSPACE ds1,ds2;
  uint64 nor, noc, fdef, sub_nor, sub_noc;
  uint64 *rch, *cch;
  uint64 i, j;
  unsigned int k;
  Dfmt *v1, *v2;
  char ***tmpnames;

  e1 = ERHdr(fname, hdr1);
  nor=hdr1[2];
  noc=hdr1[3];
  fdef=hdr1[1];
  f = malloc(FIELDLEN);
  if (NULL == f)  {
    LogString(81,"Can't malloc field structure");
    exit(22);
  }
  if (nor < rchops || noc < cchops) {
    LogString(82,"Can't chop too small");
    exit(23);
  }
  FieldASet(fdef , f);
  DSSet(f, noc, &ds1);
  /* Set up the row and column sizes */
  sub_nor = nor / rchops;
  sub_noc = noc / cchops;
  rch = malloc(sizeof(*rch) * rchops);
  cch = malloc(sizeof(*cch) * cchops);
  for (i = 0; i < rchops; i++) {
    rch[i] = sub_nor;
  }
  j = nor - sub_nor * rchops;
  /* Distribute the spare rows evenly */
  for (i = 0; i < j; i++) {
    rch[i]++;
  }
  /* Now do the same for the columns */
  for (i = 0; i < cchops; i++) {
    cch[i] = sub_noc;
  }
  j = noc - sub_noc * cchops;
  /* Distribute the spare columns evenly */
  for (i = 0; i < j; i++) {
    cch[i]++;
  }
  /* We set up the temporary filenames */
  tmpnames = mk_tmps(tmp, rchops, cchops);
  /* Got all the names, now start putting stuff in them */
  v1 = malloc(ds1.nob); /* Allocate an incoing matrix row */
  e2 = malloc(sizeof(*e2) * cchops); /* Allocate the outbound file structures */
  v2 = malloc(ds1.nob); /* Bound to be big enough */
  hdr2[0] = hdr1[0];
  hdr2[1] = hdr1[1];
  hdr2[4] = hdr1[4];
  for (i = 0; i < rchops; i++) { /* Loop over rows of files */
    hdr2[2] = rch[i]; /* Rows in this set of files*/
    for (k = 0; k < cchops; k++) {
      hdr2[3] = cch[k]; /*Columns in this set of files*/
      e2[k] = EWHdr(tmpnames[i][k], hdr2);
    }
    for (j = 0; j < rch[i]; j++) { /* Loop over rows within output file */
      uint64 start_col = 0;
      ERData(e1, ds1.nob, v1); /* Read an inout row */
      for (k = 0; k < cchops; k++) { /* Loop over file columns */
        DSSet(f, cch[k], &ds2);
        DCut(&ds1, 1, start_col, v1, &ds2, v2);
        EWData(e2[k], ds2.nob, v2);
        start_col += cch[k]; /* Move the start column on */
      }
    }
    for (k = 0; k < cchops; k++) {
      EWClose1(e2[k],1);
    }
  }
  ERClose(e1);
  free(v2);
  free(v1);
  free(rch);
  free(cch);
  return tmpnames;
}
