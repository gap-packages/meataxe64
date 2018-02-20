/*
      ztr1.c     meataxe-64 matrix transpose simple version
      ======     R. A. Parker     21.05.2016
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
    FIELD * f;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;    // as for input matrix
    DSPACE ds1,ds2;
    Dfmt *m1,*v2,*pt;
    uint64_t i,j;
    FELT fel;
    LogCmd(argc,argv);
	/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage ztr m1 ans");
        exit(21);
    }

    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    hdr[2]=noc;
    hdr[3]=nor;

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    e2 = EWHdr(argv[2],hdr);
    DSSet(f,noc,&ds1);
    DSSet(f,nor,&ds2);
 
/******  check that a row fits in memory  */
    m1=malloc(ds1.nob*nor);
    v2=malloc(ds2.nob);
    if( (m1==NULL) || (v2==NULL) )
    {
        LogString(81,"Can't malloc the space for the vectors");
        exit(23);
    }

/******  read in the matrix  */
    ERData(e1,ds1.nob*nor,m1);
    for(i=0;i<noc;i++)   // for each row of the output
    {
        memset(v2,0,ds2.nob);
        for(j=0;j<nor;j++)   // for each column of the output
        {
            pt=m1+j*ds1.nob;
            fel=DUnpak(&ds1,i,pt);
            DPak(&ds2,j,v2,fel);
        }
        EWData(e2,ds2.nob,v2);
    }

    ERClose(e1);
    EWClose(e2);
    free(m1);
    free(v2);
    free(f);
    return 0;
}

/******  end of ztr1.c    ******/
