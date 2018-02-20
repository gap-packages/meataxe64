/*
         slab.c  -   Slab Routines Code
         ======      R. A. Parker 18.8.2017
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

uint64_t SLSize (FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    DSSet(f,noc,&ds);
    return ds.nob*nor;
}
uint64_t SLSizeM(FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    uint64_t rank;
    rank=nor;
    if(rank>noc) rank=noc;
    DSSet(f,rank,&ds);
    return ds.nob*rank;
}

// the next two are correct but far too pessimistic

uint64_t SLSizeC(FIELD * f, uint64_t nor, uint64_t noc)
{
    DSPACE ds;
    uint64_t rank;
    rank=nor;
    if(rank>noc) rank=noc;
    DSSet(f,rank,&ds);
// columns at most rank, rows at most nor
    return ds.nob*nor;
}

uint64_t SLSizeR(FIELD * f, uint64_t nor, uint64_t noc)
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

// "recursive" Dfmt echelize routine (not yet recursive)

uint64_t RCEchS(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor, int lev)
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

    if(lev!=1)
    {
        printf("No recursive stuff implemented yet\n");
        exit(13);
    }
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

// no separate implementation of SLEch as yet with non-standard rows

uint64_t SLEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    return RCEchS(ds,a,rs,cs,det,m,c,r,nor,1);
}

// SLEchS must return standard rows in the row select

uint64_t SLEchS(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor)
{
    return RCEchS(ds,a,rs,cs,det,m,c,r,nor,1);
}

/* end of slab.c  */
