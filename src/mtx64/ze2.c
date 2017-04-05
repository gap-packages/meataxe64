/*
      ze2.c     MTX64 V2.0 Faster Exterior Square
      =====     R. A. Parker   29.12.2016 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *ef1,*ef2;
    uint64 hdr1[5],hdr2[5];
    uint64 fdef,nor1,noc1,nor2,noc2;
    uint64 p,q,i,col;
    FELT e11,e12;
    FIELD * f;
    DSPACE ds1,ds2,vs;
    Dfmt *m1,*m2,*cb,*v;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage ze2 m1 ans");
        exit(21);
    }
    ef1 = ERHdr(argv[1],hdr1);
    fdef=hdr1[1];
    nor1=hdr1[2];
    noc1=hdr1[3];
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    nor2=nor1*(nor1-1)/2;
    noc2=noc1*(noc1-1)/2;
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);

    hdr2[0]=1;
    hdr2[1]=fdef;
    hdr2[2]=nor2;
    hdr2[3]=noc2;
    hdr2[4]=0;
    ef2 = EWHdr(argv[2],hdr2);
    m1=malloc(ds1.nob*nor1);   // input matrix
    m2=malloc(ds2.nob*(nor1-1));  // output matrix
    cb=malloc(ds1.nob*nor1);   // clip-board
    v=malloc(ds1.nob);         // vector from m1

    if( (m1==NULL) || (m2==NULL) || (cb==NULL) )
    {
        LogString(81,"Can't malloc the space");
        exit(23);
    }

    ERData(ef1,nor1*ds1.nob,m1);  // read input matrix
    ERClose(ef1);

    for(p=0;p<(nor1-1);p++)    // for each row p of input matrix
    {
        col=0;
        memset(m2,0,ds2.nob*(nor1-1-p));    // zeroize matrix - using DPaste
        for(q=0;q<(noc1-1);q++)   // for each column of input matrix
        {
            DSSet(f,noc1-q-1,&vs);
            DCut(&ds1,1,q+1,m1+p*ds1.nob,&vs,v);
            DCut(&ds1,nor1-1-p,q+1,m1+(p+1)*ds1.nob,&vs,cb);
            e11=DUnpak(&ds1,q,m1+ds1.nob*p);
            DSMul(&vs,e11,nor1-1-p,cb);
            for(i=p+1;i<nor1;i++)
            { 
                e12=DUnpak(&ds1,q,m1+ds1.nob*i);
                e12=FieldNeg(f,e12);
                DSMad(&vs,e12,1,v,cb+(i-p-1)*vs.nob);
            }
            DPaste(&vs,cb,nor1-p-1,col,&ds2,m2);
            col+=(noc1-q-1);
        }
        EWData(ef2,ds2.nob*(nor1-1-p),m2);
    }
    EWClose(ef2);
    free(m1);
    free(m2);
    free(v);
    free(cb);
    free(f);

    return 0;

}      /******  end of ze2.c    ******/
