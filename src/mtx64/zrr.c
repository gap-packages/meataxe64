/*
      zrr.c     meataxe-64 row riffle
      =====     R. A. Parker   20.9.2017
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
    if (argc != 5)
    {
        LogString(80,"usage zrr bs m1 m0 m");
        exit(21);
    }
    fRowRiffle(argv[1], 0, argv[2], 0, argv[3], 0, argv[4], 0);
    return 0;
}
      /******  end of zrr.c    ******/
