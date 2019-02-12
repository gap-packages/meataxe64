/*
      ztest.c     meataxe-64 program to hack to test stuff
      =======     R. A. Parker    13.2.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "pcrit.h"
#include "bitstring.h"
#define SIZ 5000
#define LOOPS 40
#define FDEF 16974593
int main(int argc,  char **argv)
{
    int i,j,k,lp,x;
    Dfmt * m;
    uint8_t * r;
    DSPACE ds;
    FIELD * f;
    m=malloc(10000);
    r=malloc(SIZ);
    f = malloc(FIELDLEN);
    FieldASet1(FDEF,f,NOMUL);
    DSSet(f,600,&ds);
    for(i=0;i<SIZ;i++)
        r[i]=rand();
    for(i=0;i<10000;i++)
        m[i]=47;
    k=0;
    for(lp=0;lp<LOOPS;lp++)
    {
        for(i=0;i<SIZ;i++)
        {
            x=r[i];
            for(j=0;j<SIZ;j++)
                k+=FieldAdd(f,1,1);
//                DPak(&ds,r[j]+x,m,1);
//                k+=DUnpak(&ds,r[j]+x,m);
//          k+=r[j]+x;
        }
    }
    printf("%d %d %d\n",f->addtyp,k,x);
    return 0;
}

#ifdef NEVER
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3;
    FIELD * f;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor,noc;
    DSPACE ds;
    Dfmt *v1,*v2;
    uint64_t i;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zad m1 m2 ans");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr1);
    e2=ERHdr(argv[2],hdr2);

    if( (hdr1[1]!=hdr2[1]) || (hdr1[2]!=hdr2[2]) || (hdr1[3]!=hdr2[3]) )
    {
        LogString(80,"Matrices incompatible");
        exit(22);
    }
    fdef=hdr1[1];
    nor=hdr1[2];
    noc=hdr1[3];

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet1(fdef,f,NOMUL);
    e3 = EWHdr(argv[3],hdr1);
    DSSet(f,noc,&ds);

    v1=malloc(ds.nob);
    v2=malloc(ds.nob);

/******  Do them one row at a time  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
	ERData(e2,ds.nob,v2);
        DAdd(&ds,1,v1,v2,v2);
        EWData(e3,ds.nob,v2);
    }
    free(v1);
    free(v2);
    free(f);
    ERClose(e1);
    ERClose(e2);
    EWClose(e3);
    return 0;
}

#endif

/******  end of ztest.c    ******/
