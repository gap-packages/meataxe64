/*
      zsv.c     meataxe-64 Select (projective) vector
      =====     R. A. Parker 1.10.17
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "field.h"
#include "funs.h"

int main(int argc,  char **argv)
{
    uint64_t pvec;
    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zsv <m1> <proj vec no> <m2>");
        exit(14);
    }
    pvec=strtoul(argv[2],NULL,0);
    fProjectiveVector(argv[1], 0, pvec, argv[3], 0);
    return 0;
}

/* end of zsv.c  */
