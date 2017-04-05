/*   hpmi.c                       */
/*   R. A. Parker 20.7.2015       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"

// DtoBfmt will be needed for other characteristics

void AllocWA(HPMI * hp)
{
    const FIELD * f;

    f=hp->f;
    hp->alen=1+hp->nz0*f->alcovebytes;
    if((hp->f)->hpmischeme!=4)
        hp->a=malloc(hp->alen*hp->nz1+8);     // to allow for extra long Afmt loads
    else
        hp->a64=malloc(8*hp->nz0*hp->nz1+8);
// possibly ought to memset hp->a/a64 for valgrind
    hp->ix=malloc(hp->nz1*sizeof(uint64));
    hp->bwa=AlignMalloc(f->brickslots*f->cauldbytes);
    if(((hp->f)->charc)==3)
    {
        memset(hp->bwa,0xff,f->brickslots*f->cauldbytes);
        return;
    }
    memset(hp->bwa,0,f->brickslots*f->cauldbytes);
}

void FreeWA(HPMI * hp)
{
    free(hp->a);
    free(hp->ix);
    AlignFree(hp->bwa);
}

void hpmitab(FIELD * f)
{
    f->hpmischeme=0;
    f->cauldron=0;
    if(f->charc==2)
    {
        f->hpmischeme=2;
        if(f->pcgen==0) f->cauldron=512;
            else        f->cauldron=1024;
        f->cauldbytes=f->cauldron/8;
        f->dfmtcauld=f->cauldbytes;
        f->alcove=32;
        f->alcovebytes=5;
        f->idealnz0=1024;
        if(f->pcgen==1)
          if(f->pcsubgen==2) f->idealnz0=6000;
        f->maxawa=500000;
        f->brickslots=128;
        f->minslab=8192;
        f->bfmt=0;
    }
    if((f->charc==3) && (f->pcgen!=0) ) // no char 3 in GEN
    {
        f->hpmischeme=3;
        f->cauldron=510;
        f->cauldbytes=128;
        f->dfmtcauld=f->cauldron/5;
        f->alcove=12;
        f->alcovebytes=4;
        f->idealnz0=1024;
        f->maxawa=500000;
        f->brickslots=123;
        f->minslab=5100;
        f->bfmt=1;
    }
#ifdef SOON
    if( (f->charc>16) && (f->charc<256) && (f->pcgen==2) )
    {
        f->hpmischeme=4;
        f->cauldron=128;
        f->cauldbytes=256;
        f->dfmtcauld=256;
        f->alcove=4;
        f->alcovebytes=8;
        f->idealnz0=512;
        f->maxawa=50000;
        f->brickslots=64;
        f->minslab=1024;
        f->phs[0]=f->charc;
        f->phs[1]=(100*f->charc-8192)%f->charc;
        f->phs[2]=4*f->charc;
// make Afmt conversion table
// make BrickPop addition chain table
    }
#endif
    return;
}

void DtoA(HPMI * hp, const Dfmt * a, uint64 z4, uint64 z3)
{
    int i,j,k;
    uint8 orc,byte,byte1;
    const uint8 *pt1,*pt2;
    int startrow;
    int startbyte,s;
    int bits,nbits;
    uint8  * Thpa;
    uint16 * Thpv;
    const FIELD * f;
    uint8_t * f8;
    f=hp->f;
    f8=(uint8_t *)f;

    startrow=z4*hp->nz0;
    startbyte=z3*hp->bytesnz1;
    pt1=a+startrow*hp->noba+startbyte;
    if(f->hpmischeme==2)
    {
        for(j=0;j<hp->nz1;j++)
        {
            hp->ix[j]=j*hp->alen;
            hp->a[j*hp->alen]=0;  // skip no rows at the start.
        }
        for(i=0;i<hp->nz0;i++)  // for each row of the matrix
        {
            if(i+startrow>=hp->nora) break;
            for(j=0;j<hp->nz1;j++)   // j indexes the alcove
            {
                orc=0;               // OR of the bytes
                pt2=pt1+4*j;         // point to block of 4 bytes
                for(k=1;k<=4;k++)    // for each byte of block
                {
                    if( (startbyte+4*j+k) > hp->noba) byte=0;
                                  else               byte=*pt2;
                    orc|=byte;      // OR in the byte for sparsity
                    hp->a[hp->ix[j]+k]=byte;  // and move it into Afmt
                    pt2++;
                }
// Afmt up to 250 (251-254 spare) 255 is terminator
                if( (orc==0) && (hp->a[hp->ix[j]]<250) )  // sparsity
                    hp->a[hp->ix[j]]++;                   // skip one more row
                else
                {
                    hp->ix[j]+=5;               // keep that block
                    hp->a[hp->ix[j]]=1;         // advance pointer
                }
            }
            pt1+=hp->noba;
        }
        for(j=0;j<hp->nz1;j++)
            hp->a[hp->ix[j]]=255;  // Put in all the terminators
    }
    if(f->hpmischeme==3)
    {
        for(j=0;j<hp->nz1;j++)
        {
            hp->ix[j]=j*hp->alen;
            hp->a[j*hp->alen]=0;  // skip no rows at the start.
        }
        Thpa=f8+f->Thpa;
        Thpv=(uint16 *)(f8+f->Thpv);
        bits=0;
        for(i=0;i<hp->nz0;i++)  // for each row of matrix A
        {
            if(i+startrow>=hp->nora) break;
            nbits=0;
            s=0;
            for(j=0;j<hp->nz1;j++)   // j indexes the alcove
            {
                orc=0;
                for(k=1;k<=3;k++)     // k indexes slice within alcove
                {
                    if(nbits<8)
                    {
                        if( (startbyte+s) >= hp->noba) byte=0;
                                  else                 byte=pt1[s];
                        bits=bits+(Thpv[byte]<<nbits);
                        nbits+=10;
                        s++;
                    }
                    byte1=bits&255;
                    orc|=byte1;
                    hp->a[hp->ix[j]+k]=Thpa[byte1];  // compute and output Afmt
                    bits=(bits>>8);
                    nbits-=8;
                }
// Afmt up to 250 (251-254 spare) 255 is terminator
                if( (orc==0) && (hp->a[hp->ix[j]]<250) )  // sparsity
                    hp->a[hp->ix[j]]++;                   // skip one more row
                else
                {
                    hp->ix[j]+=4;               // keep that block
                    hp->a[hp->ix[j]]=1;         // advance pointer
                }
            }
            pt1+=hp->noba;
        }
        for(j=0;j<hp->nz1;j++)
            hp->a[hp->ix[j]]=255;  // Put in the terminators
    }
}

void DtoB(HPMI * hp, const Dfmt * a, uint8 * b)
{
    int i,j,k,nob;
    uint8 d;
    uint64 m,mu,mt;
    uint64 * Thpb;
    int ubits,tbits,nbits;
    uint8 *bu,*bt,*zbu,*zbt;
    const uint8 * ad;
    const FIELD * f;

    f=hp->f;
    if(((hp->f)->charc)==3)
    {
        Thpb=(uint64 *)(((uint8 *)hp->f)+(hp->f)->Thpb);
        for(j=0;j<hp->nz2;j++)      // ready for fastpath
        {
            for(i=0;i<hp->noca;i++)
            {
                bu=b+(j*hp->noca+i)*f->cauldbytes;
                bt=bu+(f->cauldbytes/2);
                zbu=bu+31;
                zbt=bt+31;
                nbits=0;
                ubits=0;
                tbits=0;
                nob=hp->nobbc-(j*f->dfmtcauld);
                if(nob>f->dfmtcauld) nob=f->dfmtcauld;
                ad=a+i*hp->nobbc+j*f->dfmtcauld;
                for(k=0;k<f->dfmtcauld;k++)
                {
                    if(k>=nob) d=0;
                    else d=*(ad+k);
                    m=Thpb[d];
                    mu=(m>>32)&0x1f;
                    mt=m&0x1f;
                    ubits=ubits+(mu<<nbits);
                    tbits=tbits+(mt<<nbits);
                    nbits+=5;
                    if((nbits>=8)||((k%51)==50))
                    {
                        *(bu++)=ubits&0xff;
                        *(bt++)=tbits&0xff;
                        ubits=(ubits>>8);
                        tbits=(tbits>>8);
                        nbits-=8;
                        if(nbits<0) nbits=0;
                    }
                }
                for(k=31;k<f->cauldbytes;k+=32)
                {
                    *zbu |= 0x80;
                    *zbt |= 0x80;
                    zbu+=32;
                    zbt+=32;
                }
            }
        }
    }
}

// Seed from a Dfmt input
void DSeed(HPMI * hp, const Dfmt * b, uint64 z3, uint64 z2, uint64 z1)
{
    int zlen,dlen;
    int i,j,k;
    uint64 rix;
    Dfmt sp;
    const Dfmt *pt0,*pt1;
    Dfmt *pt2;
    const FIELD * f;
    
    f=hp->f;

    if(f->hpmischeme==2)
    {
        zlen=((z2+1)*f->cauldron);
        zlen-=hp->nocb;
        if(zlen<0) zlen=0;
        if(zlen>f->cauldron) zlen=f->cauldron;
        zlen=zlen/8;    // convert into bytes to zeroize
        dlen=f->cauldbytes-zlen;  // data length to move
        rix=(z3*hp->nz1 + z1)*f->alcove;   // initialize row index
        sp=0;   // sparsity check byte
        pt0=b+rix*hp->nobbc+z2*f->cauldbytes;
        if( ((rix+32)<hp->noca) && (dlen==f->cauldbytes) )
        {
            if( (*((uint64*)pt0)) != 0 )
            {
                for(i=0;i<8;i++)
                {
                    for(j=0;j<4;j++)
                    {
                        pt1=pt0;
                        pt2=hp->bwa+(i*16+(1<<j))*f->cauldbytes;
                        memcpy(pt2,pt1,dlen);
                        pt0+=hp->nobbc;
                        rix++;
                    }
                }
                hp->sparsity=1;
                return;
            }
        }
        for(i=0;i<8;i++)
        {
            for(j=0;j<4;j++)
            {
                pt1=pt0;
                pt2=hp->bwa+(i*16+(1<<j))*f->cauldbytes;
                if(rix<hp->noca)
                {
                    for(k=0;k<dlen;k++)
                    {
                        sp|=*(pt1);
                        *(pt2++) = *(pt1++);
                    }
                    for(k=dlen;k<f->cauldbytes;k++)   *(pt2++)=0;
                }
                else memset(pt2,0,f->cauldbytes);
                pt0+=hp->nobbc;
                rix++;
            }
        }
        if(sp==0) hp->sparsity=0;
         else     hp->sparsity=1;
        return;
    }
    printf("DSeed called with f->hpmischeme == %d\n",f->hpmischeme);
    exit(13);
}

// Seed from a Bfmt input assumes characteristic 3

uint64 cauldcpy(HPMI * hp, uint8 * a, const uint8 * b)
{
    uint64 *aa,*bb;
    uint64 annd;
    int i;
    const FIELD * f;

    f=hp->f;
    annd=0xFFFFFFFFFFFFFFFF;
    aa=(uint64 *)a;
    bb=(uint64 *)b;
    for(i=0;i<f->cauldbytes/8;i++)  annd &= (aa[i]=bb[i]);
    return annd;
}

void BSeed(HPMI * hp, const uint8 * bb, uint64 z3, uint64 z2, uint64 z1)
{
    uint64 i,j;
    uint64 annd;
    const FIELD * f;

    f=hp->f;
    annd=0xFFFFFFFFFFFFFFFF;
    for(i=0;i<3;i++)
    {
// memcpy dest src siz
        j=(z3*hp->nz1+z1)*f->alcove+4*i;
        if(j<hp->noca)
            annd &= cauldcpy(hp,hp->bwa+(41*i+1)*f->cauldbytes,
                bb+(z2*hp->noca+j)*f->cauldbytes);
        else memset(hp->bwa+(41*i+1)*f->cauldbytes,0xff,f->cauldbytes);
        j++;
        if(j<hp->noca)
            annd &= cauldcpy(hp,hp->bwa+(41*i+2)*f->cauldbytes,
                bb+(z2*hp->noca+j)*f->cauldbytes);
        else memset(hp->bwa+(41*i+2)*f->cauldbytes,0xff,f->cauldbytes);
        j++;
        if(j<hp->noca)
            annd &= cauldcpy(hp,hp->bwa+(41*i+5)*f->cauldbytes,
                bb+(z2*hp->noca+j)*f->cauldbytes);
        else memset(hp->bwa+(41*i+5)*f->cauldbytes,0xff,f->cauldbytes);
        j++;
        if(j<hp->noca)
            annd &= cauldcpy(hp,hp->bwa+(41*i+14)*f->cauldbytes,
                bb+(z2*hp->noca+j)*f->cauldbytes);
        else memset(hp->bwa+(41*i+14)*f->cauldbytes,0xff,f->cauldbytes);
    }
    if(annd==0xFFFFFFFFFFFFFFFF) hp->sparsity=0;
    else hp->sparsity=1;
}

void BGrease(HPMI * hp)
{
    int i,r;
    Dfmt * bw;
    const FIELD * f;

    f=hp->f;
    if(hp->sparsity==0) return;
    bw=hp->bwa;
    r=f->cauldbytes;
    if(((hp->f)->charc)==2)
    {
        for(i=0;i<8;i++)
        {
            pcxor(bw+ 3*r,bw+2*r,bw+1*r,r);
            pcxor(bw+ 5*r,bw+4*r,bw+1*r,r);
            pcxor(bw+ 6*r,bw+4*r,bw+2*r,r);
            pcxor(bw+ 7*r,bw+4*r,bw+3*r,r);
            pcxor(bw+ 9*r,bw+8*r,bw+1*r,r);
            pcxor(bw+10*r,bw+8*r,bw+2*r,r);
            pcxor(bw+11*r,bw+8*r,bw+3*r,r);
            pcxor(bw+12*r,bw+8*r,bw+4*r,r);
            pcxor(bw+13*r,bw+8*r,bw+5*r,r);
            pcxor(bw+14*r,bw+8*r,bw+6*r,r);
            pcxor(bw+15*r,bw+8*r,bw+7*r,r);
            bw+=16*r;
        }
        return;
    }
    if(((hp->f)->charc)==3)
    {
        for(i=0;i<3;i++)
        {
            pcad3(bw+ 1*r,bw+ 2*r,bw+ 3*r);
            pcad3(bw+ 1*r,bw+ 3*r,bw+ 4*r);
            pcad3(bw+ 1*r,bw+ 5*r,bw+ 6*r);
            pcad3(bw+ 1*r,bw+ 6*r,bw+ 7*r);
            pcad3(bw+ 2*r,bw+ 5*r,bw+ 8*r);
            pcad3(bw+ 1*r,bw+ 8*r,bw+ 9*r);
            pcad3(bw+ 1*r,bw+ 9*r,bw+10*r);
            pcad3(bw+ 2*r,bw+ 8*r,bw+11*r);
            pcad3(bw+ 1*r,bw+11*r,bw+12*r);
            pcad3(bw+ 1*r,bw+12*r,bw+13*r);
            pcad3(bw+ 1*r,bw+14*r,bw+15*r);
            pcad3(bw+ 1*r,bw+15*r,bw+16*r);
            pcad3(bw+ 2*r,bw+14*r,bw+17*r);
            pcad3(bw+ 1*r,bw+17*r,bw+18*r);
            pcad3(bw+ 1*r,bw+18*r,bw+19*r);
            pcad3(bw+ 2*r,bw+17*r,bw+20*r);
            pcad3(bw+ 1*r,bw+20*r,bw+21*r);
            pcad3(bw+ 1*r,bw+21*r,bw+22*r);
            pcad3(bw+ 5*r,bw+14*r,bw+23*r);
            pcad3(bw+ 1*r,bw+23*r,bw+24*r);
            pcad3(bw+ 1*r,bw+24*r,bw+25*r);
            pcad3(bw+ 2*r,bw+23*r,bw+26*r);
            pcad3(bw+ 1*r,bw+26*r,bw+27*r);
            pcad3(bw+ 1*r,bw+27*r,bw+28*r);
            pcad3(bw+ 2*r,bw+26*r,bw+29*r);
            pcad3(bw+ 1*r,bw+29*r,bw+30*r);
            pcad3(bw+ 1*r,bw+30*r,bw+31*r);
            pcad3(bw+ 5*r,bw+23*r,bw+32*r);
            pcad3(bw+ 1*r,bw+32*r,bw+33*r);
            pcad3(bw+ 1*r,bw+33*r,bw+34*r);
            pcad3(bw+ 2*r,bw+32*r,bw+35*r);
            pcad3(bw+ 1*r,bw+35*r,bw+36*r);
            pcad3(bw+ 1*r,bw+36*r,bw+37*r);
            pcad3(bw+ 2*r,bw+35*r,bw+38*r);
            pcad3(bw+ 1*r,bw+38*r,bw+39*r);
            pcad3(bw+ 1*r,bw+39*r,bw+40*r);
            bw+=41*r;
        }
        return;
    }
}
void BrickMad(HPMI * hp, uint8 * c, uint64 z1)
{
    if(hp->sparsity==0) return;
    if(((hp->f)->charc)==2) pcbm2(hp->a+z1*hp->alen,hp->bwa,c);
    if(((hp->f)->charc)==3) pcbm3(hp->a+z1*hp->alen,hp->bwa,c);
}


void CZer(HPMI * hp, uint8 * c)
{
    uint64 i;
    const FIELD * f;
    uint16_t * c16;

    f=hp->f;
    if(f->hpmischeme==2)
    {
        memset(c,0,f->cauldbytes*hp->nz2*hp->nora);
        return;
    }
    if(f->hpmischeme==3)
    {
        for(i=0;i<f->cauldbytes*hp->nz2*hp->nora;i++) c[i]=0xff;
        return;
    }
    if(f->hpmischeme==4)
    {
        c16=(uint16_t *)c;
        for(i=0;i<f->cauldron*hp->nz2*hp->nora;i++) c16[i]=f->phs[2];
    }
}

void CtoD(HPMI * hp, uint8 * cc, Dfmt * c)
{
    int zlen,cpylen,z2,z0,bits,ix,i;
    uint8 * sp;
    Dfmt * dp;
    Dfmt dbyte;
    uint64 bu,bt;
    uint8 * Thpc;
    const FIELD * f;

    f=hp->f;

    bu=0;    // stop compiler
    bits=0;
    bt=0;    // warnings
    Thpc=(uint8 *)(hp->f) + ((hp->f)->Thpc);
    for(z2=0;z2<hp->nz2;z2++)
    {
        zlen=((z2+1)*f->cauldron);
        zlen-=hp->nocb;
        if(zlen<0) zlen=0;
        if(zlen>f->cauldron) zlen=f->cauldron;
        if((hp->f)->charc==2) zlen=zlen/8;    // convert into bytes to ignore
        if((hp->f)->charc==3) zlen=zlen/5;  
        cpylen=f->dfmtcauld-zlen;
        for(z0=0;z0<hp->nora;z0++)
        {
            dp=c+z0*hp->nobbc+z2*f->dfmtcauld;
            sp=cc+z0*f->cauldbytes+z2*hp->nora*f->cauldbytes;
            bu=0;
            bits=0;
            bt=0;
            if(((hp->f)->charc)==2)
                   memcpy(dp,sp,cpylen);
            if(((hp->f)->charc)==3)
            {
                for(i=0;i<cpylen;i++)
                {
                    if((i%51)==0)
                    {
                        bits=0;
                        bu=0;
                        bt=0;
                    }
                    if(bits<5)
                    {
                        bu=bu+((*sp)<<bits);
                        bt=bt+((*(sp+(f->cauldbytes/2)))<<bits);
                        sp++;
                        bits+=8;
                    }
                    ix=((bu&0x1F)<<5)+(bt&0x1f);
                    bits-=5;
                    bu=bu>>5;
                    bt=bt>>5;
                    dbyte=Thpc[ix];
                    dp[i]=dbyte;
                }
            }
        }
    }
}

/* end of hpmi.c  */
