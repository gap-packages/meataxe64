/*
      zad.c     meataxe-64 Matrix addition program
      =====     R. A. Parker 01.06.17
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
    if (argc != 4) 
    {
        LogString(80,"usage zad <m1> <m2> <sum>");
        exit(14);
    }
    fAdd(argv[1], 0, argv[2], 0, argv[3], 0);    // just call fAdd
    return 0;
}

/* end of zad.c  */
