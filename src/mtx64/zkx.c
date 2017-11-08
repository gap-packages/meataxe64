/*
      zkx.c     Meataxe-64 Field Extend program
      =====     R. A. Parker 29.9.17
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "field.h"
#include "funs.h"

int main(int argc,  char **argv)
{
    uint64_t fdef2;
    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zkx <m1> <field-order> <m2>");
        exit(14);
    }
    fdef2=strtoul(argv[2],NULL,0);
    fFieldExtend(argv[1], 0, fdef2, argv[3], 0);
    return 0;
}

/* end of zkx.c  */
