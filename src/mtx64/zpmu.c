/*
      zpmu.c     MTX64 permutation (map) multiply
      ======     R. A. Parker   9.8.2015 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3;
    uint64 * p2;
    uint64 i,k;
    uint64 hdr1[5],hdr2[5];
    uint64 nor1,noc1,nor2;

    LogCmd(argc,argv);
    if (argc != 4)
    {
        LogString(80,"usage zpmu inp1 inp2 prod");
        exit(21);
    }
/* read in headers */
    e1 = ERHdr(argv[1],hdr1);
    nor1=hdr1[2];
    noc1=hdr1[3];
    e2 = ERHdr(argv[2],hdr2);
    nor2=hdr2[2];
    if (nor2 != noc1)
    {
        LogString(80,"permutations or maps incompatible");
        exit(21);
    }
    p2=malloc(8*nor2);
    ERData(e2,8*nor2,(uint8 *) p2);
    hdr2[2]=nor1;
    e3 = EWHdr(argv[3],hdr2);
    for(i=0;i<nor1;i++)
    {
        ERData(e1,8,(uint8 *) &k);
        EWData(e3,8,(uint8 *)(p2+k));
    }
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    free(p2);
    return 0;

}      /******  end of zpmu.c    ******/
