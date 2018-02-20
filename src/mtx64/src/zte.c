/*
      zte.c     meataxe-64 Matrix tensor program
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
    if (argc != 4) 
    {
        LogString(80,"usage zte <m1> <m2> <product>");
        exit(14);
    }
    fTensor(argv[1], 0, argv[2], 0, argv[3], 0);    // just call fTensor
    return 0;
}

/* end of zte.c  */
