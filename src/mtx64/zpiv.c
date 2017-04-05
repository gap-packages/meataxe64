/*
      zpiv.c     MTX64 permutation invert
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
    EFIL *e1,*e2;
    uint64 * inv;
    uint64 hdr[5];
    uint64 nor;
    uint64 i,k;
    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zpiv inp outp");
        exit(21);
    }
    e1 = ERHdr(argv[1],hdr);

    nor=hdr[2];

// missing checks that nor=noc and invertible.

    inv=malloc(8*nor);

    for(i=0;i<nor;i++)
    {
        ERData(e1,8,(uint8 *) &k);
        inv[k]=i;
    }
    ERClose(e1);

    e2 = EWHdr(argv[2],hdr);
    EWData(e2,8*nor,(uint8 *)inv);
    EWClose(e2);
    free(inv);
    return 0;

}      /******  end of zpiv.c    ******/
