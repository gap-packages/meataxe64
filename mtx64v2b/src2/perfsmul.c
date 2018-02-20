/*
      perfsmul.c program to test the performance of
                 the "slab" matrix multiplication
      ======     R. A. Parker   19.2.2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "slab.h"
 
int main(int argc,  char **argv)
{
    EFIL *ef1,*ef2;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor1,noc1,nor2,noc2;
    int i,count;
    FIELD *f;
    DSPACE ds1,ds2;
    Dfmt *m1,*m2,*m3;

    if (argc != 4)
    {
        LogString(80,"usage perfsmul m1 m2 count");
        exit(21);
    }
    count=atoi(argv[3]);
    ef1 = ERHdr(argv[1],hdr1);
    ef2 = ERHdr(argv[2],hdr2);
    fdef=hdr1[1];
    nor1=hdr1[2];
    nor2=hdr2[2];
    noc1=hdr1[3];
    noc2=hdr2[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    m1=malloc(ds1.nob*nor1);
    m2=malloc(ds2.nob*nor2);
    m3=malloc(ds2.nob*nor1);
    ERData(ef1,nor1*ds1.nob,m1);
    ERData(ef2,nor2*ds2.nob,m2);
    ERClose(ef2);
    ERClose(ef1);
    for(i=0;i<count;i++)
    {
        SLMul(f, m1, m2, m3, nor1, noc1, noc2);
    }
    free(f);
    free(m1);
    free(m2);
    free(m3);
    return 0;
}

/******  end of perfsmul.c    ******/
