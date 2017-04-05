/*
      zfr.c     meataxe-64 V2.0 Frobenius automorphism
      =====     R. A. Parker    29.12.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "pcrit.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    FELT f1,f2,f3;
    FELT sig[63][63];
    uint64 hdr[5];
    uint64 fdef,nor,noc;
    FIELD * f;
    DSPACE ds,dp;
    Dfmt *v1,*v2,*vpx,*vpa;
    uint64 i,j,k,z;
    uint64_t i1,i2,i3,i4,j1,j2,j3,j4;
    int mode;
    uint16_t *log16, *alog16;
    uint8_t lut[256];

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zfr m1 m2");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];

    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(22);
    }
    FieldASet(fdef,f);
    e2 = EWHdr(argv[2],hdr);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);


    log16=NULL;        // avoid compiler warnings
    alog16=NULL;
    vpa=NULL;
    vpx=NULL;

/* decide on strategy */

/* 4 default use PExtract and PAssemble */
    mode=4;
/* 1 if pow == 1, just copy  */
    if(f->pow==1) mode=1;
/* 3 if logs available, use them */
    if( (mode==4) && (f->fdef<=65536) )
    {
        log16=(uint16_t *)((uint8_t *)f + f->Tlog16);
        alog16=(uint16_t *)((uint8_t *)f + f->Talog16);
        mode=3;
    }
/* 2 if field fits in a byte and not prime field, use lookup */
/* notice we use the log tables from previous case */
    if( (mode==3) &&(f->fdef<=256) )
    {
        for(i=0;i<256;i++) lut[i]=0;
        if(f->fdef==4)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=alog16[(f->charc*log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=alog16[(f->charc*log16[i2])%(f->fdef-1)];
                for(i3=0;i3<f->fdef;i3++)
                {
                  if(i3==0) j3=0; 
                  else j3=alog16[(f->charc*log16[i3])%(f->fdef-1)];
                  for(i4=0;i4<f->fdef;i4++)
                  {
                    if(i4==0) j4=0; 
                    else j4=alog16[(f->charc*log16[i4])%(f->fdef-1)];
                    lut[64*i1+16*i2+4*i3+i4]=64*j1+16*j2+4*j3+j4;
                  }
                }
              }
            }
        }
        if(f->fdef==8)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=alog16[(f->charc*log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=alog16[(f->charc*log16[i2])%(f->fdef-1)];
                lut[8*i1+i2]=8*j1+j2;
              }
            }
        }
        if(f->fdef==9)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=alog16[(f->charc*log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=alog16[(f->charc*log16[i2])%(f->fdef-1)];
                lut[9*i1+i2]=9*j1+j2;
              }
            }
        }
        if(f->fdef==16)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=alog16[(f->charc*log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=alog16[(f->charc*log16[i2])%(f->fdef-1)];
                lut[16*i1+i2]=16*j1+j2;
              }
            }
        }
        if(f->fdef>16)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
                if(i1==0) j1=0; 
                else j1=alog16[(f->charc*log16[i1])%(f->fdef-1)];
                lut[i1]=j1;
            }
        }
        mode=2;
    }
/*      use extract and assemble  */
    if(mode==4)
    {
        f1=f->charc;    // f1 = X
        f2=1;
// compute f2 = X^p in big field
        z=f->charc;
        /* invariant want f2*(f1^z) */
        while(z!=0)
        {
            if((z%2)==1) f2=FieldMul(f,f2,f1);
            f1=FieldMul(f,f1,f1);
            z=z/2;
        }
        f1=1;
        for(i=0;i<f->pow;i++)
        {
            f3=f1;
            for(j=0;j<f->pow;j++)
            {
                sig[i][j]=f3%f->charc;
                f3=f3/f->charc;
            }
            f1=FieldMul(f,f2,f1);
        }
        PSSet(f,noc,&dp);
        vpx=malloc(dp.nob*f->pow);
        vpa=malloc(dp.nob*f->pow); 
    }

/******  For each row of the matrix  */
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v1);
        if(mode==1)
        {
            EWData(e2,ds.nob,v1);
            continue;
        }
        if(mode==2)
        {
            pcunf(v1,ds.nob,lut);
            EWData(e2,ds.nob,v1);
            continue;
        }
        if(mode==4)
        {
            PExtract(&ds,v1,vpx,1,dp.nob);
            memset(vpa,0,f->pow*dp.nob);
            for(j=0;j<f->pow;j++)
            {
                for(k=0;k<f->pow;k++)
                {
                    DSMad(&dp,sig[j][k],1,vpx+j*dp.nob,vpa+k*dp.nob);
                }
            }
            PAssemble(&ds,vpa,v2,1,dp.nob);
            EWData(e2,ds.nob,v2);
            continue;
        }
        memset(v2,0,ds.nob);
/****** for each column of that row */
        for(j=0;j<noc;j++)
        {
            f1=DUnpak(&ds,j,v1);

/* compute f2 = Frobenius(f1)  */
            switch (mode)
            {
              case 3:
                f2=alog16[(f->charc*log16[f1])%(f->fdef-1)];
                break;
              default:          // mode 5 - no longer used
                printf("Internal error in Frobenius program\n");
                exit(13);
            }
            DPak(&ds,j,v2,f2);
        }
        EWData(e2,ds.nob,v2);
    }
    free(v1);
    free(v2);
    free(f);
    if(mode==4)
    {
        free(vpx);
        free(vpa);
    }
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zfr.c    ******/
