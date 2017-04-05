/*
      znu1.c     meataxe-64 Version 2.0 Null-Space program
      ======     R. A. Parker 27.11.16
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "funs.h"

char tmp1[] = "tmp1";
char tmp2[] = "tmp2";
char tmp3[] = "tmp3";
char tmp4[] = "tmp4";
char tmp5[] = "tmp5";

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
// make the temps properly!

    ftra(argv[1], 0, tmp1, 1);
    fpef(tmp1,tmp2,tmp3,1);
    ftra(tmp3,1,tmp4,1);
    nullity=fmkn(tmp2,1,tmp4,1,argv[2],0);
    sprintf(st,"Nullity %lu",nullity);
    printf("%s\n",st);
    LogString(20,st);

//printf rank
//delete temps
}

/*  end of znu4.c    */
