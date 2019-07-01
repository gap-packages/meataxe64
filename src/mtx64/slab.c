/*
         slab.c  -   Slab Routines Code
         ======      R. A. Parker 19.2.2018 (SAL RCEch)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"
#include "slab.h"
#include "linf.h"
#include "pmul.h"
#include "bitstring.h"
#include "dfmtf.h"
#include "gauss.h"

// #define DEBUG 1

int  FieldSet1(uint64_t fdef, FIELD * f, int flags)
{
    int res;
    res=FieldASet1(fdef,f,flags);
    if(res!=1) return res;
    hpmiset(f);
    linftab(f);
    return 1;   
}

void FieldSet (uint64_t fdef, FIELD * f)
{
    int res;
    res=FieldSet1(fdef,f,0);
    (void)res;
}

uint64_t SLSize (const FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    DSSet(f,noc,&ds);
    return ds.nob*nor;
}
uint64_t SLSizeM(const FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    uint64_t rank;
    rank=nor;
    if(rank>noc) rank=noc;
    DSSet(f,rank,&ds);
    return ds.nob*rank;
}

// For brevity, let e = f->entbyte and b = f->bytesper 
// Let F(noc,r)=r*[(noc-r+e)/e*b] as real numbers (no rounding)
// maximum value of F(noc,r) is at r=(noc+e)/2
// For all r, bytes for Remnant <= F(noc,r)
// So anyway bytes <= (r^2+1)/e*b with r=(noc+e)/2
// If nor < (noc+e)/2 then bytes <= F(noc,nor)

uint64_t SLSizeR(const FIELD * f, uint64_t nor, uint64_t noc)
{
    uint64_t r,sp;
    r=(noc+f->entbyte)/2;
    sp=(r*r+1)*f->bytesper/f->entbyte;
    if(nor<r) sp=nor*((noc-nor+2*f->entbyte)*f->bytesper/f->entbyte);
    return sp;
}

// Now let F(nor,r)=(nor-r)*(r+e) so bytes <= F(nor,r)*b/e
// max byte at r=(nor-e)/2 so F = (nor-nor/2+e/2)(nor/2-e/2+e)
// = (nor+e)/2 . (nor+e)/2 so maxbytes = b/e * s^2 with s = (nor+e)/2
// If noc < (nor-e)/2 max is F(nor,noc)*b/e =(nor-noc)(noc+e)*b/e 

uint64_t SLSizeC(const FIELD * f, uint64_t nor, uint64_t noc)
{
    uint64_t s,sp;
    s=(nor+f->entbyte)/2;
    sp=(s*s+1)*f->bytesper/f->entbyte;
    if((noc+f->entbyte)<s) 
        sp=(nor-noc)*(noc+f->entbyte)*f->bytesper/f->entbyte;
    return sp;
}

void BCMul(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c)
{
    const FIELD *f;

    f=dsa->f;
// if the matrices are tiny use DMul
    if( (nora<3) || (dsa->noc<3) || (dsbc->noc<3) )
    {
        DFMul(dsa,dsbc,nora,a,astride,b,bstride,c);
        return;
    }
    if(f->linfscheme!=0)  // linear functions case?
    {
        LLMul(dsa,dsbc,nora,a,astride,b,bstride,c);
        return;
    }
    if(f->pow==1)    // Ground field - just call PLMul
    {
        PLMul(dsa,dsbc,nora,a,astride,b,bstride,c);
        return;
    }
// else do it by steam over extension field
    DFMul(dsa,dsbc,nora,a,astride,b,bstride,c);

}

void BCMad(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, uint64_t cstride)
{
    Dfmt *temp;
    temp=malloc(dsbc->nob*nora);
    BCMul(dsa,dsbc,nora,a,astride,b,bstride,temp);
    TAdd(dsbc,nora,temp,dsbc->nob,c,cstride,c,cstride);
    free(temp);
}

void SWMad(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, uint64_t cstride, int lev);

void SWMul(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, int lev)
{
    memset(c,0,nora*dsbc->nob);
    SWMad(dsa,dsbc,nora,a,astride,b,bstride,c,dsbc->nob,lev);
}

void SWMad(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, uint64_t cstride, int lev)
{
    DSPACE dsha,dshbc;
    const FIELD *f;
    const Dfmt *a11,*a12,*a21,*a22,*b11,*b12,*b21,*b22;
    Dfmt *c11,*c12,*c21,*c22;
    Dfmt *r1,*r2,*r3;
    uint64_t rowshac, rowshb, nobha, nobhbc;
    if(lev==0)
    {
        BCMad(dsa,dsbc,nora,a,astride,b,bstride,c,cstride);
        return;
    }
    f=dsa->f;
    rowshac=nora/2;
    rowshb=dsa->noc/2;
    DSSet(f,dsa->noc/2,&dsha);
    DSSet(f,dsbc->noc/2,&dshbc);
    nobha=dsha.nob;
    nobhbc=dshbc.nob;
    r1=malloc(rowshac*nobha);          // allocate the three work areas
    r2=malloc(rowshb*nobhbc);
    r3=malloc(rowshac*nobhbc);
    a11=a;  b11=b;  c11=c;            // set the 12 pointers
    a21=a+astride*rowshac;
    b21=b+bstride*rowshb;
    c21=c+cstride*rowshac;
    a12=a11+nobha;  a22=a21+nobha;
    b12=b11+nobhbc; b22=b21+nobhbc;
    c12=c11+nobhbc; c22=c21+nobhbc;     

    TAdd(&dsha,rowshac,a21,astride,             // r1=a21+a22
        a22,astride,r1,nobha);
    TSub(&dshbc,rowshb,b12,bstride,             // r2=b12-b11
        b11,bstride,r2,nobhbc);
    SWMul(&dsha,&dshbc,rowshac,r1,nobha,        // r3=r1*r2
        r2,nobhbc,r3,lev-1);
    TAdd(&dshbc,rowshac,c12,cstride,            // c12=c12+r3
        r3,nobhbc,c12,cstride); 
    TAdd(&dshbc,rowshac,c22,cstride,            // c22=c22+r3
        r3,nobhbc,c22,cstride);
    TSub(&dsha,rowshac,r1,nobha,                // r1=r1-a11
        a11,astride,r1,nobha);
    TSub(&dshbc,rowshb,b22,bstride,             // r2=b22-r2
        r2,nobhbc,r2,nobhbc);
    SWMul(&dsha,&dshbc,rowshac,a11,astride,     // r3=a11*b11
        b11,bstride,r3,lev-1);
    TAdd(&dshbc,rowshac,c11,cstride,            // c11=c11+r3
        r3,nobhbc,c11,cstride);
    SWMad(&dsha,&dshbc,rowshac,r1,nobha,        // r3=r3+r1*r2
        r2,nobhbc,r3,nobhbc,lev-1);
    SWMad(&dsha,&dshbc,rowshac,a12,astride,     // c11=c11+a12*b21
        b21,bstride,c11,cstride,lev-1);
    TSub(&dsha,rowshac,a12,astride,             // r1=a12-r1
        r1,nobha,r1,nobha);
    TSub(&dshbc,rowshb,b21,bstride,             // r2=b21-r2
        r2,nobhbc,r2,nobhbc);
    SWMad(&dsha,&dshbc,rowshac,r1,nobha,        // c12=c12+r1*b22
        b22,bstride,c12,cstride,lev-1);
    TAdd(&dshbc,rowshac,c12,cstride,            // c12=c12+r3
        r3,nobhbc,c12,cstride);
    SWMad(&dsha,&dshbc,rowshac,a22,astride,     // c21=c21+a22*r2
        r2,nobhbc,c21,cstride,lev-1);
    TSub(&dsha,rowshac,a11,astride,             // r1=a11-a21
        a21,astride,r1,nobha);
    TSub(&dshbc,rowshb,b22,bstride,             // r2=b22-b12
        b12,bstride,r2,nobhbc);
    SWMad(&dsha,&dshbc,rowshac,r1,nobha,        // r3=r3+r1*r2
        r2,nobhbc,r3,nobhbc,lev-1);
    TAdd(&dshbc,rowshac,c21,cstride,            // c21=c21+r3
        r3,nobhbc,c21,cstride);
    TAdd(&dshbc,rowshac,c22,cstride,            // c22=c22+r3
        r3,nobhbc,c22,cstride);
    free(r1);
    free(r2);
    free(r3);
}

void TSMul(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c)
{
    int lev;           // Strassen recursion level
    uint64_t thresh;
    const FIELD *f;
    uint64_t r1,r2,r3,r4,r5,r6;
   
    f=dsa->f;
// compute lev (Strassen level) and divis (2^lev)
// r1 r2 r3 the three dimensions of the slab
// for power of two and threshold
    r1=nora;
    r2=dsa->noc;
    r3=dsbc->noc;
// r4 r5 the number of bytes per row for power of 2
    r4=dsa->nob;  // fullness of bytes
    r5=dsbc->nob;  // checked later
// r6 the number of cauldrons for power of 2
    if( (f->cauldron!=0) && ((f->pow==1) || (f->linfscheme==1)))
    {
        r6=(dsbc->noc+f->cauldron-1)/f->cauldron;
// allow cauldrons to be increased for Strassen if there are enough
        if( ((r6%2)==1) && (r6>20)  ) r6++;
        if( ((r6%4)==2) && (r6>60)  ) r6+=2;
        if( ((r6%8)==4) && (r6>160) ) r6+=4;
    }
    else         r6=0;
#ifdef DEBUG
printf("r1 %lu   r2 %lu   r3 %lu   r4 %lu   r5 %lu   r6 %lu  ",
        r1,r2,r3,r4,r5,r6);
#endif

    thresh=11111;   // not set yet or default
    if( (f->cauldron!=0) && (f->pow==1) )  // HPMI primes
    {
        if(f->fdef==2) thresh=12000;
        if(f->fdef==3) thresh=15000;
        if(f->fdef==5) thresh=2500;
        if( (f->fdef>=7) && (f->fdef<=13) ) thresh=3000;
        if( (f->fdef>=17) && (f->fdef<=61) ) thresh=1600;
        if( (f->fdef>=67) && (f->fdef<=193) ) thresh=1500;
    }
    if( (f->cauldron!=0) && (f->linfscheme==1) )  // HPMI non-primes
    {
        thresh=10000;  // for those not measured yet
        if(f->fdef==4) thresh=7000;
        if(f->fdef==9) thresh=18000;
        if(f->fdef==16) thresh=12000;
        if( (f->charc>=67)  && (f->charc<=193) ) thresh=4500;
    }
    if( (thresh==11111) && (f->linfscheme==1) )  // linf over Dfmt primes
    {
        thresh=110;
    }
    if(thresh==11111)  // remaining cases are Dfmt
    {
        if(f->madtyp==2)  thresh=1800;  // 8-bit prime > 193
        if(f->madtyp==4)  thresh=100;   // 16-bit prime
        if(f->madtyp==5)  thresh=40;    // Zech logs
        if(f->madtyp==6)  thresh=40;    // Zech logs  untested
        if(f->madtyp==7)  thresh=40;    // Zech logs
        if(f->madtyp==8)  thresh=40;    // Zech logs xor untested
        if(f->madtyp==9)  thresh=20;    // 32-bit prime
        if(f->madtyp==10) thresh=2;     // qmul 32-bit
        if(f->madtyp==12)
        {
            if(f->pow==1) thresh=25;    // 64-bit prime
               else       thresh=2;     // qmul 64-bit
        }
    }
    lev=0;
    while(1)
    {
        if( (r1<thresh) || (r2<thresh) || (r3<thresh) ) break;
        if( ((r1&1)==1) || ((r2&1)==1) || ((r3&1)==1) ||
            ((r4&1)==1) || ((r5&1)==1) || ((r6&1)==1) ) break;
        lev++;
        r1/=2;  r2/=2;  r3/=2;  r4/=2;  r5/=2;   r6/=2;
    }
    if( (dsa->noc%f->entbyte)!=0 ) lev=0;
    if( (dsbc->noc%f->entbyte)!=0 ) lev=0;
#ifdef DEBUG
printf("thresh %lu   lev %d\n",thresh,lev);
#endif
    SWMul(dsa,dsbc,nora,a,astride,b,bstride,c,lev);
}



#ifdef NEVER

// stripping code in case it is needed later

// Need to strip so that everything is divisible by divis
// Phase 1 - number of rows of A/C must be divisible by divis
    stnora=nora%divis;
    if( stnora!=0 )
    {
        nora=nora-stnora;
        DFMul(dsa,dsbc,stnora,a+nora*astride,astride,b,bstride,
              c+nora*dsbc->nob);
    }
// Phase 2 - the number of bytes in a row of A must be divisible
// by divis*entbyte (doesn't work for PSSet fields!)
    stnoca=dsa->noc%(divis*f->entbyte);
    DSSet(f,stnoca,&tempds);
    newnoca=dsa->noc-stnoca;
    DSSet(f,newnoca,&newdsa);
    DFMul(&tempds,dsbc,nora,a+newdsa.nob,astride,
              b+bstride*newnoca,bstride, c);
    dsa1=&newdsa;
// Phase 3 - the number of bytes in a row of B/C must be divisible
    stnocb=dsbc->noc%(divis*f->entbyte);
    cstride=dsbc->nob;
    DSSet(f,stnocb,&tempds);
    newnocb=dsbc->noc-stnocb;
    DSSet(f,newnocb,&newdsbc);
    DFMad(dsa1,&tempds,nora,a,astride,
          b+newdsbc.nob,bstride,c+newdsbc.nob,cstride);
    dsbc1=&newdsbc;

#endif


void SLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
          Dfmt * c, uint64_t nora, uint64_t noca, uint64_t nocb)
{
    DSPACE dsa,dsbc;
    DSSet(f,noca,&dsa);
    DSSet(f,nocb,&dsbc);
    TSMul(&dsa,&dsbc,nora,a,dsa.nob,b,dsbc.nob,c);
    return;
}

extern void SLMad(const FIELD * f, const Dfmt * a, const Dfmt * b,
                  Dfmt * temp, Dfmt * c, 
                  uint64_t nora, uint64_t noca, uint64_t nocb)
{
    DSPACE ds;
    if(nora==0) return;
    if(noca==0) return;
    if(nocb==0) return;
    SLMul(f,a,b,temp,nora,noca,nocb);
    DSSet(f,nocb,&ds);
    DAdd(&ds,nora,temp,c,c);
}

void SLTra(const FIELD *f, const Dfmt *am, Dfmt *bm,
           uint64_t naru, uint64_t nacu)
{

// a is matrix from, b is matrix to:

    long nar,nac,kbd,i,j,k;
    nar=naru;
    nac=nacu;

    if(f->paktyp==0)
    {
        uint64_t *a64,*b64;
        for(i=0;i<nar;i+=8)
        {
            kbd=nar-i;
            if(kbd>8) kbd=8;
            a64=(uint64_t *)am;
            a64+=i*nac;
            b64=(uint64_t *)bm;
            b64+=i;
            for(j=0;j<nac;j++)
            {
                for(k=0;k<kbd;k++) *(b64+k)=*(a64+k*nac);
                a64++;
                b64+=nar;
            }
        }
        return;
    }

    if(f->paktyp==1)
    {
        uint32_t *a32,*b32;
        for(i=0;i<nar;i+=16)
        {
            kbd=nar-i;
            if(kbd>16) kbd=16;
            a32=(uint32_t *)am;
            a32+=i*nac;
            b32=(uint32_t *)bm;
            b32+=i;
            for(j=0;j<nac;j++)
            {
                for(k=0;k<kbd;k++) *(b32+k)=*(a32+k*nac);
                a32++;
                b32+=nar;
            }
        }
        return;
    }

    if(f->paktyp==2)
    {
        uint16_t *a16,*b16;
        for(i=0;i<nar;i+=32)
        {
            kbd=nar-i;
            if(kbd>32) kbd=32;
            a16=(uint16_t *)am;
            a16+=i*nac;
            b16=(uint16_t *)bm;
            b16+=i;
            for(j=0;j<nac;j++)
            {
                for(k=0;k<kbd;k++) *(b16+k)=*(a16+k*nac);
                a16++;
                b16+=nar;
            }
        }
        return;
    }

    if(f->paktyp==3)
    {
        uint8_t *a8,*b8;
        for(i=0;i<nar;i+=64)
        {
            kbd=nar-i;
            if(kbd>64) kbd=64;
            a8=(uint8_t *)am;
            a8+=i*nac;
            b8=(uint8_t *)bm;
            b8+=i;
            for(j=0;j<nac;j++)
            {
                for(k=0;k<kbd;k++) *(b8+k)=*(a8+k*nac);
                a8++;
                b8+=nar;
            }
        }
        return;
    }

    if(f->paktyp==4)
    {
        uint8_t *a8,*b8,*f8;
        uint16_t *tra16;
        uint16_t x;
        uint64_t noba,nobb;

        f8=(uint8_t *)f;
        tra16=(uint16_t *) (f8+f->Ttra);
        noba=(nac+1)/2;
        nobb=(nar+1)/2;

        for(i=0;i<nar;i+=128)
        {
            kbd=nar-i;
            if(kbd>128) kbd=128;

            a8=(uint8_t *)am;
            a8+=i*noba;
            b8=(uint8_t *)bm;
            b8+=i/2;
            for(j=0;j<(nac-1);j+=2)
            {
                for(k=0;k<(kbd-1);k+=2)
                {
                    x    =   tra16[*(a8+k*noba)]+
                     f->fdef*tra16[*(a8+(k+1)*noba)];
                    *(b8+k/2)=x&255;
                    *(b8+k/2+nobb)=(x>>8)&255;
                }
                if(k<kbd)
                {
                    x    =   tra16[*(a8+k*noba)];
                    *(b8+k/2)=x&255;
                    *(b8+k/2+nobb)=(x>>8)&255;
                }
                a8++;
                b8+=2*nobb;
            }
            if(j<nac)
            {
                for(k=0;k<(kbd-1);k+=2)
                {
                    x    =   tra16[*(a8+k*noba)]+
                     f->fdef*tra16[*(a8+(k+1)*noba)];
                    *(b8+k/2)=x&255;
                }
                if(k<kbd)
                {
                    x    =   tra16[*(a8+k*noba)];
                    *(b8+k/2)=x&255;
                }
            }
        }
        return;
    }

    if(f->paktyp==5)
    {
        uint8_t *a8,*b8,*f8;
        uint32_t *tra32;
        uint32_t x;
        uint64_t noba,nobb;

        f8=(uint8_t *)f;
        tra32=(uint32_t *) (f8+f->Ttra);
        noba=(nac+2)/3;
        nobb=(nar+2)/3;

        for(i=0;i<nar;i+=192)
        {
            kbd=nar-i;
            if(kbd>192) kbd=192;

            a8=(uint8_t *)am;
            a8+=i*noba;
            b8=(uint8_t *)bm;
            b8+=i/3;
            for(j=0;j<(nac-2);j+=3)
            {
                for(k=0;k<(kbd-2);k+=3)
                {
                    x    =   tra32[*(a8+k*noba)]+
                           5*tra32[*(a8+(k+1)*noba)]+
                          25*tra32[*(a8+(k+2)*noba)];
                    *(b8+k/3)=x&255;
                    *(b8+k/3+nobb)=(x>>8)&255;
                    *(b8+k/3+2*nobb)=(x>>16)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra32[*(a8+k*noba)];
                    if( (kbd-k)==2 )
                        x+=5*tra32[*(a8+(k+1)*noba)];
                    *(b8+k/3)=x&255;
                    *(b8+k/3+nobb)=(x>>8)&255;
                    *(b8+k/3+2*nobb)=(x>>16)&255;
                }
                a8++;
                b8+=3*nobb;
            }
            if( (nac-j)!=0 )
            {
                for(k=0;k<(kbd-2);k+=3)
                {
                    x    =   tra32[*(a8+k*noba)]+
                           5*tra32[*(a8+(k+1)*noba)]+
                          25*tra32[*(a8+(k+2)*noba)];
                    *(b8+k/3)=x&255;
                    if( (nac-j)==2) *(b8+k/3+nobb)=(x>>8)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra32[*(a8+k*noba)];
                    if( (kbd-k)==2 )
                        x+=5*tra32[*(a8+(k+1)*noba)];
                    *(b8+k/3)=x&255;
                    if( (nac-j)==2) *(b8+k/3+nobb)=(x>>8)&255;
                }

                a8++;
                b8+=3*nobb;
            }
        }
        return;
    }

    if(f->paktyp==6)
    {
        uint8_t *a8,*b8,*f8;
        uint32_t *tra32;
        uint32_t x;
        uint64_t noba,nobb;

        f8=(uint8_t *)f;
        tra32=(uint32_t *) (f8+f->Ttra);
        noba=(nac+3)/4;
        nobb=(nar+3)/4;

        for(i=0;i<nar;i+=256)
        {
            kbd=nar-i;
            if(kbd>256) kbd=256;

            a8=(uint8_t *)am;
            a8+=i*noba;
            b8=(uint8_t *)bm;
            b8+=i/4;
            for(j=0;j<(nac-3);j+=4)
            {
                for(k=0;k<(kbd-3);k+=4)
                {
                    x    =   tra32[*(a8+k*noba)]+
                           4*tra32[*(a8+(k+1)*noba)]+
                          16*tra32[*(a8+(k+2)*noba)]+
                          64*tra32[*(a8+(k+3)*noba)];
                    *(b8+k/4)=x&255;
                    *(b8+k/4+nobb)=(x>>8)&255;
                    *(b8+k/4+2*nobb)=(x>>16)&255;
                    *(b8+k/4+3*nobb)=(x>>24)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra32[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=4*tra32[*(a8+(k+1)*noba)];
                    if( (kbd-k)==3 )
                        x+=16*tra32[*(a8+(k+2)*noba)];
                    *(b8+k/4)=x&255;
                    *(b8+k/4+nobb)=(x>>8)&255;
                    *(b8+k/4+2*nobb)=(x>>16)&255;
                    *(b8+k/4+3*nobb)=(x>>24)&255;
                }
                a8++;
                b8+=4*nobb;
            }
            if( (nac-j)!=0 )
            {
                for(k=0;k<(kbd-3);k+=4)
                {
                    x    =   tra32[*(a8+k*noba)]+
                           4*tra32[*(a8+(k+1)*noba)]+
                          16*tra32[*(a8+(k+2)*noba)]+
                          64*tra32[*(a8+(k+3)*noba)];
                    *(b8+k/4)=x&255;
                    if( (nac-j)>=2) *(b8+k/4+nobb)=(x>>8)&255;
                    if( (nac-j)==3) *(b8+k/4+2*nobb)=(x>>16)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra32[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=4*tra32[*(a8+(k+1)*noba)];
                    if( (kbd-k)==3 )
                        x+=16*tra32[*(a8+(k+2)*noba)];
                    *(b8+k/4)=x&255;
                    if( (nac-j)>=2) *(b8+k/4+nobb)=(x>>8)&255;
                    if( (nac-j)==3) *(b8+k/4+2*nobb)=(x>>16)&255;
                }
                a8++;
                b8+=4*nobb;
            }
        }
        return;
    }

    if(f->paktyp==7)
    {
        uint8_t *a8,*b8,*f8;
        uint64_t *tra64;
        uint64_t x;
        uint64_t noba,nobb;

        f8=(uint8_t *)f;
        tra64=(uint64_t *) (f8+f->Ttra);
        noba=(nac+4)/5;
        nobb=(nar+4)/5;

        for(i=0;i<nar;i+=320)
        {
            kbd=nar-i;
            if(kbd>320) kbd=320;

            a8=(uint8_t *)am;
            a8+=i*noba;
            b8=(uint8_t *)bm;
            b8+=i/5;
            for(j=0;j<(nac-4);j+=5)
            {
                for(k=0;k<(kbd-4);k+=5)
                {
                    x    =   tra64[*(a8+k*noba)]+
                           3*tra64[*(a8+(k+1)*noba)]+
                           9*tra64[*(a8+(k+2)*noba)]+
                          27*tra64[*(a8+(k+3)*noba)]+
                          81*tra64[*(a8+(k+4)*noba)];
                    *(b8+k/5)=x&255;
                    *(b8+k/5+nobb)=(x>>8)&255;
                    *(b8+k/5+2*nobb)=(x>>16)&255;
                    *(b8+k/5+3*nobb)=(x>>24)&255;
                    *(b8+k/5+4*nobb)=(x>>32)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra64[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=3*tra64[*(a8+(k+1)*noba)];
                    if( (kbd-k)>=3 )
                        x+=9*tra64[*(a8+(k+2)*noba)];
                    if( (kbd-k)>=4 )
                        x+=27*tra64[*(a8+(k+3)*noba)];
                    *(b8+k/5)=x&255;
                    *(b8+k/5+nobb)=(x>>8)&255;
                    *(b8+k/5+2*nobb)=(x>>16)&255;
                    *(b8+k/5+3*nobb)=(x>>24)&255;
                    *(b8+k/5+4*nobb)=(x>>32)&255;
                }
                a8++;
                b8+=5*nobb;
            }
            if( (nac-j)!=0 )
            {
                for(k=0;k<(kbd-4);k+=5)
                {
                    x    =   tra64[*(a8+k*noba)]+
                           3*tra64[*(a8+(k+1)*noba)]+
                           9*tra64[*(a8+(k+2)*noba)]+
                          27*tra64[*(a8+(k+3)*noba)]+
                          81*tra64[*(a8+(k+4)*noba)];
                    *(b8+k/5)=x&255;
                    if( (nac-j)>=2) *(b8+k/5+nobb)=(x>>8)&255;
                    if( (nac-j)>=3) *(b8+k/5+2*nobb)=(x>>16)&255;
                    if( (nac-j)>=4) *(b8+k/5+3*nobb)=(x>>24)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra64[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=3*tra64[*(a8+(k+1)*noba)];
                    if( (kbd-k)>=3 )
                        x+=9*tra64[*(a8+(k+2)*noba)];
                    if( (kbd-k)>=4 )
                        x+=27*tra64[*(a8+(k+3)*noba)];
                    *(b8+k/5)=x&255;
                    if( (nac-j)>=2) *(b8+k/5+nobb)=(x>>8)&255;
                    if( (nac-j)>=3) *(b8+k/5+2*nobb)=(x>>16)&255;
                    if( (nac-j)>=4) *(b8+k/5+3*nobb)=(x>>24)&255;
                }
                a8++;
                b8+=5*nobb;
            }
        }
        return;
    }

    if(f->paktyp==8)
    {
        uint8_t *a8,*b8,*f8;
        uint64_t *tra64;
        uint64_t x;
        uint64_t noba,nobb;

        f8=(uint8_t *)f;
        tra64=(uint64_t *) (f8+f->Ttra);
        noba=(nac+7)/8;
        nobb=(nar+7)/8;

        for(i=0;i<nar;i+=512)
        {
            kbd=nar-i;
            if(kbd>512) kbd=512;

            a8=(uint8_t *)am;
            a8+=i*noba;
            b8=(uint8_t *)bm;
            b8+=i/8;
            for(j=0;j<(nac-7);j+=8)
            {
                for(k=0;k<(kbd-7);k+=8)
                {
                    x    =   tra64[*(a8+k*noba)]+
                           2*tra64[*(a8+(k+1)*noba)]+
                           4*tra64[*(a8+(k+2)*noba)]+
                           8*tra64[*(a8+(k+3)*noba)]+
                          16*tra64[*(a8+(k+4)*noba)]+
                          32*tra64[*(a8+(k+5)*noba)]+
                          64*tra64[*(a8+(k+6)*noba)]+
                         128*tra64[*(a8+(k+7)*noba)];
                    *(b8+k/8)=x&255;
                    *(b8+k/8+nobb)=(x>>8)&255;
                    *(b8+k/8+2*nobb)=(x>>16)&255;
                    *(b8+k/8+3*nobb)=(x>>24)&255;
                    *(b8+k/8+4*nobb)=(x>>32)&255;
                    *(b8+k/8+5*nobb)=(x>>40)&255;
                    *(b8+k/8+6*nobb)=(x>>48)&255;
                    *(b8+k/8+7*nobb)=(x>>56)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra64[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=2*tra64[*(a8+(k+1)*noba)];
                    if( (kbd-k)>=3 )
                        x+=4*tra64[*(a8+(k+2)*noba)];
                    if( (kbd-k)>=4 )
                        x+=8*tra64[*(a8+(k+3)*noba)];
                    if( (kbd-k)>=5 )
                        x+=16*tra64[*(a8+(k+4)*noba)];
                    if( (kbd-k)>=6 )
                        x+=32*tra64[*(a8+(k+5)*noba)];
                    if( (kbd-k)>=7 )
                        x+=64*tra64[*(a8+(k+6)*noba)];
                    *(b8+k/8)=x&255;
                    *(b8+k/8+nobb)=(x>>8)&255;
                    *(b8+k/8+2*nobb)=(x>>16)&255;
                    *(b8+k/8+3*nobb)=(x>>24)&255;
                    *(b8+k/8+4*nobb)=(x>>32)&255;
                    *(b8+k/8+5*nobb)=(x>>40)&255;
                    *(b8+k/8+6*nobb)=(x>>48)&255;
                    *(b8+k/8+7*nobb)=(x>>56)&255;
                }
                a8++;
                b8+=8*nobb;
            }
            if( (nac-j)!=0 )
            {
                for(k=0;k<(kbd-7);k+=8)
                {
                    x    =   tra64[*(a8+k*noba)]+
                           2*tra64[*(a8+(k+1)*noba)]+
                           4*tra64[*(a8+(k+2)*noba)]+
                           8*tra64[*(a8+(k+3)*noba)]+
                          16*tra64[*(a8+(k+4)*noba)]+
                          32*tra64[*(a8+(k+5)*noba)]+
                          64*tra64[*(a8+(k+6)*noba)]+
                         128*tra64[*(a8+(k+7)*noba)];
                    *(b8+k/8)=x&255;
                    if( (nac-j)>=2) *(b8+k/8+nobb)=(x>>8)&255;
                    if( (nac-j)>=3) *(b8+k/8+2*nobb)=(x>>16)&255;
                    if( (nac-j)>=4) *(b8+k/8+3*nobb)=(x>>24)&255;
                    if( (nac-j)>=5) *(b8+k/8+4*nobb)=(x>>32)&255;
                    if( (nac-j)>=6) *(b8+k/8+5*nobb)=(x>>40)&255;
                    if( (nac-j)>=7) *(b8+k/8+6*nobb)=(x>>48)&255;
                }
                if( (kbd-k)!=0 )
                {
                    x    =   tra64[*(a8+k*noba)];
                    if( (kbd-k)>=2 )
                        x+=2*tra64[*(a8+(k+1)*noba)];
                    if( (kbd-k)>=3 )
                        x+=4*tra64[*(a8+(k+2)*noba)];
                    if( (kbd-k)>=4 )
                        x+=8*tra64[*(a8+(k+3)*noba)];
                    if( (kbd-k)>=5 )
                        x+=16*tra64[*(a8+(k+4)*noba)];
                    if( (kbd-k)>=6 )
                        x+=32*tra64[*(a8+(k+5)*noba)];
                    if( (kbd-k)>=7 )
                        x+=64*tra64[*(a8+(k+6)*noba)];
                    *(b8+k/8)=x&255;
                    if( (nac-j)>=2) *(b8+k/8+nobb)=(x>>8)&255;
                    if( (nac-j)>=3) *(b8+k/8+2*nobb)=(x>>16)&255;
                    if( (nac-j)>=4) *(b8+k/8+3*nobb)=(x>>24)&255;
                    if( (nac-j)>=5) *(b8+k/8+4*nobb)=(x>>32)&255;
                    if( (nac-j)>=6) *(b8+k/8+5*nobb)=(x>>40)&255;
                    if( (nac-j)>=7) *(b8+k/8+6*nobb)=(x>>48)&255;
                }
                a8++;
                b8+=8*nobb;
            }
        }
        return;
    }
}

// REX rs1 a2 a2p a2np

void RowSel(DSPACE * ds, uint64_t * bs, Dfmt * x, Dfmt * xs, Dfmt * xn)
{
    Dfmt *p,*ps,*pn;
    uint64_t i;
    p=x;
    ps=xs;
    pn=xn;
    for(i=0;i<bs[0];i++)
    {
        if(BSBitRead(bs,i)==1)
        {
            memcpy(ps,p,ds->nob);
            ps+=ds->nob;
        }
        else
        {
            memcpy(pn,p,ds->nob);
            pn+=ds->nob;
        }
        p+=ds->nob;
    }
}

// RRF rs1 a2p a2np a2

void RowRif(DSPACE * ds, uint64_t * bs, Dfmt * xs, Dfmt * xn, Dfmt * x)
{
    Dfmt *p,*ps,*pn;
    uint64_t i;
    p=x;
    ps=xs;
    pn=xn;
    for(i=0;i<bs[0];i++)
    {
        if(BSBitRead(bs,i)==1)
        {
            memcpy(p,ps,ds->nob);
            ps+=ds->nob;
        }
        else
        {
            memcpy(p,pn,ds->nob);
            pn+=ds->nob;
        }
        p+=ds->nob;
    }
}

// "recursive" echelize routine

uint64_t RCEch(GAUSS * gs, DSPACE * ds, Dfmt *a, 
             uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    uint64_t colleft,colright,i,j,rk1,rk2,rank,rowtop,rowbottom;
    FELT det1,det2;
    const FIELD * f;
    Dfmt *a1,*a2,*m1,*c1,*r1,*a2p,*a2np;
    Dfmt *m2,*c2,*r2,*y,*temp,*rem1,*y1,*y2,*ku,*x1,*x2,*mu,*ml;
    uint64_t *rs1,*cs1,*rs2,*cs2,*rf,*cf;
    DSPACE ds1,ds2,ds3,ds4,dsrem,dsm;
    f=ds->f;
    if( (nor<=gs->maxrows) && (ds->noc<=gs->maxcols) )
    {
        rank = BCEch(gs,ds,a,rs,cs,det,m,c,r,nor,0);
        return rank;
    }
    memset(rs,0,16+8*((nor+63)/64));
    memset(cs,0,16+8*((ds->noc+63)/64));

//    TRIM    special case if matrix is entirely zero
    for(i=0;i<nor;i++)
    {
        j=DNzl(ds,a+i*ds->nob);
        if(j!=ZEROROW) break;
    }
    if(i==nor)        // matrix is zero
    {
        rs[0]=nor;    // rs is nor zero bits
        rs[1]=0;
        cs[0]=ds->noc;  // cs is ds->noc zero bits
        cs[1]=0;
//  M is rank x rank so nothing of determinant 1
        *det=1;
//  C is nor x rank so nothing
//  R is rank x ds->noc so nothing 
        return 0;
    }
    if(nor<ds->noc)   // chop L/R
    {
        colleft=ds->noc/2+8;
        if(colleft>(nor+10)) colleft=nor+10;
        i=colleft%f->entbyte;
        colleft-=i;
        i=SLSize(f,nor,colleft);
        a1=(Dfmt *) malloc(i);
        DSSet(f,colleft,&ds1);
        DCut(ds,nor,0,a,&ds1,a1);
        i=SLSizeM(f,nor,colleft);
        m1=(Dfmt *) malloc(i);
        i=SLSizeC(f,nor,colleft);
        c1=(Dfmt *) malloc(i);
        i=SLSizeR(f,nor,colleft);
        r1=(Dfmt *) malloc(i);
        i=16+((nor+63)/64)*8;
        rs1=malloc(i);
        rk1=RCEch(gs,&ds1,a1,rs1,cs,&det1,m1,c1,r1,nor);
        free(a1);
// special cases rk1=nor and rk1=0 ?
        colright=ds->noc-colleft;
        i=SLSize(f,nor,colright);
        a2=(Dfmt *) malloc(i);
        DSSet(f,colright,&ds2);
        DCut(ds,nor,colleft,a,&ds2,a2);
        i=SLSize(f,rk1,colright);
        a2p=(Dfmt *) malloc(i);
        i=SLSize(f,nor-rk1,colright);
        a2np=(Dfmt *) malloc(i);
        RowSel(&ds2,rs1,a2,a2p,a2np);
        free(a2);
        i=SLSize(f,nor-rk1,colright);
        temp=malloc(i);
        SLMad(f,c1,a2p,temp,a2np,nor-rk1,rk1,colright);
        free(temp);
        i=SLSizeM(f,nor-rk1,colright);
        m2=(Dfmt *) malloc(i);
        i=SLSizeC(f,nor-rk1,colright);
        c2=(Dfmt *) malloc(i);
        i=SLSizeR(f,nor-rk1,colright);
        r2=(Dfmt *) malloc(i);
        i=16+((nor-rk1+63)/64)*8;
        rs2=malloc(i);
        i=16+((colright+63)/64)*8;
        cs2=malloc(i);
        rk2=RCEch(gs,&ds2,a2np,rs2,cs2,&det2,m2,c2,r2,nor-rk1);
        free(a2np);
        rank=rk1+rk2;
        DSSet(f,rank,&dsm);
        i=16+((rank+63)/64)*8;
        rf=malloc(i); 
        BSCombine(rs1,rs2,rs,rf);
        BSShiftOr(cs2,colleft,cs);
        cs[0]=ds->noc;
        cs[1]=rank;
        i=SLSize(f,rk1,colright);
        temp=malloc(i);
        SLMul(f,m1,a2p,temp,rk1,rk1,colright);
        free(a2p);
        i=SLSize(f,rk1,rk2);
        x1=malloc(i);
        i=SLSize(f,rk1,colright-rk2);
        x2=malloc(i);
        BSColSelect(f,cs2,rk1,temp,x1,x2);
        free(cs2);
        free(temp);
        i=SLSize(f,rk1,colright-rk2);
        temp=malloc(i);
        DSSet(f,colright-rk2,&ds4);
        SLMad(f,x1,r2,temp,x2,rk1,rk2,colright-rk2);
        free(temp);
        DSSet(f,ds->noc-rank,&dsrem);
        memset(r,0,dsrem.nob*rank);
        DSSet(f,colleft-rk1,&ds3);
        DPaste(&ds3,r1,rk1,0,&dsrem,r);
        free(r1);
        DPaste(&ds4,x2,rk1,colleft-rk1,&dsrem,r);
        free(x2);
        DPaste(&ds4,r2,rk2,colleft-rk1,&dsrem,r+rk1*dsrem.nob);
        free(r2);
        i=SLSize(f,nor-rk1,rank);
        temp=malloc(i);
        BSColRifZ(f,rf,nor-rk1,c1,temp);
        free(c1);
        i=SLSize(f,rk2,rank);
        y=malloc(i);
        RowSel(&dsm,rs2,temp,y,c);
        free(rs2);
        BSColPutS(f,rf,rk2,1,y);
        SLMad(f,c2,y,temp,c,nor-rank,rk2,rank);
        // cleaner
        free(c2);
        SLMul(f,m2,y,temp,rk2,rk2,rank);
        free(m2);
        free(y);
        BSColRifZ(f,rf,rk1,m1,m);
        free(m1);
        free(rf);
        i = SLSize(f, rk1, rank);
        ku = malloc(i);
        SLMad(f,x1,temp,ku,m,rk1,rk2,rank);
        free(x1);
        free(ku);
        DCpy(&dsm, temp, rk2, m + rk1*dsm.nob);
        free(temp);
        if(BSRifDet(rs1)==1) det1=FieldNeg(f,det1);
        free(rs1);
        *det=FieldMul(f,det1,det2);
        return rank;
    }
    else              // chop top/bottom
    {
        rowtop = nor/2;
        if (rowtop  > ds->noc + 10)
            rowtop = ds->noc + 10;
        rowbottom = nor - rowtop;
        i=SLSizeM(f,rowtop,ds->noc);
        m1=(Dfmt *) malloc(i);
        i=SLSizeC(f,rowtop, ds->noc);
        c1=(Dfmt *) malloc(i);
        i=SLSizeR(f,rowtop, ds->noc);
        r1=(Dfmt *) malloc(i);
        i=16+((ds->noc+63)/64)*8;
        cs1=malloc(i);
        rk1 = RCEch(gs,ds, a, rs, cs1, &det1, m1, c1, r1, rowtop);
        a2 = a + rowtop*ds->nob;
        i = SLSize(f,rowbottom, rk1);
        a2p = malloc(i);
        i = SLSize(f, rowbottom, ds->noc - rk1);
        a2np = malloc(i);
        BSColSelect(f, cs1, rowbottom, a2, a2p, a2np);
        temp = malloc(i); // same size as a2np
        DSSet(f, ds->noc - rk1, &ds2);
        SLMad(f, a2p, r1, temp, a2np, rowbottom, rk1, ds->noc - rk1);
        free(temp);
        i=SLSizeM(f,rowbottom,ds->noc-rk1);
        m2=(Dfmt *) malloc(i);
        i=SLSizeC(f,rowbottom,ds->noc-rk1);
        c2=(Dfmt *) malloc(i);
        i=SLSizeR(f,rowbottom,ds->noc-rk1);
        r2=(Dfmt *) malloc(i);
        i=16+(((ds->noc-rk1)+63)/64)*8;
        cs2=malloc(i);
        i = 16+((rowbottom + 63)/64)*8;
        rs2 = malloc(i);
        rk2 = RCEch(gs,&ds2, a2np, rs2, cs2, &det2, m2, c2, r2, rowbottom);
        free(a2np);
        rank = rk1 + rk2;
        DSSet(f, rank, &dsm);
        i = 16 + ((rank + 63)/64)*8;
        cf = malloc(i);
        BSCombine(cs1, cs2, cs, cf);
        free(cs1);
        BSShiftOr(rs2, rowtop, rs);
        rs[0]=nor;
        rs[1]=rank;
        i = SLSize(f, rk1, rk2);
        x1 = malloc(i);
        i = SLSize(f, rk1, ds->noc - rank);
        rem1 = malloc(i);
        BSColSelect(f, cs2, rk1, r1, x1, rem1);
        free(r1);
        free(cs2);
        temp = malloc(i); //same dimension as rem1
        DSSet(f, ds->noc - rank, &dsrem);
        SLMad(f, x1, r2, temp, rem1, rk1, rk2, ds->noc - rank);
        free(temp);
        RowRif(&dsrem, cf, rem1, r2, r);
        free(r2);
        free(rem1);
        i = SLSize(f, rowbottom, rk1);
        temp = malloc(i);
        SLMul(f,a2p,m1,temp,rowbottom, rk1, rk1);
        free(a2p);
        DSSet(f, rk1, &ds3);
        i = SLSize(f, rk2, rk1);
        y1 = malloc(i);
        i = SLSize(f, rowbottom-rk2, rk1);
        y2 = malloc(i);
        RowSel(&ds3, rs2, temp, y1, y2);
        free(rs2);
        SLMad(f,c2, y1, temp, y2, rowbottom-rk2, rk2, rk1); // temp big enough
        memset(c, 0, SLSize(f, nor - rank, rank));
        DPaste(&ds3, c1, rowtop-rk1, 0, &dsm, c);
        free(c1);
        DPaste(&ds3, y2, rowbottom-rk2, 0, &dsm, c + dsm.nob*(rowtop - rk1));
        free(y2);
        DSSet(f, rk2, &ds4);
        DPaste(&ds4, c2, rowbottom-rk2, rk1, &dsm, c + dsm.nob*(rowtop - rk1));
        free(c2);
        i = SLSize(f, rk1, rank);
        mu = malloc(i);
        memset(mu,0,i);
        DPaste(&ds3, m1, rk1, 0, &dsm, mu);
        free(m1);
        i = SLSize(f, rk2, rank);
        ml = malloc(i);
        memset(ml,0,i);
        SLMul(f, m2, y1, temp, rk2, rk2, rk1); // temp still the right size
        free(y1);
        DPaste(&ds3, temp, rk2, 0, &dsm, ml);
        free(temp);
        DPaste(&ds4, m2, rk2, rk1, & dsm, ml);
        free(m2);
        i = SLSize(f, rk1, rank);
        temp = malloc(i);
        SLMad(f, x1, ml, temp, mu, rk1, rk2, rank);
        free(x1);
        free(temp);
        memset(m,0,SLSize(f, rank, rank));
        RowRif(&dsm, cf, mu, ml, m);
        free(mu);
        free(ml);
        if(BSRifDet(cf)==1) det1=FieldNeg(f,det1);
        free(cf);
        *det=FieldMul(f,det1,det2);
        return rank;
    }
}

// "exported" slab echelize - needs early release etc. with proggies

uint64_t SLEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    GAUSS * gs;
    uint64_t rank;
    gs=GaussCreate(ds->f);
    rank = RCEch(gs,ds,a,rs,cs,det,m,c,r,nor);
    GaussDestroy(gs);
    return rank;
}


/* end of slab.c  */
