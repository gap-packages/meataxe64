/*
      zpor.c     MTX64 permutation order
      ======     R. A. Parker   18.8.2015 
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
    EFIL *e;
    uint64_t hdr[5];
    uint64_t nor,lenbs,ord,ord1;
    uint64_t * p;
    long k;
    uint64_t i,j;
    uint64_t * bs;
    char st[200];

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 2)
    {
        LogString(80,"usage zpor perm");
        exit(21);
    }
    e = ERHdr(argv[1],hdr);
    nor=hdr[2];
    p=malloc(8*nor);
    ERData(e,8*nor,(uint8_t *) p);
    ERClose(e);
    lenbs=8*(2+(nor+63)/64);
    bs=malloc(lenbs);
    memset(bs,0,lenbs);
    bs[0]=nor;
    ord=1;
    for(i=0;i<nor;i++)
    {
        if(BSBitRead(bs,i)) continue;
        BSBitSet(bs,i);
        j=p[i];
        k=1;
        while(j!=i)
        {
            BSBitSet(bs,j);
            j=p[j];
            k++;
        }
        ord1=ord;
        while( (ord%k)!=0 )
        {
            ord+=ord1;
            if(ord>1000000000000) break;                
        }
        if(ord>1000000000000) break;  
    }
    if(ord<=1000000000000) sprintf(st,"Order of %s is %ld",argv[1],ord);
    else sprintf(st,"Order of %s is > 1000000000000",argv[1]);
    printf("%s\n",st);
    LogString(20,st);
    free(p);
    free(bs);
    return 0;


}      /******  end of zpor.c    ******/
