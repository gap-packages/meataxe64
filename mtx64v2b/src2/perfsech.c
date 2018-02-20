/*
      perfsech.c program to test the performance of
                 the "slab" echelonization
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
    EFIL *ef;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    int i,count;
    FIELD *f;
    DSPACE ds;
    Dfmt *a,*a1,*m,*c,*r;
    uint64_t *rs,*cs;
    FELT det;

    if (argc != 3)
    {
        LogString(80,"usage perfsech m1 count");
        exit(21);
    }
    count=atoi(argv[2]);
    ef = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc,&ds);
    a=malloc(ds.nob*nor);
    a1=malloc(ds.nob*nor);
    m=malloc(SLSizeM(f,nor,noc));
    r=malloc(SLSizeR(f,nor,noc));
    c=malloc(SLSizeC(f,nor,noc));
    rs=malloc(16+((nor+63)/64)*8);
    cs=malloc(16+((noc+63)/64)*8);
    ERData(ef,nor*ds.nob,a);
    ERClose(ef);
    for(i=0;i<count;i++)
    {
        memcpy(a1,a,ds.nob*nor);
        SLEch(&ds, a1, rs, cs, &det, m, c,r, nor);
    }
    free(f);
    free(a);
    free(a1);
    free(m);
    free(c);
    free(rs);
    free(cs);
    return 0;
}

/******  end of perfsmul.c    ******/
