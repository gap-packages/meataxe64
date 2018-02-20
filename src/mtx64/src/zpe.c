/*
      zpe.c     meataxe-64 Produce Echelon Form program
      =====     R. A. Parker 2.6.17
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "funs.h"

int main(int argc,  char **argv)
{
    uint64_t rank;
    char st[200];
    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zpe m cs rem");
        exit(14);
    }
// make the temps properly!

    rank=fProduceNREF("temp",argv[1],0,argv[2],0,argv[3],0);
    sprintf(st,"Rank of %s is %lu", argv[1], rank);
    printf("%s\n", st);
    LogString(20, st);
    return 0;
}

/*  end of zpe.c    */
