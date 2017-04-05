/*    Meataxe-64    pcrit0.c     */
/*    ==========    ========     */

/*    R. A. Parker     10.July 2015 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"

/* this routine is here because it depend on technology */

extern void pchal(FIELD * f)
{
    f->pcgen=0;
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


void pcxor(Dfmt * d, const Dfmt * s1, const Dfmt * s2, int nob)
{
    uint64_t i;
    for(i=0;i<nob;i++)
        *(d++) = *(s1++) ^ *(s2++);
}

void pcbunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1, const uint8_t * t2)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = *(t2+256*(*d)+*(t1+(*(s++))));
        d++;
    }
}

void pcxunf(Dfmt * d, const Dfmt * s, uint64_t nob,
                   const uint8_t * t1)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = (*d)^*(t1+(*(s++)));
        d++;
    }
}

void pcunf(Dfmt * d, uint64_t nob, const uint8_t * t1)
{
    uint64_t i;
    for(i=0;i<nob;i++)
    {
        *d = *(t1+(*d));
        d++;
    }
}

void pcbif(Dfmt * d,  const Dfmt * s1, const Dfmt * s2,
            uint64_t nob, const uint8 * t)
{
    uint64_t i;
    for(i=0;i<nob;i++)
        *(d++) = t[((*(s1++))*256) + *(s2++)];
}

/*  characteristic 2 brick mad routine  */

void pcbm2(const uint8 *a, uint8 * bv, uint8 *c)
{
    uint64 *b;
    uint8 * ap;
    uint64 * bkr;
    uint64 * cp;
    int ax;
    int i,j;
    uint64 *ptc,*pt1,*pt2,*pt3,*pt4;
    b=(uint64 *) bv;
    ap=(uint8 *) a;       // ap is pointer to Afmt
    cp=(uint64 *) c;      // cp is pointer to Cfmt

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
void pcad3(const uint8 *a, const uint8 *b, uint8 * c)
{
    printf("pcad3 not implemented in pcrit0\n");
}

void pcbm3(const uint8 *a, uint8 * bv, uint8 *c)
{
    printf("pcbm3 Not implemented yet in pcrit0\n");
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

/* end of pcrit0c.c  */
