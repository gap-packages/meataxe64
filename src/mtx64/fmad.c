/*
         fmad.c  -   meataxe64 Level III function - multiply and add
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

void fmad(const char *out, const char *ina, const char *inb, const char *tmp1, const char *tmp2, int silent)
{
  fmu(tmp1, ina, inb, 1); /* Always silent to the temporary */
  fadd(tmp2, out, tmp1, silent);
  rename(tmp2, out);
}

void fmad1(const char *ina, const char *inb, const char *inc, const char *out, const char *tmp, int silent)
{
  fmu(tmp, ina, inb, 1); /* Always silent to the temporary */
  fadd(out, inc, tmp, silent);
}
