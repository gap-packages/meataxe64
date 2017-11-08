/*
      zec.c     meataxe-64 Echelize
      =====     R. A. Parker 22.6.17
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
    LogCmd(argc,argv);
    if (argc != 7) 
    {
        LogString(80,"usage zec m rs cs mul cln rem");
        exit(14);
    }
    fech(argv[1],0,argv[2],0,argv[3],0,argv[4],0,
         argv[5],0,argv[6],0);
    return 0;

}

/*  end of zec.c    */
