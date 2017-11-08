/*
      ztc.c     Meataxe-64 Trace program
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
    char st[200];
    FELT f;
    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage ztc <m1>");
        exit(14);
    }
    f=fTrace(argv[1], 0);    // just call fTrace
    sprintf(st,"Trace of %s is %ld",argv[1],f);
    printf("%s\n",st);
    LogString(20,st);
    return 0;
}

/* end of ztc.c  */
