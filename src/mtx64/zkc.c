/*
      zkc.c     meataxe-64 Matrix Field Contract program
      =====     R. A. Parker 28.9.17
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
    int res;
    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zkc <m1> <field-order> <m2>");
        exit(14);
    }
    fdef2=strtoul(argv[2],NULL,0);
    res=fFieldContract(argv[1], 0, fdef2, argv[3], 0);
    if(res==1)
    {
        LogString(80,"Elements not all in smaller field");
        exit(14);
    }
    return 0;
}

/* end of zkc.c  */
