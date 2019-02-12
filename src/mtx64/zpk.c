/*
      zpk.c     meataxe-64 Peek at Matrix
      =====     R. A. Parker   26.1.2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "slab.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    FIELD * f;
    DSPACE ds;

    if(argc!=2)
    {
        printf("usage zpk <filename>\n");
        exit(1);
    }
    printf("%s",argv[1]);
    EPeek(argv[1],hdr);

    if(hdr[0]==1)
    {
        fdef=hdr[1];
        nor=hdr[2];
        noc=hdr[3];
        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        DSSet(f,noc,&ds);
        printf(" = matrix %ld rows, %ld cols (%ld bytes)",
            nor,noc,ds.nob);
        printf(" field %ld, prime %ld\n",fdef,f->charc);
    }
    if(hdr[0]==2)
    {
printf(" = bitstring, %ld bits, %ld set\n",
                       hdr[2],    hdr[3]);
    }
    if(hdr[0]==3)
    {
printf(" = map/permutation, domain %ld, image %ld\n",
                                  hdr[2],     hdr[3]);
    }
    if(hdr[0]==4)
    {
printf(" = polynomial degree %ld with %ld factor(s) field %ld\n",
                           hdr[2],   hdr[3],           hdr[1]);
    }
    return 0;
}

/******  end of zpk.c    ******/
