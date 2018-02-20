/*
      zpc.c     meataxe-64 Pivot Combine
      =====     R. A. Parker    15.07.2014
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
        LogString(80,"usage zpc b1 b2 combined rowrif");
        exit(21);
    }
    fPivotCombine(argv[1], 0, argv[2], 0, argv[3], 0, argv[4], 0);
    return 0;

}      /******  end of zpc.c    ******/
