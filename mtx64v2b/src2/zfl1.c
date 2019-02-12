/*
      zfl1.c     meataxe-64 Flatten bs/remnant to matrix
      ======     R. A. Parker 13.09.18
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
    uint64_t hdr[5];
    uint64_t i,js,ju,k;
    EFIL *e1, *e2;
    uint64_t siz,nor,nos,nou,noc,fdef;
    uint64_t * bs;
    DSPACE ds1,ds2;
    FIELD * f;
    Dfmt *v1,*v2;
    FELT min1,fel;

    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zfl <bs> <remnant> <output matrix>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);    // bitstring matrix

    siz=8*((hdr[2]+63)/64 + 2);
    bs=malloc(siz);
    ERData(e1,siz,(uint8_t *) bs);
    ERClose(e1);
    noc=bs[0];
    nos=bs[1];
    nor=nos;
    nou=noc-nos;
printf("nor %lu    nos %lu    nou %lu\n",nor,nos,nou);
    e1 = ERHdr(argv[2],hdr);    // remnant matrix
    fdef=hdr[1];
    if ( (hdr[2]!=nor) || (hdr[3]!=nou) )
    {
        LogString(80,"Matrices Incompatible in zfl");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,nou,&ds1);
    DSSet(f,noc,&ds2);
    hdr[3]=noc;
    e2 = EWHdr(argv[3],hdr);    // remnant matrix
    v1=malloc(ds1.nob);   // Input matrix row
    v2=malloc(ds2.nob);   // Output matrix row
    min1=f->charc-1;
    for(i=0;i<nor;i++)
    {
        memset(v2,0,ds2.nob);
        ERData(e1,ds1.nob,v1);
        ju=0;
        js=0;
        for(k=0;k<noc;k++)
        {
            if(BSBitRead(bs,k)==1)
            {
                if(i==js) DPak(&ds2,k,v2,min1);
                js++;
            }
            else
            {
                fel=DUnpak(&ds1,ju,v1);
                DPak(&ds2,k,v2,fel);
                ju++;
            }
        }
        EWData(e2,ds2.nob,v2);
    }
    ERClose(e1);
    EWClose(e2);
    free(v1);
    free(v2);
    free(f);
    return 0;                  // job done
}

/*  end of zfl1.c    */
