/*
              ftra.c     Disk chopping transpose
              ======     R. A. Parker   1.6.2017
     Does not yet chop on disk
 */

#include <stdint.h>
#include <stdlib.h>
#include "mezz.h"
#include "field.h"
#include "funs.h"

void fTranspose(const char *tmp, const char *in, int sin, 
         const char *out, int sout)
{
    mtra(in,sin,out,sout);
    return;
}

/* end of ftra.c  */
