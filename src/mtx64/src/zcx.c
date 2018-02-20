/*
      zcx.c     meataxe-64 column extract
      =====     R. A. Parker    20.09.2017
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
        LogString(80,"usage zcx <bs> <m1> <sel> <nonsel>");
        exit(14);
    }
    fColumnExtract(argv[1],0,argv[2],0,argv[3],0,argv[4],0);
    return 0;
}

/******  end of zcx.c    ******/
