/*
      zs2.c     meataxe-64 Symmetric Square program
      =====     R. A. Parker 28.9.17
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
        LogString(80,"usage zs2 <m1> <m2>");
        exit(14);
    }
    fSymmetricSquare(argv[1], 0, argv[2], 0); // just call fSymmetricSquare
    return 0;
}

/* end of zs2.c  */
