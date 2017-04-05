/*
         slab.c  -   Slab Routines Code
         ======      R. A. Parker 8.7.2015
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
#include "bitstring.h"

void HDump1(HPMI * hp)
{
    uint16 * Thpv;
    uint8  * Thpa;
    uint64 * Thpb;
    uint8  * Thpc;
    int i;
    Thpv=(uint16 *)(((uint8 *)hp->f)+(hp->f)->Thpv);
    printf("Thpv\n");
    for(i=0;i<243;i++)
    {
        printf("%3d %x\n",i,Thpv[i]);
    }
    Thpa=((uint8 *)hp->f)+(hp->f)->Thpa;
    printf("Thpa\n");
    for(i=0;i<256;i++)
    {
        printf("%2x %d\n",i,Thpa[i]);
    }
    Thpb=(uint64 *)(((uint8 *)hp->f)+(hp->f)->Thpb);
    printf("Thpb\n");
    for(i=0;i<243;i++)
    {
        printf("%3d %lx\n",i,Thpb[i]);
    }
    Thpc=((uint8 *)hp->f)+(hp->f)->Thpc;
    printf("Thpc\n");
    for(i=0;i<1023;i++)
    {
        printf("%3x %d\n",i,Thpc[i]);
    }
}

void HDump(HPMI * hp)
{
    const FIELD * f;
    f=hp->f;
    printf(" cauldron %ld\n",(long)f->cauldron);
    printf(" cauldbytes  %ld\n",(long)f->cauldbytes);
    printf(" dfmtcauld  %ld\n",(long)f->dfmtcauld);
    printf(" alcove   %ld\n",(long)f->alcove);
    printf(" alcovebytes %ld\n",(long)f->alcovebytes);
    printf(" idealnz0 %ld\n",(long)f->idealnz0);
    printf(" maxawa   %ld\n",(long)f->maxawa);
    printf(" brickslots  %ld\n",(long)f->brickslots);
    printf(" minslab  %ld\n",(long)f->minslab);
    printf(" nora     %ld\n",(long)hp->nora);
    printf(" noca     %ld\n",(long)hp->noca);
    printf(" noba     %ld\n",(long)hp->noba);
    printf(" nocb     %ld\n",(long)hp->nocb);
    printf(" nobbc    %ld\n",(long)hp->nobbc);
    printf(" nz0      %ld\n",(long)hp->nz0);
    printf(" nz1      %ld\n",(long)hp->nz1);
    printf(" bytesnz1 %ld\n",(long)hp->bytesnz1);
    printf(" nz2      %ld\n",(long)hp->nz2);
    printf(" nz3      %ld\n",(long)hp->nz3);
    printf(" nz4      %ld\n",(long)hp->nz4);
    printf(" alen     %ld\n",(long)hp->alen);
    printf(" a        %ld\n",(long)hp->a);
    printf(" ix       %ld\n",(long)hp->ix);
    printf(" bwa      %ld\n",(long)hp->bwa);
    printf(" sparsity %ld\n",(long)hp->sparsity);
    printf("\n");
#ifdef NEVER
    HDump1(hp);
#endif
}

void PLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
          Dfmt * c, uint64 nora, uint64 noca, uint64 nocb)
{
    const Dfmt *da, *db;
    Dfmt *dc;
    uint8 * bb;
    uint8 * cc;         // Cfmt answer
    uint8 * ct;         // pointer to current cauldron
    DSPACE dsa;
    DSPACE dsb;
    uint64 i,j;
    FELT e;
    HPMI hp;
    uint64 nz0,nz1,nz2,nz3,nz4;
    uint64 z1,z2,z3,z4;
    uint64 alca;
    if(nocb==0) return;
    if(nora==0) return;
    hp.f=f;
    PSSet(f,nocb,&dsb);
    PSSet(f,noca,&dsa);
    da=a;
    db=b;
    dc=c;
    memset(dc,0,dsb.nob*nora);
    if(noca==0) return;
    if(f->hpmischeme == 0)  // no HPMI at all - ground field steam
    {
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
    hp.nora=nora;
    hp.noca=noca;
    hp.noba=dsa.nob;
    hp.nocb=nocb;
    hp.nobbc=dsb.nob;

/*  First understand how to chop the rows of A/C  */
/*      nz0 rows are done by brick mad, nz4 times */
/*      note nora is nonzero if we get here       */

        // how many set of rows down A/C.
    nz4=(nora+f->idealnz0-1)/f->idealnz0;
        // max number of rows A/C at a time
    nz0=(nora+nz4-1)/nz4;
        // apply the little theorem
    nz4=(nora+nz0-1)/nz0;

/*  Now understand how to chop the cols of A = rows of B         */
/*    alcove * nz1 (Afmt conversion size) * nz3 to make them all */

        // how many alcoves are there to do in all
    alca=(noca+f->alcove-1)/f->alcove;
        // how many alcoves would we like to do at once
    nz1=f->maxawa/(f->alcovebytes*nz0);
    nz3=(alca+nz1-1)/nz1;               // multiples we need
    nz1=(alca+nz3-1)/nz3;               // ideal size for that multiple
        // need to make sure that nz1 alcoves is an integral
        // number of bytes of Dfmt
    while(((f->alcove*nz1)%f->pentbyte)!=0) nz1--;
    if(nz1<1)
    {
        // if nz1 was zero, find its smallest positive value
        nz1++;
        while(((f->alcove*nz1)%f->pentbyte)!=0) nz1++;
    }
        // so how many chunks (nz3) do we actually need to do
    nz3=(alca+nz1-1)/nz1;

/*  Finally how many cauldrons are there.  That's easy      */
    nz2=(nocb+f->cauldron-1)/f->cauldron;

    hp.nz0=nz0;
    hp.nz1=nz1;
// following line assumes entries fit in a byte
// and that the field is a ground field
    hp.bytesnz1=(f->alcove*nz1*f->pbytesper)/f->pentbyte;
    hp.nz2=nz2;
    hp.nz3=nz3;
    hp.nz4=nz4;

/*  First job is to convert the matrix to Bfmt            */
/*  This is a no-op in characteristic 2 but some          */
/*  flags, mallocs etc will be needed here in due course  */

        // Allocate all the work areas we need
    AllocWA(&hp);
/*  If there is a Bfmt, allocate it and then convert      */
/*  matrix b into Bfmt                                    */
    bb=NULL;                  // avoid compiler warnings
    if(f->bfmt==1)
    {
        bb=AlignMalloc(f->cauldbytes*hp.nz2*hp.noca);
        DtoB(&hp, b, bb);
    }
        // Allocate and zeroize the answer Cfmt area
    cc=AlignMalloc(f->cauldbytes*hp.nz2*hp.nora);
    CZer(&hp,cc);
    for(z4=0;z4<nz4;z4++)
    {
      for(z3=0;z3<nz3;z3++)
      {
        DtoA(&hp,a,z4,z3);
        for(z2=0;z2<nz2;z2++)
        {
          ct=cc + (z4*nz0+z2*hp.nora)*f->cauldbytes;
          for(z1=0;z1<nz1;z1++)
          {
// grab the z3 z2 z1 data into the brick work area
            if(f->bfmt==0)
                DSeed(&hp,b,z3,z2,z1);
            else
                BSeed(&hp,bb,z3,z2,z1);
// populate the brick with the grease table
            BGrease(&hp);
// do the brick mad itself
            BrickMad(&hp,ct,z1);
          }
        }
      }
    }
    CtoD(&hp,cc,c);
// free all the things we used
    AlignFree(cc);
    if(f->bfmt==1) AlignFree(bb);
    FreeWA(&hp);
}

void SLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
          Dfmt * c, uint64 nora, uint64 noca, uint64 nocb)
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
    if(f->pow==1)    // maybe ground field has HPMI
    {
        if(f->hpmischeme != 0)  // HPMI usable
        {
            PLMul(f,a,b,c,nora,noca,nocb);
            return;
        }
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
           uint64 naru, uint64 nacu)
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

        f8=(uint8 *)f;
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

        f8=(uint8 *)f;
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

        f8=(uint8 *)f;
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

        f8=(uint8 *)f;
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

        f8=(uint8 *)f;
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

extern uint64 SLEch(const FIELD *f, Dfmt *a, uint64 *rs, uint64 *cs,
                    Dfmt *m, Dfmt *c, Dfmt *r, uint64 nor, uint64 noc)
{
    int * piv;
    uint64 fel;
    uint64 nck,rank,i,j,z,col;
    size_t sbsr, sbsc;
    DSPACE dsa,dsk,dsm,dsr;
    Dfmt *vo, *junk;
    Dfmt *k;
    Dfmt *va,*vk;

    k=r;    // use remnant area for keeptrack
    piv=malloc(nor*sizeof(int));
    DSSet(f,noc,&dsa);
    vo=malloc(dsa.nob);
    junk=malloc(dsa.nob);
    nck=nor;
    if(nck>noc) nck=noc;
    DSSet(f,nck,&dsk);
    memset(m,0,nck*dsk.nob);
    memset(k,0,nor*dsk.nob);   // keeptrack starts as zero

    sbsr=8*(2+(nor+63)/64);
    memset(rs,0,sbsr);
    rs[0]=nor;
    sbsc=8*(2+(noc+63)/64);
    memset(cs,0,sbsc);
    cs[0]=noc;

    rank=0;

    for(i=0;i<nor;i++)
    {
        va=a+i*dsa.nob;
        vk=k+i*dsk.nob;
        col=DNzl(&dsa,va);
        if(col==ZEROROW) continue;
        piv[i]=col;
        fel=DUnpak(&dsa,col,va);
        fel=FieldInv(f,fel);
        fel=FieldNeg(f,fel);
        DSMul(&dsa,fel,1,va);
        DPak(&dsk,rank,vk,1);
        DSMul(&dsk,fel,1,vk);
        BSBitSet(rs,i);
        BSBitSet(cs,col);
        rank++;
        for(j=0;j<nor;j++)
        {
            if(j==i) continue;
            fel=DUnpak(&dsa,col,a+j*dsa.nob);
            DSMad(&dsa,fel,1,va,a+j*dsa.nob);
            DSMad(&dsk,fel,1,vk,k+j*dsk.nob);
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
    for(i=0;i<noc;i++)     // by permuting the rows of keeptrack
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
    DSSet(f,noc-rank,&dsr);
    memset(r,0,(nor-rank)*dsr.nob);
    z=0;                   // finally the remnant
    for(i=0;i<noc;i++)     // by permuting the rows of matrix a
    {
        if(BSBitRead(cs,i)==0) continue;
        for(j=0;j<nor;j++)
        {
            if(BSBitRead(rs,j)==0) continue;
            if(piv[j]!=i) continue;
            BSColSelect(f,cs,1,a+j*dsa.nob,junk,r+z*dsr.nob);
            z++;
// I think this can break
        }
    }

    free(piv);
    free(vo);
    free(junk);
    return rank;

}

/* end of slab.c  */
