// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// funs4.c   functions for  Tensor and Pieces of tensor powers

//    Contents

// fTensor
// fExteriorSquare
// fExteriorCube
// fSymmetricSquare

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

// fTensor

void fTensor(const char *mt1, int s1, const char *mt2, int s2,
             const char *mt3, int s3)
{
    EFIL *ef1,*ef2,*ef3;
    FIELD * f;
    FELT e1;
    uint64_t hdr1[5],hdr2[5],hdr3[5];
    uint64_t fdef,nor1,noc1,nor2,noc2,nor3,noc3;
    uint64_t i1,i2,j1,i,j,k,t;
    DSPACE ds1,ds2,ds3;
    Dfmt *v1,*m2,*v3,*v4;
    long mem,wkr,thiswkr;
    uint64_t * p2;
 
    ef1 = ERHdr(mt1,hdr1);
    ef2 = ERHdr(mt2,hdr2);
    if((hdr1[0]==3)&&(hdr2[0]==3))
    {
        nor1=hdr1[2];
        noc1=hdr1[3];
        nor2=hdr2[2];
        noc2=hdr2[3];
        p2=malloc(8*nor2);
        ERData(ef2,8*nor2,(uint8_t *) p2);
        hdr3[0]=3;
        hdr3[1]=1;
        hdr3[2]=nor1*nor2;
        hdr3[3]=noc1*noc2;
        hdr3[4]=0;
        ef3 = EWHdr(mt3,hdr3);
        for(i=0;i<nor1;i++)
        {
            ERData(ef1,8,(uint8_t *) &k);
            for(j=0;j<nor2;j++)
            {
                t=k*noc1+p2[j];
                EWData(ef3,8,(uint8_t *)&t);
            }
        }
        free(p2);
        ERClose(ef1);
        ERClose(ef2);
        EWClose(ef3);
        return;
    }
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
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    nor3=nor1*nor2;
    noc3=noc1*noc2;
    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);
    DSSet(f,noc3,&ds3);
    hdr3[0]=1;
    hdr3[1]=fdef;
    hdr3[2]=nor3;
    hdr3[3]=noc3;
    hdr3[4]=0;
    ef3 = EWHdr(mt3,hdr3);
/*  Strategy 0     zero rows or cols           */
/*  Still need to read the input for checksums!*/
    if( (noc3==0) || (nor3==0) )
    {
        if( (nor1!=0) && (noc1!=0) )
        {
            v1=malloc(ds1.nob);
            for(i1=0;i1<nor1;i1++)
                ERData(ef1,ds1.nob,v1);
            free(v1);
        }
        if( (nor2!=0) && (noc2!=0) )
        {
            v1=malloc(ds2.nob);
            for(i1=0;i1<nor2;i1++)
                ERData(ef2,ds1.nob,v1);
            free(v1);
        }
        ERClose1(ef1,s1);
        ERClose1(ef2,s2);
        EWClose1(ef3,s3);
        free(f);
        return;
    }
/*  Strategy 1 - read in matrix 2, and then - */
/*      wkr rows at a time (ideally wkr=nor2) */
/*      make wkr rows of the output by taking */
/*      that many rows of matrix 2, scaling   */
/*      and then pasting into the answer      */
    mem=f->megabytes;
    wkr=1000000;
    mem=mem*wkr;
    mem-=nor2*ds2.nob;
    mem-=ds1.nob;
    wkr=mem/(ds2.nob+ds3.nob);
    if(wkr>nor2)wkr=nor2;
    if(wkr>0)                // strategy 1 it is
    {
        v1=malloc(ds1.nob);
        m2=malloc(ds2.nob*nor2);
        v3=malloc(ds3.nob*wkr);
        v4=malloc(ds2.nob*wkr);
        ERData(ef2,nor2*ds2.nob,m2);   // Read in matrix 2
/* for each row of matrix 1 */
        for(i1=0;i1<nor1;i1++)
        {
            ERData(ef1,ds1.nob,v1);
            i2=0;
/* for each wkr-load of matrix 2  */
            while(i2<nor2)
            {
                thiswkr=nor2-i2;
                if(thiswkr>wkr) thiswkr=wkr;
                memset(v3,0,ds3.nob*thiswkr);
/* for each column of matrix 1 */
                for(j1=0;j1<noc1;j1++)
                {
                    e1=DUnpak(&ds1,j1,v1);
                    DCpy(&ds2,m2+ds2.nob*i2,thiswkr,v4);
                    DSMul(&ds2,e1,thiswkr,v4);
                    DPaste(&ds2,v4,thiswkr,j1*noc2,&ds3,v3);
                }
                EWData(ef3,ds3.nob*thiswkr,v3);
                i2+=thiswkr;
            }
        }
        ERClose1(ef1,s1);
        ERClose1(ef2,s2);
        EWClose1(ef3,s3);
        free(v1);
        free(m2);
        free(v3);
        free(v4);
        free(f);
        return;
    }
/*  Strategy 2 - read through matrix 2 once   */
/*      for each row of matrix 1.  wkr rows   */
/*      of matrix 2 and the result are        */
/*      dealt with at once using copy, scale  */
/*      and paste                             */
    mem=f->megabytes;
    wkr=1000000;
    mem=mem*wkr;
    mem-=ds1.nob;
    wkr=mem/(2*ds2.nob+ds3.nob);
    if(wkr>nor2) wkr=nor2;    // not sure this can happen
    if(wkr>0)   // is strategy 2 viable?
    {
        v1=malloc(ds1.nob);
        m2=malloc(ds2.nob*wkr);
        v3=malloc(ds3.nob*wkr);
        v4=malloc(ds2.nob*wkr);

/* for each row of matrix 1 */
        for(i1=0;i1<nor1;i1++)
        {
            ERData(ef1,ds1.nob,v1);
            i2=0;
/* for each wkr-load of matrix 2  */
            while(i2<nor2)
            {
                thiswkr=nor2-i2;
                if(thiswkr>wkr) thiswkr=wkr;
                ERData(ef2,thiswkr*ds2.nob,m2);   // Read in some matrix 2
                memset(v3,0,ds3.nob*thiswkr);
/* for each column of matrix 1 */
                for(j1=0;j1<noc1;j1++)
                {
                    e1=DUnpak(&ds1,j1,v1);
                    DCpy(&ds2,m2,thiswkr,v4);
                    DSMul(&ds2,e1,thiswkr,v4);
                    DPaste(&ds2,v4,thiswkr,j1*noc2,&ds3,v3);
                }
                EWData(ef3,ds3.nob*thiswkr,v3);
                i2+=thiswkr;
            }
            if((i1+1)<nor1)
            {
                ERClose1(ef2,1);
                ef2 = ERHdr(mt2,hdr2);
            }
        }
        ERClose1(ef1,s1);
        ERClose1(ef2,s2);
        EWClose1(ef3,s3);
        free(v1);
        free(m2);
        free(v3);
        free(v4);
        free(f);
        return;
    }
    printf("No strategy available for tensor product\n");
    exit(17);
}

// fExteriorSquare

void fExteriorSquare(const char *mt1, int s1, const char *mt2, int s2)
{
    EFIL *ef1,*ef2;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor1,noc1,nor2,noc2;
    uint64_t p,q,i,col;
    FELT e11,e12;
    FIELD * f;
    DSPACE ds1,ds2,vs;
    Dfmt *m1,*m2,*cb,*v;

    ef1 = ERHdr(mt1,hdr1);
    fdef=hdr1[1];
    nor1=hdr1[2];
    noc1=hdr1[3];
    f = malloc(FIELDLEN);
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
    ef2 = EWHdr(mt2,hdr2);
    m1=malloc(ds1.nob*nor1);   // input matrix
    m2=malloc(ds2.nob*(nor1-1));  // output matrix
    cb=malloc(ds1.nob*nor1);   // clip-board
    v=malloc(ds1.nob);         // vector from m1

    ERData(ef1,nor1*ds1.nob,m1);  // read input matrix
    ERClose1(ef1,s1);

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
    EWClose1(ef2,s2);
    free(m1);
    free(m2);
    free(v);
    free(cb);
    free(f);
}

// fExteriorCube

void fExteriorCube(const char *mt1, int s1, const char *mt2, int s2)
{
    EFIL *ef1,*ef2;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor1,noc1,nor2,noc2;
    FIELD * f;
    int i1,i2,i3,j1,j2,ix;
    DSPACE ds1,ds2,ds3;
    Dfmt *m1,*v2,*v31,*v32,*v33;
    FELT e11,e12,e13,e21,e22,e23;
    FELT w1,w2,d12,d13,d23;

    ef1 = ERHdr(mt1,hdr1);

    fdef=hdr1[1];
    nor1=hdr1[2];
    noc1=hdr1[3];

    f = malloc(FIELDLEN);
    FieldASet(fdef,f);

    nor2=((nor1*(nor1-1)/2)*(nor1-2))/3;
    noc2=((noc1*(noc1-1)/2)*(noc1-2))/3;

    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);

    m1=malloc(ds1.nob*nor2);
    v2=malloc(ds2.nob);
    v31=malloc(ds1.nob);
    v32=malloc(ds1.nob);
    v33=malloc(ds1.nob);

/* first read in matrix 1  */

    ERData(ef1,nor1*ds1.nob,m1);
    ERClose1(ef1,s1);

/* start producing the output file  */

    hdr2[0]=1;
    hdr2[1]=fdef;
    hdr2[2]=nor2;
    hdr2[3]=noc2;
    hdr2[4]=0;
    ef2 = EWHdr(mt2,hdr2);

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
                        d13=FieldSub(f,w2,w1);   // negated
                        w1=FieldMul(f,e12,e23);
                        w2=FieldMul(f,e22,e13);
                        d23=FieldSub(f,w1,w2);
                        DSSet(f,noc1-j2-1,&ds3);
                        memset(v31,0,ds3.nob);
                        memset(v32,0,ds3.nob);
                        memset(v33,0,ds3.nob);
                        DCut(&ds1,1,j2+1,m1+ds1.nob*i1,&ds3,v31);
                        DCut(&ds1,1,j2+1,m1+ds1.nob*i2,&ds3,v32);
                        DCut(&ds1,1,j2+1,m1+ds1.nob*i3,&ds3,v33);
                        DSMul(&ds3,d12,1,v33);
                        DSMad(&ds3,d23,1,v31,v33);
                        DSMad(&ds3,d13,1,v32,v33);
                        DPaste(&ds3,v33,1,ix,&ds2,v2);
                        ix+=(noc1-j2-1);
                    }
                }
                EWData(ef2,ds2.nob,v2);
            }
        }
    }
    EWClose1(ef2,s2);
    free(m1);
    free(v2);
    free(v31);
    free(v32);
    free(v33);
    free(f);
}

// fSymmetricSquare

void fSymmetricSquare(const char *mt1, int s1, const char *mt2, int s2)
{
    EFIL *ef1,*ef2;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor1,noc1,nor2,noc2;
    FIELD * f;
    uint64_t i1,i2,j1,j2,ix;
    DSPACE ds1,ds2,ds3;
    Dfmt *m1,*v2,*v21,*v22;
    uint64_t *mp;
    FELT e11,e12,e21,e3;
    ef1 = ERHdr(mt1,hdr1);

    nor1=hdr1[2];
    noc1=hdr1[3];
    nor2=nor1*(nor1+1)/2;
    noc2=noc1*(noc1+1)/2;

/* Action on quadratic polynomials */

/* a b  ->    a^2  2ab     */
/* c d        ac  ad+bc    */

    hdr2[2]=nor2;
    hdr2[3]=noc2;
    hdr2[4]=0;

    if(hdr1[0]==3)   //symmetric square of permutations actually works
    {
        hdr2[0]=3;
        hdr2[1]=1;
        mp=(uint64_t *) malloc(nor1*sizeof(uint64_t));
        ERData(ef1,nor1*sizeof(uint64_t),(uint8_t *)mp);
        ERClose1(ef1,s1);
        ef2 = EWHdr(mt2,hdr2);

/* first the a^2  2ab part */
        EWData(ef2,nor1*sizeof(uint64_t),(uint8_t *)mp);

/* now the ac ad+bc part */
        for(i1=1;i1<nor1;i1++)
        {

            for(i2=i1;i2<nor1;i2++)
            {
                j1=mp[i1-1];
                j2=mp[i2];
                if(j1>j2) { ix=j1; j1=j2;  j2=ix; }
                ix=noc1 + j1*noc1-((j1+2)*(j1+1)/2)+j2;
                EWData(ef2,8,(uint8_t *)&ix);
            }
        }
        EWClose1(ef2,s2);
        free(mp);
        return;
    }

    fdef=hdr1[1];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);

    hdr2[0]=1;
    hdr2[1]=fdef;

    DSSet(f,noc1,&ds1);
    DSSet(f,noc2,&ds2);

    m1=malloc(ds1.nob*nor1);
    v2=malloc(ds2.nob);
    v21=malloc(ds1.nob);
    v22=malloc(ds1.nob);

/* first read in matrix 1  */

    ERData(ef1,nor1*ds1.nob,m1);
    ERClose1(ef1,s1);

/* start producing the output file  */

    ef2 = EWHdr(mt2,hdr2);

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
        for(j1=1;j1<noc1;j1++)
        {
            for(j2=j1;j2<noc1;j2++)
            {
                e11=DUnpak(&ds1,j1-1,m1+ds1.nob*i1);
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
    for(i1=1;i1<nor1;i1++)
    {
        for(i2=i1;i2<nor1;i2++)
        {
            memset(v2,0,ds2.nob);
            ix=0;
/* ac */
            for(j1=0;j1<noc1;j1++)
            {
                e11=DUnpak(&ds1,j1,m1+ds1.nob*(i1-1));
                e21=DUnpak(&ds1,j1,m1+ds1.nob*i2);
                e3=FieldMul(f,e11,e21);  
                DPak(&ds2,ix,v2,e3);
                ix++; 
            }
/*  ad+bc  */
            for(j1=1;j1<noc1;j1++)
            {
                e11=DUnpak(&ds1,j1-1,m1+ds1.nob*(i1-1));
                e12=DUnpak(&ds1,j1-1,m1+ds1.nob*i2);
                DSSet(f,noc1-j1,&ds3);
                memset(v21,0,ds3.nob);
                memset(v22,0,ds3.nob);
                DCut(&ds1,1,j1,m1+ds1.nob*(i1-1),&ds3,v21);
                DCut(&ds1,1,j1,m1+ds1.nob*i2,&ds3,v22);
                DSMul(&ds3,e11,1,v22);
                DSMad(&ds3,e12,1,v21,v22);
                DPaste(&ds3,v22,1,ix,&ds2,v2);
                ix+=(noc1-j1);
            }
            EWData(ef2,ds2.nob,v2);
        }
    }
    EWClose1(ef2,s2);
    free(m1);
    free(v2);
    free(v21);
    free(v22);
    free(f);
    return;
}

/* end of funs4.c  */
