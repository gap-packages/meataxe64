/*    Meataxe-64    pcritGENc.c     */
/*    ==========    ========     */

/*    R. A. Parker     5.Nov 2017 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "tabmake.h"

void hpmiset(FIELD * f)
{
    f->cauldron=0;
    if(f->charc==2)
    {
        f->AfmtMagic=1;
        f->BfmtMagic=1;
        f->CfmtMagic=1;
        f->BwaMagic=2;
        f->GreaseMagic=2;
        f->SeedMagic=2;
        f->cauldron=512;
        f->bfmtcauld=64;
        f->cfmtcauld=64;
        f->dfmtcauld=64;
        f->alcove=32;
        f->alcovebytes=5;
        f->recbox=1024;
        f->czer=0;
        f->bzer=0;
        f->bbrickbytes=2049;
        f->bwasize=8192;
    }
    return;
}

// GEN implementation of large fields of characteristic 2

void pccl32 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint32_t * d1, uint32_t * d2)
{
    uint64_t i,x,y,p,res,z;
    z=1;
    z=z<<(64-clpm[2]);

    p=clpm[1]>>(clpm[2]-1);

    for(i=0;i<noc;i++)
    {
        x=scalar>>clpm[2];
        y=*(d1++);
        res=0;
// invariant - answer = res + x*y
        while(x!=0)
        {
            if((x&1)!=0)
                res^=y;
            x=x>>1;
            y=y<<1;
            if((y&z)!=0) y^=p;
        }
        y=*d2;
        *(d2++)=y^res;
    }
}

void pccl64 (const uint64_t * clpm, uint64_t scalar, uint64_t noc, 
                      uint64_t * d1, uint64_t * d2)
{
    uint64_t i,x,y,p,res,z;
    z=1;
    z=z<<(64-clpm[2]);
    p=clpm[1]>>(clpm[2]-1);

    for(i=0;i<noc;i++)
    {
        y=*(d1++);
        x=scalar>>clpm[2];
        res=0;
// invariant - answer = res + x*y
        while(x!=0)
        {
            if((x&1)!=0)
                res^=y;
            x=x>>1;
            y=y<<1;
            if((y&z)!=0) y^=p;
        }
        y=*d2;
        *(d2++)=y^res;
    }
}

/* choose suitable nor from nob  */
uint64_t pcstride(uint64_t s)
{
    uint64_t i,j,k,d;
    if(s<600) return 200;    // fits in L2 in its entirety
    for(i=1;i<25;i++)        // how many L1 banks can we use
    {
        d=s*i;
        if( ((d+7)&4095)<15) break;
    }
    for(j=1;j<50;j++)        // how many L2 banks can we use
    {
        d=s*j;
        if( ((d+15)&65535)<31) break;
    }
    k=8*i;
    if(k<(4*j)) k=4*j;
    return k;
}

extern uint64_t pcpmad(uint64_t p,uint64_t a,uint64_t b,uint64_t c)
{
    uint64_t e,d,one;
    one=1;
    e=0;
/*  e + a*b (mod p) is the answer to start with  */
    while(b!=0)
    {
        if((b&one)==one)
        {
            e=e+a;
            if(e<a) e=e-p;
            e=e%p;
        }
        d=a+a;
        if(d<a) d=d-p;
        a=d%p;
        b=b>>one;
/* invariant . . . e + a*b+c (mod p) is the answer */
    }

    e=e+c;
    if(e<c) e=e-p;
    e=e%p;

    return e;
}


void pcxor(uint8_t * d, const uint8_t * s1, const uint8_t * s2, int nob)
{
    uint64_t i;
    for(i=0;i<nob;i++)
        *(d++) = *(s1++) ^ *(s2++);
}

void pcbunf(uint8_t * d, const uint8_t * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = *(t2+256*(*d)+*(t1+(*(s++))));
        d++;
    }
}

void pcxunf(uint8_t * d, const uint8_t * s, uint64_t nob,
                   const uint8_t * t1)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = (*d)^*(t1+(*(s++)));
        d++;
    }
}

void pcunf(uint8_t * d, uint64_t nob, const uint8_t * t1)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = *(t1+(*d));
        d++;
    }
}

void pcbif(uint8_t * d,  const uint8_t * s1, const uint8_t * s2,
            uint64_t nob, const uint8_t * t)
{
    uint64_t i;
    for(i=0;i<nob;i++)
        *(d++) = t[((*(s1++))*256) + *(s2++)];
}

/*  characteristic 2 brick mad routine  */

void pcbm2(const uint8_t *a, uint8_t * bv, uint8_t *c)
{
    uint64_t *b;
    uint8_t * ap;
    uint64_t * bkr;
    uint64_t * cp;
    int ax;
    int i,j;
    uint64_t *ptc,*pt1,*pt2,*pt3,*pt4;
    b=(uint64_t *) bv;
    ap=(uint8_t *) a;       // ap is pointer to Afmt
    cp=(uint64_t *) c;      // cp is pointer to Cfmt

    ax=*(ap++);           // get the very first byte of Afmt

    while(ax!=255)        // if terminate, that's the lot
    {
        cp+=(ax*8);      // skip rows as instructed (0 at start?)

        for(i=0;i<2;i++)  // process slices
        {
            bkr=b+512*i;
            ax=*(ap++);                    // byte of Afmt
            pt1=bkr+((ax&15)*8);          // indirect to grease row
            pt2=bkr+(((ax>>4)&15)*8)+128; // indirect to grease row
            ax=*(ap++);                    // byte of Afmt
            pt3=bkr+((ax&15)*8)+256;      // indirect to grease row
            pt4=bkr+(((ax>>4)&15)*8)+384; // indirect to grease row
            ptc=cp;
            for(j=0;j<8;j++)    // this is the real work!
                *(ptc++) ^=  *(pt1++)^*(pt2++)^*(pt3++)^*(pt4++);
        }
        ax=*(ap++);        // get next skip/terminate byte;
    }
}


/*  characteristic 3 routines  */
void pcad3(const uint8_t *a, const uint8_t *b, uint8_t * c)
{
    printf("pcad3 not implemented in GEN\n");
}

void pcbm3(const uint8_t *a, uint8_t * bv, uint8_t *c)
{
    printf("pcbm3 not implemented yet in GEN\n");
}

void pcbmas(const uint8_t *a, uint8_t * bv, uint8_t *c,
            const uint64_t * parms)
{
    printf("pcbmas not implemented yet in GEN\n");
}
void pcchain(const uint8_t *prog, uint8_t * bv, const uint64_t * parms)
{
    printf("pcchain not implemented yet in GEN\n");
}
/* end of pcrit0c.c  */
