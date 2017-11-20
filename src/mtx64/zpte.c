/*
      zpmu.c     MTX64 permutation (map) tensor
      ======     R. A. Parker   10.8.2015 
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
    uint64_t * p2;
    uint64_t i,j,k,t;
    uint64_t hdr1[5],hdr2[5],hdr3[5];
    uint64_t nor1,noc1,nor2,noc2;

    LogCmd(argc,argv);
    if (argc != 4)
    {
        LogString(80,"usage zpte inp1 inp2 tens");
        exit(21);
    }
/* read in headers */
    e1 = ERHdr(argv[1],hdr1);
    nor1=hdr1[2];
    noc1=hdr1[3];
    e2 = ERHdr(argv[2],hdr2);
    nor2=hdr2[2];
    noc2=hdr2[3];

    p2=malloc(8*nor2);
    ERData(e2,8*nor2,(uint8_t *) p2);
    hdr3[0]=3;
    hdr3[1]=1;
    hdr3[2]=nor1*nor2;
    hdr3[3]=noc1*noc2;
    hdr3[4]=0;
    e3 = EWHdr(argv[3],hdr3);
    for(i=0;i<nor1;i++)
    {
        ERData(e1,8,(uint8_t *) &k);
        for(j=0;j<nor2;j++)
        {
            t=k*noc1+p2[j];
            EWData(e3,8,(uint8_t *)&t);
        }
    }
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    return 0;

}      /******  end of zpmu.c    ******/
