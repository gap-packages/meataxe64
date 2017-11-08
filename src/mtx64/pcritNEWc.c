/*    Meataxe-64    pcritHASc.c     */
/*    ==========    =========     */

/*    R. A. Parker      12 July 2015 */

#include <stdio.h>
#include <stdint.h>
#include "field.h"
#include "tabmake.h"

/* these routines here because they depend on technology */

void hpmiset(FIELD * f)
{
    uint64_t bias;
    f->cauldron=0;
    if(f->charc==2)
    {
        f->AfmtMagic=1;
        f->alcovebytes=5;
        f->BfmtMagic=1;
        f->cauldron=1024;
        f->bfmtcauld=128;
        f->cfmtcauld=128;
        f->dfmtcauld=128;
        f->CfmtMagic=1;
        f->BwaMagic=2;
        f->GreaseMagic=2;
        f->SeedMagic=2;
        f->alcove=32;
        f->recbox=1024;
        f->czer=0;
        f->bzer=0;
        f->bbrickbytes=4097;
        f->bwasize=16384;
    }
    if(f->charc==3)
    {
        f->AfmtMagic=3;
        f->BfmtMagic=3;
        f->CfmtMagic=3;
        f->BwaMagic=3;
        f->GreaseMagic=3;
        f->SeedMagic=3;
        f->cauldron=510;
        f->bfmtcauld=128;
        f->cfmtcauld=128;
        f->dfmtcauld=102;
        f->alcove=12;
        f->alcovebytes=4;
        f->recbox=1024;
        f->czer=0xffffffffffffffff;
        f->bzer=0xffffffffffffffff;
        f->bbrickbytes=1537;
        f->bwasize=15744;
        hpmitab3(f);
    }
    if( (f->charc>=5)&&(f->charc<=181) )
    {
        f->AfmtMagic=2;
        f->BfmtMagic=1;
        f->CfmtMagic=2;
        if( (f->charc>=17) && (f->charc<=61) ) f->CfmtMagic=4;
        if( (f->charc>=7) && (f->charc<=13) ) f->CfmtMagic=5;
        if(f->charc==5) f->CfmtMagic=6;
        f->BwaMagic=1;
        f->GreaseMagic=1;
        f->SeedMagic=1;
        if( (f->charc>=17) && (f->charc<=61) ) f->SeedMagic=4;
        if( (f->charc>=7) && (f->charc<=13) ) f->SeedMagic=5;
        if(f->charc==5) f->SeedMagic=6;
        f->cauldron=64;
        if(f->charc<=61) f->cauldron=96;
        if(f->charc<=13) f->cauldron=128;
        if(f->charc==5)  f->cauldron=126;
        f->bfmtcauld=f->cauldron;
        if(f->charc<=13) f->bfmtcauld=f->cauldron/2;
        if(f->charc<=5) f->bfmtcauld=f->cauldron/3;
        f->cfmtcauld=128;
        f->dfmtcauld=f->bfmtcauld;
        f->alcove=7;
        if( (f->charc>=7) && (f->charc<=13) ) f->alcove=14;
        if (f->charc==5) f->alcove=21;
        f->alcovebytes=8;
        f->recbox=1024;
        bias=7*f->charc;
        if(f->charc<=7) bias=6*f->charc;
        if(f->charc>=67)
            f->czer=bias+(bias<<16)+(bias<<32)+(bias<<48);
        if( (f->charc>=17)&&(f->charc<=61) )
            f->czer=bias+(bias<<10)+(bias<<20)
                   +(bias<<32)+(bias<<42)+(bias<<52);
        if( (f->charc>=5)&&(f->charc<=13) )
            f->czer=bias+(bias<<8)+(bias<<16)+(bias<<24)
                   +(bias<<32)+(bias<<40)+(bias<<48)+(bias<<56);
        f->bzer=0;
        f->bbrickbytes=1+f->alcove*f->bfmtcauld;
        f->bwasize=16384;
        hpmitabas(f);
    }
    return;
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

/* end of pcritHASc.c  */
