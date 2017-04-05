/*
      zpc.c     meataxe-64 Pivot Combine
      =====     R. A. Parker    15.07.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3,*e4;
    uint64 nor,nor2,noc1, noc2, noc;
    uint64 hdr1[5],hdr2[5];
    size_t siz,siz4;
    uint64 *bs1,*bs2,*bs3,*bs4;

    LogCmd(argc,argv);
    if (argc != 5)
    {
        LogString(80,"usage zpc b1 b2 combined rowrif");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);   // bs
    e2=ERHdr(argv[2],hdr2);   // matrix set
    nor=hdr1[2];  // total bits
    noc1=hdr1[3];
    nor2=hdr2[2];
    if(nor2!=nor-noc1)
    {
        LogString(80,"Strings incompatible");
        exit(22);
    }
    noc2=hdr2[3];
    noc=noc1+noc2;
    hdr1[3]=noc;
    e3=EWHdr(argv[3],hdr1);
    hdr1[2]=noc;
    hdr1[3]=noc1;
    e4=EWHdr(argv[4],hdr1);

    siz = 8*(2+(nor+63)/64);
    bs1=malloc(siz);
    ERData(e1,siz,(uint8 *)bs1);
    ERClose(e1);
    siz = 8*(2+(nor2+63)/64);
    bs2=malloc(siz);
    ERData(e2,siz,(uint8 *)bs2);
    ERClose(e2);
    siz = 8*(2+(nor+63)/64);
    bs3=malloc(siz);
    memset(bs3,0,siz);
    siz4 = 8*(2+(noc+63)/64);
    bs4=malloc(siz4);
    memset(bs4,0,siz4);

    BSCombine(bs1,bs2,bs3,bs4);
    EWData(e3,siz ,(uint8 *)bs3);
    EWData(e4,siz4,(uint8 *)bs4);

    EWClose(e3);
    EWClose(e4);
    free(bs1);
    free(bs2);
    free(bs3);
    free(bs4);
    return 0;

}      /******  end of zpc.c    ******/
