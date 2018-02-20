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
    TLS tls;
    uint64_t hdr[5];
    uint64_t rank;
    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage ziv m iv");
        exit(14);
    }
    EPeek(argv[1],hdr);
    if(hdr[2]!=hdr[3])
    {
        printf("Invert - matrix not square\n");
        exit(15);
    }

    tls.f = malloc(FIELDLEN);
    FieldSet(hdr[1],tls.f);
    rank=fech(argv[1],0,"NULL",1,"NULL",1,"temp1",1,
         "NULL",1,"NULL",1);
    if(rank!=hdr[2])
    {
        printf("Invert - Matrix is singular\n");
        remove("temp1");
        exit(13);
    }
    fsmu(&tls,"temp1",1,argv[2],0,((tls.f)->charc)-1);
    remove("temp1");
    free(tls.f);
    return 0;
}

/*  end of ziv.c    */
