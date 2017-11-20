/*
      zmu1.c     MTX64 matrix multiplication
                 single-threaded FIELD version
      ======     R. A. Parker   13.6.2014
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    uint64_t fdef,nor1,noc1,nor2,noc2,nor3,noc3;
    uint64_t i,j;
    EFIL *ef1,*ef2,*ef3;
    uint64_t hdr1[5],hdr2[5],hdr3[5];
    DSPACE ds1,ds2;
    Dfmt *m1,*m2,*m3;
    Dfmt *da,*db;
    FIELD * f;
    FELT e;
    LogCmd(argc,argv);
    /******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zmu1 m1 m2 ans");
        exit(21);
    }
    ef1 = ERHdr(argv[1],hdr1);
    ef2 = ERHdr(argv[2],hdr2);
    if(hdr1[1]!=hdr2[1])
    {
        LogString(80,"Matrices have different fields");
        exit(22);
    }
    fdef=hdr1[1];
    nor1=hdr1[2];
    nor2=hdr2[2];
    noc1=hdr1[3];
    noc2=hdr2[3];
    if(noc1!=nor2)
    {
        LogString(80,"Matrices have incompatible shapes");
        exit(23);
    }
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(24);
    }
    FieldASet(fdef,f);
    nor3=nor1;
    noc3=noc2;
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    hdr3[0]=1;
    hdr3[1]=fdef;
    hdr3[2]=nor3;
    hdr3[3]=noc3;
    hdr3[4]=0;
    ef3 = EWHdr(argv[3],hdr3);
 
	    /******  Allocate space for all the matrices  */
    m1=malloc(ds1.nob*nor1);
    m2=malloc(ds2.nob*nor2);
    m3=malloc(ds2.nob);
    if( (m1==NULL) || (m2==NULL) || (m3==NULL) )
    {
        LogString(81,"Can't malloc the matrix space");
        exit(23);
    }

/* first read in matrices 1 and 2  */
    ERData(ef1,nor1*ds1.nob,m1);
    ERData(ef2,nor2*ds2.nob,m2);
    ERClose(ef2);
    ERClose(ef1);
    da=m1;

    for(i=0;i<nor1;i++)
    {
        memset(m3,0,ds2.nob);
        db=(Dfmt *) m2;
        for(j=0;j<noc1;j++)
        {
            e=DUnpak(&ds1,j,da);
            DSMad(&ds2,e,1,db,m3);
            db+=ds2.nob;
        }
        EWData(ef3,ds2.nob,m3);
        da+=ds1.nob;
    }

    EWClose(ef3);
    free(f);
    free(m1);
    free(m2);
    free(m3);
    return 0;

}

/******  end of zmu1.c    ******/
