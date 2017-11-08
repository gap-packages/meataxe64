/*
      zfr.c     meataxe-64 Frobenius automorphism program
      =====     R. A. Parker 27.9.17
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "field.h"
#include "funs.h"


int main(int argc,  char **argv)
{
    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage zfr <m1> <m2>");
        exit(14);
    }
    fFrobenius(argv[1], 0, argv[2], 0);    // just call fFrobenius
    return 0;
}

/* end of zfr.c  */
