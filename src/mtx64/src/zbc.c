/*
      zpc.c     meataxe-64 BitString Complement
      =====     R. A. Parker    18.05.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
#include "pcrit.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64_t nor, noc;
    uint64_t hdr[5];
    size_t siz;
    uint64_t *bs1,*bs2;
    uint64_t i;

    LogCmd(argc,argv);
    if (argc != 3)
    {
        LogString(80,"usage zbc bs comp");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);    // bs in
    nor=hdr[2];  // total bits
    noc=hdr[3];  // set bits
    hdr[3]=nor-noc;
    e2=EWHdr(argv[2],hdr);
    siz = 8*(2+(nor+63)/64);
    bs1=malloc(siz);
    bs2=malloc(siz);
    ERData(e1,siz,(uint8_t *)bs1);
    ERClose(e1);
    memset(bs2,0,siz);
    bs2[0]=nor;
    bs2[1]=nor-noc;
    hdr[3]=nor-noc;
    for(i=0;i<nor;i++)
        if(BSBitRead(bs1, i)==0) BSBitSet (bs2,i);

    EWData(e2,siz ,(uint8_t *)bs2);

    EWClose(e2);

    free(bs1);
    free(bs2);

    return 0;

}      /******  end of zbc.c    ******/
