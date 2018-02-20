// Copyright (C) Richard Parker   2017
// zmu.c     meataxe-64 Nikolaus version Multiply program

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
        LogString(80,"usage zmu <m1> <m2> <product>");
        exit(14);
    }
    fMultiply("temp",argv[1], 0, argv[2], 0, argv[3], 0);    // just call fMultiply
    return 0;
}

/* end of zmu.c  */
