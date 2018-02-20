/*
      ze2.c     meataxe-64 Exterior Square program
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
        LogString(80,"usage ze2 <m1> <m2>");
        exit(14);
    }
    fExteriorSquare(argv[1], 0, argv[2], 0);    // just call fExteriorSquare
    return 0;
}

/* end of ze2.c  */
