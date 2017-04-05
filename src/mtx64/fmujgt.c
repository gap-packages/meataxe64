/*
         fmu.c  -   meataxe64 Level III function - multiply
         =====      J. G. Thackray 27.08.2016
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "tfarm.h"
#include "proggies.h"
#include "tuning.h"

void fmu(const char *out, const char *ina, const char *inb, int silent)
{
  uint64 fdef, nor1, noc1, nor2, noc2, nor3, noc3;
  uint64 i, j;
  EFIL *ef1, *ef2, *ef3;
  uint64 hdr1[5], hdr2[5], hdr3[5];
  DSPACE ds1, ds2;
  Dfmt *m1, *m2, *m3;
  Dfmt *da, *db;
  FIELD * f;
  FELT e;
  ef1 = ERHdr(ina, hdr1);
  ef2 = ERHdr(inb, hdr2);
  if (hdr1[1] != hdr2[1]) {
    LogString(80, "Matrices have different fields");
    exit(22);
  }
  fdef = hdr1[1];
  nor1 = hdr1[2];
  nor2 = hdr2[2];
  noc1 = hdr1[3];
  noc2 = hdr2[3];
  if (noc1 != nor2) {
    LogString(80, "Matrices have incompatible shapes");
    exit(23);
  }
  f = malloc(FIELDLEN);
  if (f == NULL) {
    LogString(81, "Can't malloc field structure");
    exit(24);
  }
  FieldSet(fdef, f);
  nor3 = nor1;
  noc3 = noc2;
  DSSet(f, noc1, &ds1);
  DSSet(f, noc2, &ds2);
  hdr3[0] = 1;
  hdr3[1] = fdef;
  hdr3[2] = nor3;
  hdr3[3] = noc3;
  hdr3[4] = 0;
  ef3 = EWHdr(out, hdr3);
  /*  Allocate space for all the matrices */
  m1 = malloc(ds1.nob * nor1);
  m2 = malloc(ds2.nob * nor2);
  m3 = malloc(ds2.nob);
  if ((m1 == NULL) || (m2 == NULL) || (m3 == NULL)) {
    LogString(81, "Can't malloc the matrix space");
    exit(23);
  }
  /* first read in matrices 1 and 2 */
  ERData(ef1, nor1 * ds1.nob, m1);
  ERData(ef2, nor2 * ds2.nob, m2);
  if (silent) {
    ERClose1(ef1,1);
    ERClose1(ef2,1);
  } else {
    ERClose(ef1);
    ERClose(ef2);
  }
  da = m1;

  for (i = 0; i<nor1; i++) {
    memset(m3, 0, ds2.nob);
    db = (Dfmt *) m2;
    for (j = 0; j<noc1; j++) {
      e = DUnpak(&ds1, j, da);
      DSMad(&ds2, e, 1, db, m3);
      db+=ds2.nob;
    }
    EWData(ef3, ds2.nob, m3);
    da+=ds1.nob;
  }

  if (silent) {
    EWClose1(ef3,1);
  } else {
    EWClose(ef3);
  }
  free(f);
  free(m1);
  free(m2);
  free(m3);
}
