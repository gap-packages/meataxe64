/*
         fadc.c  -   meataxe64 Level III function - add
         ======      J. G. Thackray 27.08.2016
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
#include "funs.h"

void fadd(const char *out, const char *ina, const char *inb, int silent)
{
  EFIL *e1, *e2, *e3;
  FIELD * f;
  uint64 hdr1[5], hdr2[5];
  uint64 fdef, nor, noc;
  DSPACE ds;
  Dfmt *v1, *v2;
  uint64 i;

  e1 = ERHdr(ina, hdr1);
  e2 = ERHdr(inb, hdr2);
  if ((hdr1[1] != hdr2[1]) || (hdr1[2] != hdr2[2]) || (hdr1[3] != hdr2[3]) ) {
    LogString(80, "Matrices incompatible");
    exit(22);
  }
  fdef = hdr1[1];
  nor = hdr1[2];
  noc = hdr1[3];
  f =  malloc(FIELDLEN);
  if (f == NULL) {
    LogString(81, "Can't malloc field structure");
    exit(22);
  }
  FieldSet(fdef, f);
  e3 = EWHdr(out, hdr1);
  DSSet(f, noc, &ds);
  v1 = malloc(ds.nob);
  v2 = malloc(ds.nob);
  /******  Do them one row at a time  */
  for (i = 0; i < nor; i++) {
    ERData(e1, ds.nob, v1);
    ERData(e2, ds.nob, v2);
    DAdd(&ds, 1, v1, v2, v2);
    EWData(e3, ds.nob, v2);
  }
  free(v1);
  free(v2);
  free(f);
  ERClose1(e1,silent);
  ERClose1(e2,silent);
  EWClose1(e3,silent);

}
