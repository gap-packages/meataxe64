/*
      zs2.c     MTX64 V2.0 Symmetric Square
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
    FIELD * f;
    int i1,i2,j1,j2,ix;
    DSPACE ds1,ds2,ds3;
    Dfmt *m1,*v2,*v21,*v22;
    FELT e11,e12,e21,e3;
    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zs2 m1 ans");
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

    nor2=nor1*(nor1+1)/2;
    noc2=noc1*(noc1+1)/2;

    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);

    m1=malloc(ds1.nob*nor1);
    v2=malloc(ds2.nob);
    v21=malloc(ds1.nob);
    v22=malloc(ds1.nob);
    if( (m1==NULL) || (v2==NULL) || (v21==NULL) || (v22==NULL) )
    {
        LogString(81,"Can't malloc the working space");
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

/* Action on quadratic polynomials */

/* a b  ->    a^2  2ab     */
/* c d        ac  ad+bc    */

/* first the a^2  2ab part */

    for(i1=0;i1<nor1;i1++)
    {
        memset(v2,0,ds2.nob);
        ix=0;
/* a^2 */
        for(j1=0;j1<noc1;j1++)
        {
            e11=DUnpak(&ds1,j1,m1+ds1.nob*i1);
            e3=FieldMul(f,e11,e11);
            DPak(&ds2,ix,v2,e3);
            ix++;  
        }
/*  2ab  */
        for(j1=0;j1<(noc1-1);j1++)
        {
            for(j2=j1+1;j2<noc1;j2++)
            {
                e11=DUnpak(&ds1,j1,m1+ds1.nob*i1);
                e12=DUnpak(&ds1,j2,m1+ds1.nob*i1);
                e3=FieldMul(f,e11,e12);
                e3=FieldAdd(f,e3,e3);     // * 2 works even if p=2
                DPak(&ds2,ix,v2,e3);
                ix++;          
            }
        }
        EWData(ef2,ds2.nob,v2);
    }

/* now the ac ad+bc part */
    for(i1=0;i1<(nor1-1);i1++)
    {
        for(i2=i1+1;i2<nor1;i2++)
        {
            memset(v2,0,ds2.nob);
            ix=0;
/* ac */
            for(j1=0;j1<noc1;j1++)
            {
                e11=DUnpak(&ds1,j1,m1+ds1.nob*i1);
                e21=DUnpak(&ds1,j1,m1+ds1.nob*i2);
                e3=FieldMul(f,e11,e21);  
                DPak(&ds2,ix,v2,e3);
                ix++; 
            }
/*  ad+bc  */
            for(j1=0;j1<(noc1-1);j1++)
            {
                e11=DUnpak(&ds1,j1,m1+ds1.nob*i1);
                e12=DUnpak(&ds1,j1,m1+ds1.nob*i2);
                DSSet(f,noc1-j1-1,&ds3);
                memset(v21,0,ds3.nob);
                memset(v22,0,ds3.nob);
                DCut(&ds1,1,j1+1,m1+ds1.nob*i1,&ds3,v21);
                DCut(&ds1,1,j1+1,m1+ds1.nob*i2,&ds3,v22);
                DSMul(&ds3,e11,1,v22);
                DSMad(&ds3,e12,1,v21,v22);
                DPaste(&ds3,v22,1,ix,&ds2,v2);
                ix+=(noc1-j1-1);
            }
            EWData(ef2,ds2.nob,v2);
        }
    }
    EWClose(ef2);
    free(m1);
    free(v2);
    free(v21);
    free(v22);
    free(f);
    return 0;

}      /******  end of zs2.c    ******/
