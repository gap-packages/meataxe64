// Copyright (C) Richard Parker   2019
// Meataxe64 Version 3 Attempt 4.  ffn5.c   Miscellanous small functions
// ****** Incomplete  ******

// Contents
// ffAdd

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "ffn.h"

void ffnAdd(FIELD *f, Dfmt * a, Dfmt * b, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    DSSet(f,noc,&ds);
    DAdd(&ds,nor,a,b,b);
    return;
}

/******  end of ffn5.c    ******/
