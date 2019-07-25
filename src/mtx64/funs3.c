// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// funs3.c     Some field changing functions

//    Contents
// fFrobenius
// fFieldContract
// fFieldExtend

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tuning.h"
#include "field.h"
#include "pcrit.h"
#include "funs.h"
#include "io.h"
#include "bitstring.h"

// fFrobenius

void fFrobenius(const char *m1, int s1, const char *m2, int s2)
{
    EFIL *e1,*e2;
    FELT f1,f2,f3;
    FELT sig[63][63];
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    FIELD * f;
    DSPACE ds,dp;
    Dfmt *v1,*v2,*vpx,*vpa;
    uint64_t i,j,k,z;
    uint64_t i1,i2,i3,i4,j1,j2,j3,j4;
    int mode;
    uint8_t lut[256];

    e1=ERHdr(m1,hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];

    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    e2 = EWHdr(m2,hdr);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);

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
              else j1=f->alog16[(f->charc*f->log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=f->alog16[(f->charc*f->log16[i2])%(f->fdef-1)];
                for(i3=0;i3<f->fdef;i3++)
                {
                  if(i3==0) j3=0; 
                  else j3=f->alog16[(f->charc*f->log16[i3])%(f->fdef-1)];
                  for(i4=0;i4<f->fdef;i4++)
                  {
                    if(i4==0) j4=0; 
                    else j4=f->alog16[(f->charc*f->log16[i4])%(f->fdef-1)];
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
              else j1=f->alog16[(f->charc*f->log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=f->alog16[(f->charc*f->log16[i2])%(f->fdef-1)];
                lut[8*i1+i2]=8*j1+j2;
              }
            }
        }
        if(f->fdef==9)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=f->alog16[(f->charc*f->log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=f->alog16[(f->charc*f->log16[i2])%(f->fdef-1)];
                lut[9*i1+i2]=9*j1+j2;
              }
            }
        }
        if(f->fdef==16)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
              if(i1==0) j1=0; 
              else j1=f->alog16[(f->charc*f->log16[i1])%(f->fdef-1)];
              for(i2=0;i2<f->fdef;i2++)
              {
                if(i2==0) j2=0; 
                else j2=f->alog16[(f->charc*f->log16[i2])%(f->fdef-1)];
                lut[16*i1+i2]=16*j1+j2;
              }
            }
        }
        if(f->fdef>16)
        {
            for(i1=0;i1<f->fdef;i1++)
            {
                if(i1==0) j1=0; 
                else j1=f->alog16[(f->charc*f->log16[i1])%(f->fdef-1)];
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
                f2=f->alog16[(f->charc*f->log16[f1])%(f->fdef-1)];
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
    ERClose1(e1,s1);
    EWClose1(e2,s2);
}

extern int  fFieldContract(const char *m1, int s1, uint64_t fdef2,
                           const char *m2, int s2)
{
    EFIL *e1,*e2;
    FIELD *f1,*f2;
    DSPACE ds1,ds2;
    Dfmt *v1,*v2;
    uint64_t fdef1,nor,noc,i,j;
    uint64_t ratio,good,mode;
    uint64_t hdr[5];
    FELT x1,x2;

    e1=ERHdr(m1,hdr);
    fdef1=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f1 = malloc(FIELDLEN);
    f2 = malloc(FIELDLEN);
    FieldASet(fdef1,f1);
    FieldASet(fdef2,f2);
    if( (f1->charc!=f2->charc) || ((f1->pow%f2->pow)!=0) ) 
    {
        LogString(80,"Field contraction not possible");
        exit(23);
    }
    hdr[1]=fdef2;
    e2 = EWHdr(m2,hdr);
    DSSet(f1,noc,&ds1);
    DSSet(f2,noc,&ds2);
    v1=malloc(ds1.nob);
    v2=malloc(ds2.nob);
    ratio=(f1->fdef-1)/(f2->fdef-1);

/* decide on strategy */

/* 4.  Give up!  */
    mode=4;              // default - no way.

/* 1 if fields are identical, just copy */
    if(f1->fdef==f2->fdef) mode=1;

/* 2 if moving to the ground field, use natural embedding */
    if((mode==4) && (f2->pow==1) ) mode=2;

/* 3 if log16 and alog16 available, use them */
    if((mode==4) && (f1->fdef <= 65536) )
    {
        mode=3;
    }

//  There should be a mode 5 that uses linear forms, but I haven't
//  written that (yet).  Hence if both fields are extension fields,
//  and the larger is > 65536, this function fails at the moment

    if (mode == 4)
    {
        LogString(80,"This program version cannot handle this case");
        exit(21);
    }

    good=1;    // it's going OK so far

    for(i=0;i<nor;i++)
    {
	ERData(e1,ds1.nob,v1);
        if(mode==1)    // fields identical
        {
            EWData(e2,ds2.nob,v1);
            continue;
        }
        memset(v2,0,ds2.nob);
        for(j=0;j<noc;j++)
        {
            x1=DUnpak(&ds1,j,v1);
            if(mode==2)
            {
                if(x1>=f2->fdef) good=0;
                else x2=x1;  // from ground field
            }
            if(mode==3)
            {
                if(x1==0) x2=0;
                else
                {
                    x2=f2->alog16[f1->log16[x1]/ratio];  // using logs
                    if( (f1->log16[x1]%ratio)!=0 ) good=0;
                }
            }
            if(good==0) break;
            DPak(&ds2,j,v2,x2);
        }
        if(good==0) break;
        EWData(e2,ds2.nob,v2);
    }

    free(v1);
    free(v2);
    free(f1);
    free(f2);
    ERClose1(e1,s1);
    EWClose1(e2,s2);
    if (good == 0)
    {
        remove(m2);
        return 1;
    }
    return 0;
}

extern void fFieldExtend(const char *m1, int s1, uint64_t fdef2,
                         const char *m2, int s2)
{
    uint64_t fdef1,nor,noc;
    uint64_t hdr[5];
    uint64_t i,j;
    uint64_t siz;
    EFIL *e1,*e2;
    FIELD *f1,*f2;
    DSPACE ds1,ds2;
    Dfmt *v1,*v2;
    FELT x1,x2,y1,y2;
    int mode;
    uint64_t z;
    FELT * tab;

    e1=ERHdr(m1,hdr);
    fdef1=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f1 = malloc(FIELDLEN);
    f2 = malloc(FIELDLEN);
    FieldASet(fdef1,f1);
    FieldASet(fdef2,f2);
    if( (f1->charc!=f2->charc) || ((f2->pow%f1->pow)!=0) ) 
    {
        LogString(80,"Field extension not possible");
        exit(23);
    }

    hdr[1]=fdef2;
    e2 = EWHdr(m2,hdr);
    DSSet(f1,noc,&ds1);
    DSSet(f2,noc,&ds2);
    v1=malloc(ds1.nob);
    v2=malloc(ds2.nob);
    tab=NULL;               // avoid compiler warnings

/* decide on strategy */

/* 3 default at the moment is to use a look-up table */

    mode=3;

/* 1 if fields are identical, just copy */

    if(f1->fdef==f2->fdef) mode=1;

/* 2 if moving from ground field, use natural embedding */
    if((mode==3) && (f1->pow==1) ) mode=2;

/* There should also be a linear forms method.  May   */
/* need a new interface to the linf module.  Basic    */
/* idea is to extract in small field, do linear forms */
/* up to the large field, and to reassemble there     */

/* else allocate look-up table (malloc might fail!)  */
    if(mode==3)
    {
        siz=f1->fdef*sizeof(FELT);
        tab=NULL;
        if( (siz/1000000)<f1->megabytes )
            tab=malloc(siz);
        if(tab==NULL)
        {
            printf("fFieldExtend cannot handle this case\n");
            exit(17);
        }
        z=(f2->fdef-1)/(f1->fdef-1);
/* compute y2 = (gen)^z */
        x2=f2->charc;
        y2=1;
/*  answer is y2^z * x2  */
        while(z!=0)
        {
            if( (z%2)==1) y2=FieldMul(f2,y2,x2);
            x2=FieldMul(f2,x2,x2);
            z=z/2;
        }
        y1=f1->charc;
        x1=1;
        x2=1;
/* so x1,y1 (in field1) equals x2,y2 (in field2) */
        for(i=0;i<f1->fdef;i++)
        {
            tab[x1]=x2;
            x1=FieldMul(f1,x1,y1);
            x2=FieldMul(f2,x2,y2);
        }
        tab[0]=0;
    }
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds1.nob,v1);
        if(mode==1)    // fields identical
        {
            EWData(e2,ds2.nob,v1);
            continue;
        }
        memset(v2,0,ds2.nob);
        for(j=0;j<noc;j++)
        {
            x1=DUnpak(&ds1,j,v1);
            if(mode==2) x2=x1;  // from ground field
               else     x2=tab[x1];  // from lookup table
            DPak(&ds2,j,v2,x2);
        }
        EWData(e2,ds2.nob,v2);
    }
    ERClose1(e1,s1);
    EWClose1(e2,s2);
    free(v1);
    free(v2);
    free(f1);
    free(f2);
    if(mode==3) free(tab);
    return;
}

/* end of funs3.c  */
