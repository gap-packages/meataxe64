/*
         slab.c  -   Slab Routines Code + SAL SLEch
         ======      R. A. Parker 13.2.2018
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

// the next two are correct but far too pessimistic

uint64_t SLSizeC(const FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    uint64_t rank;
    rank=nor;
    if(rank>noc) rank=noc;
    DSSet(f,rank,&ds);
// columns at most rank, rows at most nor
    return ds.nob*nor;
}

uint64_t SLSizeR(const FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    DSSet(f,noc,&ds);
// remnant no bigger than the original matrix
    return ds.nob*nor;
}

void SLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
          Dfmt * c, uint64_t nora, uint64_t noca, uint64_t nocb)
{
    FELT e;
    const Dfmt *da,*db;
    Dfmt *dc;
    DSPACE dsa,dsb;
    uint64_t i,j;
    if(f->linfscheme!=0)  // linear functions case?
    {
        LLMul(f,a,b,c,nora,noca,nocb);
        return;
    }
    if(f->pow==1)    // Ground field - just call PLMul
    {
        PLMul(f,a,b,c,nora,noca,nocb);
        return;
    }
// else do it by steam over extension field
    DSSet(f,noca,&dsa);
    DSSet(f,nocb,&dsb);
    da=a;
    dc=c;
    memset(c,0,nora*dsb.nob);
    for(i=0;i<nora;i++)
    {
        db=(Dfmt *) b;
        for(j=0;j<noca;j++)
        {
            e=DUnpak(&dsa,j,da);
            DSMad(&dsb,e,1,db,dc);
            db+=dsb.nob;
        }
        da+=dsa.nob;
        dc+=dsb.nob;
    }
    return;
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

// Base case echelize - scheduled for demolition!

uint64_t BCEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    int * piv;
    uint64_t fel;
    uint64_t nck,rank,i,j,z,col;
    size_t sbsr, sbsc;
    FELT deter;
    DSPACE dsk,dsm,dsr;
    Dfmt *vo, *junk;
    Dfmt *k;
    Dfmt *va,*vk;
    const FIELD * f;

    f=ds->f;
    deter=1;
    k=r;    // use remnant area for keeptrack
    piv=malloc(nor*sizeof(int));
    vo=malloc(ds->nob);
    junk=malloc(ds->nob);
    nck=nor;
    if(nck>ds->noc) nck=ds->noc;
    DSSet(f,nck,&dsk);
    memset(m,0,nck*dsk.nob);
    memset(k,0,nor*dsk.nob);   // keeptrack starts as zero

    sbsr=8*(2+(nor+63)/64);
    memset(rs,0,sbsr);
    rs[0]=nor;
    sbsc=8*(2+(ds->noc+63)/64);
    memset(cs,0,sbsc);
    cs[0]=ds->noc;

    rank=0;

    for(i=0;i<nor;i++)
    {
        va=a+i*ds->nob;
        vk=k+i*dsk.nob;
        col=DNzl(ds,va);
        if(col==ZEROROW) continue;
        piv[i]=col;
        fel=DUnpak(ds,col,va);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(ds,fel,1,va);
        DPak(&dsk,rank,vk,1);
        DSMul(&dsk,fel,1,vk);
        deter=FieldMul(f,fel,deter);
        BSBitSet(rs,i);
        BSBitSet(cs,col);
        rank++;
        for(j=0;j<nor;j++)
        {
            if(j==i) continue;
            fel=DUnpak(ds,col,a+j*ds->nob);   // create A1
            DSMad(ds,fel,1,va,a+j*ds->nob);
            DSMad(&dsk,fel,1,vk,k+j*dsk.nob); // consume A1
        }
    }

    rs[1]=rank;
    cs[1]=rank;

    DSSet(f,rank,&dsm);    // first get the cleaner out
    memset(c,0,(nor-rank)*dsm.nob);
    z=0;
    for(j=0;j<nor;j++)
    {
        if(BSBitRead(rs,j)==1) continue;
        DCut(&dsk,1,0,k+j*dsk.nob,&dsm,c+z*dsm.nob);
        z++;     
    }
    z=0;                   // now get multiplier out
// still need to sort out the sign of the determinant here
    for(i=0;i<ds->noc;i++)     // by permuting the rows of keeptrack
    {
        if(BSBitRead(cs,i)==0) continue;
        for(j=0;j<nor;j++)
        {
            if(BSBitRead(rs,j)==0) continue;
            if(piv[j]!=i) continue;
            DCut(&dsk,1,0,k+j*dsk.nob,&dsm,m+z*dsm.nob);
            z++;
        }
    }
    DSSet(f,ds->noc-rank,&dsr);
    memset(r,0,(nor-rank)*dsr.nob);
    z=0;                   // finally the remnant
    for(i=0;i<ds->noc;i++)     // by permuting the rows of matrix a
    {
        if(BSBitRead(cs,i)==0) continue;
        for(j=0;j<nor;j++)
        {
            if(BSBitRead(rs,j)==0) continue;
            if(piv[j]!=i) continue;
            BSColSelect(f,cs,1,a+j*ds->nob,junk,r+z*dsr.nob);
            z++;
// I think this can break
        }
    }

    free(piv);
    free(vo);
    free(junk);
    *det=deter;
    return rank;

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

// REX rs1 a2 a2p a2np

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

// "recursive" echelize routine (to be upgraded)

#define MINROWS 300
#define MINCOLS 300

uint64_t RCEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    uint64_t colleft,colright,i,rk1,rk2,rank,rowtop,rowbottom;
    FELT det1,det2;
    const FIELD * f;
    Dfmt *a1,*a2,*m1,*c1,*r1,*a2p,*a2np;
    Dfmt *m2,*c2,*r2,*y,*temp,*rem1,*y1,*y2,*ku,*x1,*x2,*mu,*ml;
    uint64_t *rs1,*cs1,*rs2,*cs2,*rf,*cf;
    DSPACE ds1,ds2,ds3,ds4,dsrem,dsm;
    f=ds->f;
    if( (nor<=MINROWS) || (ds->noc<=MINCOLS) )
        return BCEch(ds,a,rs,cs,det,m,c,r,nor);
    memset(rs,0,16+8*((nor+63)/64));
    memset(cs,0,16+8*((ds->noc+63)/64));
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
        rk1=RCEch(&ds1,a1,rs1,cs,&det1,m1,c1,r1,nor);
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
        SLMul(f,c1,a2p,temp,nor-rk1,rk1,colright);
        DAdd(&ds2,nor-rk1,temp,a2np,a2np);
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
        rk2=RCEch(&ds2,a2np,rs2,cs2,&det2,m2,c2,r2,nor-rk1);
        free(a2np);
        rank=rk1+rk2;
        DSSet(f,rank,&dsm);
        i=16+((rank+63)/64)*8;
        rf=malloc(i); 
        BSCombine(rs1,rs2,rs,rf);
        free(rs1);
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
        SLMul(f,x1,r2,temp,rk1,rk2,colright-rk2);
        DSSet(f,colright-rk2,&ds4);
        DAdd(&ds4,rk1,x2,temp,x2);
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
        SLMul(f,c2,y,temp,nor-rank,rk2,rank);
        DAdd(&dsm,nor-rank,c,temp,c);
        // cleaner
        // Steve's code from here
        // plus I added some free's in the code above
        free(c2);
        SLMul(f,m2,y,temp,rk2,rk2,rank);
        free(m2);
        free(y);
        BSColRifZ(f,rf,rk1,m1,m);
        free(m1);
        free(rf);
        i = SLSize(f, rk1, rank);
        ku = malloc(i);
        SLMul(f,x1,temp,ku,rk1,rk2,rank);
        free(x1);
        DAdd(&dsm,rk1, m, ku,m);
        free(ku);
        DCpy(&dsm, temp, rk2, m + rk1*dsm.nob);
        free(temp);
// det = det1*det2*something
        // to here
// multiplier
        return rank;
    }
    else              // chop top/bottom
    {
        //Steve again
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
        rk1 = RCEch(ds, a, rs, cs1, &det1, m1, c1, r1, rowtop);
        a2 = a + rowtop*ds->nob;
        i = SLSize(f,rowbottom, rk1);
        a2p = malloc(i);
        i = SLSize(f, rowbottom, ds->noc - rk1);
        a2np = malloc(i);
        BSColSelect(f, cs1, rowbottom, a2, a2p, a2np);
        temp = malloc(i); // same size as a2np
        SLMul(f, a2p, r1, temp, rowbottom, rk1, ds->noc - rk1);
        DSSet(f, ds->noc - rk1, &ds2);
        DAdd(&ds2, rowbottom, a2np, temp, a2np);
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
        rk2 = RCEch(&ds2, a2np, rs2, cs2, &det2, m2, c2, r2, rowbottom);
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
        SLMul(f, x1, r2, temp, rk1, rk2, ds->noc - rank);
        DSSet(f, ds->noc - rank, &dsrem);
        DAdd(&dsrem, rk1, temp, rem1, rem1);
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
        SLMul(f,c2, y1, temp, rowbottom-rk2, rk2, rk1); // temp must be big enough
        DAdd(&ds3, rowbottom - rk2, temp, y2, y2);
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
        SLMul(f, x1, ml, temp, rk1, rk2, rank);
        free(x1);
        DAdd(&dsm, rk1, temp, mu, mu);
        free(temp);
        memset(m,0,SLSize(f, rank, rank));
        RowRif(&dsm, cf, mu, ml, m);
        free(mu);
        free(ml);
        free(cf);
        // determinant to do
        return rank;
        // end of Steve's code
    }
}

// "exported" slab echelize - needs early release etc. with proggies

uint64_t SLEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    return RCEch(ds,a,rs,cs,det,m,c,r,nor);
}


/* end of slab.c  */
