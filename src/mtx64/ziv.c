// Copyright (C) Richard Parker   2017
// ziv.c     meataxe-64 Nikolaus version Invert

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "funs.h"
#include "slab.h"

int main(int argc,  char **argv)
{
    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage ziv m iv");
        exit(14);
    }
    fInvert("temp",argv[1],0,argv[2],0);
    return 0;
}

/*  end of ziv.c    */
