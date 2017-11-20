/*
      ze3.c     MTX64 Exterior CubeSquare
      =====     R. A. Parker   1.5.2014 
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
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor1,noc1,nor2,noc2;
    FIELD * f;
    int i1,i2,i3,j1,j2,j3,ix;
    DSPACE ds1,ds2;
    Dfmt *m1,*v2;
    FELT e11,e12,e13,e21,e22,e23,e31,e32,e33;
    FELT w1,w2,w3,d12,d13,d23;
    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage ze3 m1 ans");
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

    nor2=((nor1*(nor1-1)/2)*(nor1-2))/3;
    noc2=((noc1*(noc1-1)/2)*(noc1-2))/3;

    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);

         /******  check that a row fits in memory  */
    m1=malloc(ds1.nob*nor2);
    v2=malloc(ds2.nob);
    if( (m1==NULL) || (v2==NULL) )
    {
        LogString(81,"Can't malloc the matrix space");
        exit(23);
    }

/* first read in matrix 1  */

    ERData(ef1,nor1*ds1.nob,m1);
    ERClose(ef1);


/* start producing the output file  */

    hdr2[0]=1;
    hdr2[1]=fdef;
    hdr2[2]=nor2;
    hdr2[3]=noc2;
    hdr2[4]=0;
    ef2 = EWHdr(argv[2],hdr2);

/* for each row of matrix 1 */
    for(i1=0;i1<nor1;i1++)
    {
/* for each later row of matrix 1 */
        for(i2=i1+1;i2<nor1;i2++)
        {
/* for each even later row of matrix 1 */
            for(i3=i2+1;i3<nor1;i3++)
            {
                memset(v2,0,ds2.nob);
/* for each column of matrix 1 */
                ix=0;
                for(j1=0;j1<noc1;j1++)
                {
                    e11=DUnpak(&ds1,j1,m1+ds1.nob*i1);
                    e12=DUnpak(&ds1,j1,m1+ds1.nob*i2);
                    e13=DUnpak(&ds1,j1,m1+ds1.nob*i3);
/* for each subsequent column of matrix 1 */
                    for(j2=j1+1;j2<noc1;j2++)
                    {
                        e21=DUnpak(&ds1,j2,m1+ds1.nob*i1);
                        e22=DUnpak(&ds1,j2,m1+ds1.nob*i2);
                        e23=DUnpak(&ds1,j2,m1+ds1.nob*i3);
                        w1=FieldMul(f,e11,e22);
                        w2=FieldMul(f,e21,e12);
                        d12=FieldSub(f,w1,w2);
                        w1=FieldMul(f,e11,e23);
                        w2=FieldMul(f,e21,e13);
                        d13=FieldSub(f,w1,w2);
                        w1=FieldMul(f,e12,e23);
                        w2=FieldMul(f,e22,e13);
                        d23=FieldSub(f,w1,w2);
/* for each even later column of matrix 1 */
                        for(j3=j2+1;j3<noc1;j3++)
                        {
                            e31=DUnpak(&ds1,j3,m1+ds1.nob*i1);
                            e32=DUnpak(&ds1,j3,m1+ds1.nob*i2);
                            e33=DUnpak(&ds1,j3,m1+ds1.nob*i3);
                            w1=FieldMul(f,d12,e33);
                            w2=FieldMul(f,d23,e31);
                            w3=FieldMul(f,d13,e32);
                            w1=FieldAdd(f,w1,w2);
                            w1=FieldSub(f,w1,w3);
                            DPak(&ds2,ix,v2,w1);
                            ix++;
                        }
                    }
                }
                EWData(ef2,ds2.nob,v2);
            }
        }
    }
    EWClose(ef2);
    free(m1);
    free(v2);
    free(f);
    return 0;

}      /******  end of ze3.c    ******/
