// Copyright (C) Richard Parker   2017
//      ztr.c   meataxe-64 Nikolaus version Transpose program

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
        LogString(80,"usage ztr <matin> <transout>");
        exit(14);
    }
    fTranspose("temp",argv[1], 0, argv[2], 0);    // just call fTranspose
    return 0;
}

/* end of ztr.c  */
