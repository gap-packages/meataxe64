/*
      znu.c     meataxe-64 Null-Space program
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
    uint64_t nullity;
    char st[200];
    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage znu <m1> <ns>");
        exit(14);
    }
    nullity=fNullSpace("temp",argv[1],0,argv[2],0);
    sprintf(st,"Nullity %lu",nullity);
    printf("%s\n",st);
    LogString(20,st);
    return 0;
}

/*  end of znu.c    */
