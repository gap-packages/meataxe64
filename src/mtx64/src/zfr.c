// Copyright (C) Richard Parker   2017
// zfr.c     meataxe-64 Nikolaus version Frobenius automorphism program

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
        LogString(80,"usage zfr <m1> <m2>");
        exit(14);
    }
    fFrobenius(argv[1], 0, argv[2], 0);    // just call fFrobenius
    return 0;
}

/* end of zfr.c  */
