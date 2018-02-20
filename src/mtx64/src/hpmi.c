// Copyright (C) Richard Parker   2017
//  hpmi.c Meataxe64 Nikolaus version

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"

void DtoA(DSPACE * ds1, uint64_t * ix, const Dfmt * d, Afmt * a,
            uint64_t nora, uint64_t noca)
{
    int i,j,k;
    uint64_t nz1;
    uint8_t orc,byte,byte1;
    const uint8_t *pt1,*pt2;
    int s;
    int bits,nbits;
    int copybytes;
    uint8_t  * Thpa;
    uint16_t * Thpv;
    uint64_t alen;
    DSPACE ds;
    const FIELD * f;

    uint8_t * f8;
    f=ds1->f;
    PSSet(f,noca,&ds);
    f8=(uint8_t *)f;
    alen=1+nora*f->alcovebytes;
    copybytes=f->alcovebytes-1;
    pt1=d;
    nz1=(noca+f->alcove-1)/f->alcove;
    for(j=0;j<nz1;j++)
    {
        ix[j]=j*alen;
        a[j*alen]=0;  // skip no rows at the start.
    }
    Thpa=f8+f->Thpa;
    if( (f->AfmtMagic==1)||(f->AfmtMagic==2) )
    {
        for(i=0;i<nora;i++)  // for each row of the matrix
        {
            for(j=0;j<nz1;j++)   // j indexes the alcove
            {
                orc=0;               // OR of the bytes
                pt2=pt1+copybytes*j;         // point to block of bytes
                for(k=1;k<=copybytes;k++)    // for each byte of block
                {
                    if( (copybytes*j+k) > ds.nob) byte=0;
                                  else            byte=*pt2;
                    orc|=byte;      // OR in the byte for sparsity
                    if(f->AfmtMagic==2) byte=Thpa[byte];
                    a[ix[j]+k]=byte;  // and move it into Afmt
                    pt2++;
                }
// Afmt up to 250 (251-254 spare) 255 is terminator
                if( (orc==0) && (a[ix[j]]<250) )  // sparsity
                    a[ix[j]]++;                   // skip one more row
                else
                {
                    ix[j]+=f->alcovebytes;  // keep that block
                    a[ix[j]]=1;             // advance pointer
                }
            }
            pt1+=ds1->nob;
        }

    }
    if(f->AfmtMagic==3)
    {

        Thpv=(uint16_t *)(f8+f->Thpv);
        bits=0;
        for(i=0;i<nora;i++)  // for each row of matrix A
        {
            nbits=0;
            s=0;
            for(j=0;j<nz1;j++)   // j indexes the alcove
            {
                orc=0;
                for(k=1;k<=3;k++)     // k indexes slice within alcove
                {
                    if(nbits<8)
                    {
                        if( s >= ds.nob) byte=0;
                        else             byte=pt1[s];
                        bits=bits+(Thpv[byte]<<nbits);
                        nbits+=10;
                        s++;
                    }
                    byte1=bits&255;
                    orc|=byte1;
                    a[ix[j]+k]=Thpa[byte1];  // compute and output Afmt
                    bits=(bits>>8);
                    nbits-=8;
                }
// Afmt up to 250 (251-254 spare) 255 is terminator
                if( (orc==0) && (a[ix[j]]<250) )  // sparsity
                    a[ix[j]]++;                   // skip one more row
                else
                {
                    ix[j]+=4;               // keep that block
                    a[ix[j]]=1;         // advance pointer
                }
            }
            pt1+=ds1->nob;
        }

    }
    for(j=0;j<nz1;j++)
        a[ix[j]]=255;  // Put in the terminators
}

uint64_t DtoB(DSPACE * ds1, const Dfmt * d, Bfmt * b, 
              uint64_t nor, uint64_t noc)
{
    uint64_t i,k;
    const Dfmt *pt0, *pt1, *ad;
    Dfmt xd;
    Bfmt *pt2;
    Bfmt sp;
    DSPACE ds;
    uint64_t * Thpb;
    int ubits,tbits,nbits;
    uint8_t *bu,*bt;
    uint64_t m,mu,mt;
    const FIELD * f;

    f=ds1->f;
    pt0=d;
    PSSet(f,noc,&ds);
    if(f->BfmtMagic==1)
    {
        sp=0;
        for(i=0;i<f->alcove;i++)
        {
            pt1=pt0;
            pt2=b+i*f->bfmtcauld+1;
            if(i<nor)
            {
                for(k=0;k<ds.nob;k++)
                {
                    sp|=*(pt1);
                    *(pt2++) = *(pt1++);
                }
                for(k=ds.nob;k<f->bfmtcauld;k++)   *(pt2++)=0;
            }
            else memset(pt2,0,f->bfmtcauld);
            pt0+=ds1->nob;
        }
        if(sp==0)
        {
            *b=0;
            return 1;
        }
        else
        {
            *b=1;
            return f->alcove*f->bfmtcauld+1;
        }
    }
    if(f->BfmtMagic==3)
    {
        sp=0xff;
        Thpb=(uint64_t *)(((uint8_t *)f)+f->Thpb);
        for(i=0;i<12;i++)
        {
            bu=b+i*f->bfmtcauld+1;
            bt=bu+(f->bfmtcauld/2);
            if(i>=nor)
            {
                memset(bu,0xff,f->bfmtcauld);
                continue;
            }
            nbits=0;
            ubits=0;
            tbits=0;
            ad=d+i*ds1->nob;
            for(k=0;k<f->dfmtcauld;k++)
            {
                if(k>=ds.nob) xd=0;
                else xd=*(ad+k);
                m=Thpb[xd];
                mu=(m>>32)&0x1f;
                mt=m&0x1f;
                ubits=ubits+(mu<<nbits);
                tbits=tbits+(mt<<nbits);
                nbits+=5;
                if((k%51)==50)
                {
                    ubits|=0x80;
                    tbits|=0x80;
                }
                if((nbits>=8)||((k%51)==50))
                {
                    *(bu++)=ubits&0xff;
                    *(bt++)=tbits&0xff;
                    sp&=ubits&0xff;
                    sp&=tbits&0xff;
                    ubits=(ubits>>8);
                    tbits=(tbits>>8);
                    nbits-=8;
                    if(nbits<0) nbits=0;
                }
            }
        }
        if(sp==0xff)
        {
            *b=0;
            return 1;
        }
        else
        {
            *b=1;
            return 12*f->bfmtcauld+1;
        }
    }
    return 0;    // should never happen - compiler warning
}

int BSeed(const FIELD * f, uint8_t * bwa, Bfmt * b)
{
    int i,j,x;
    uint64_t y,z;
    Bfmt *pt1,*pt2;
    uint16_t * pt3;
    uint32_t * pt4;
    if((*b)==0) return 0;
    pt1=b+1;

    if(f->SeedMagic==1)    // uint8_t Bfmt -> uint16_t BWA
    {
        pt3=(uint16_t *) bwa;
        pt3+=f->cauldron;
        for(i=0;i<f->parms[6];i++)
        {
            for(j=0;j<f->cauldron;j++)
                *(pt3+j) = *(pt1++);   // convert uint8_t to uint16_t
            pt3+=f->cauldron*f->parms[5];
        }
    }
    if(f->SeedMagic==2)    // characteristic 2
    {
        for(i=0;i<8;i++)
        {
            for(j=0;j<4;j++)
            {
                pt2=bwa+(i*16+(1<<j))*f->bfmtcauld;
                memcpy(pt2,pt1,f->bfmtcauld);
                pt1+=f->bfmtcauld;
            }
        }
    }
    if(f->SeedMagic==3)      // characteristic 3
    {
        for(i=0;i<3;i++)
        {
            memcpy(bwa+(41*i+1)*f->bfmtcauld,pt1,f->bfmtcauld);
            pt1+=f->bfmtcauld;
            memcpy(bwa+(41*i+2)*f->bfmtcauld,pt1,f->bfmtcauld);
            pt1+=f->bfmtcauld;
            memcpy(bwa+(41*i+5)*f->bfmtcauld,pt1,f->bfmtcauld);
            pt1+=f->bfmtcauld;
            memcpy(bwa+(41*i+14)*f->bfmtcauld,pt1,f->bfmtcauld);
            pt1+=f->bfmtcauld;
        }
    }
    if(f->SeedMagic==4)      // 1-byte Dfmt to 10 bit field.
    {
        x=f->cauldron;
        x=x/3;
        pt4=(uint32_t *) bwa;
        pt4+=x;
        for(i=0;i<f->parms[6];i++)
        {
            for(j=0;j<x;j++)
            {
                *(pt4+j)=(*pt1)+((*(pt1+1))<<10)+((*(pt1+2))<<20);
                pt1+=3;
            }
            pt4+=x*f->parms[5];
        }
    }
    if(f->SeedMagic==5)    // characteristic 7,11,13
    {
        x=f->bfmtcauld;
        for(i=0;i<f->parms[6];i++)
        {
            pt2=bwa+(i*f->parms[5]+1)*f->cfmtcauld;
            for(j=0;j<x;j++)
            {
                y=*(pt1++);
                z=(y*f->bar48)>>48;    // maybe better with lookup table
                pt2[1]=z;
                pt2[0]=y-z*f->charc;
                pt2+=2;
            }
            pt2=bwa+(i*f->parms[5]+2)*f->cfmtcauld;
            for(j=0;j<x;j++)
            {
                y=*(pt1++);
                z=(y*f->bar48)>>48;
                pt2[1]=z;
                pt2[0]=y-z*f->charc;
                pt2+=2;
            }
        }
    }
    if(f->SeedMagic==6)    // characteristic 5
    {
        for(i=0;i<f->parms[6];i++)
        {
            pt2=bwa+(i*f->parms[5]+1)*f->cfmtcauld;
            for(j=0;j<f->bfmtcauld;j++)
            {
                y=*(pt1++);
                z=(y*f->bar48)>>48;   // z=y/5
                x=(z*f->bar48)>>48;   // x=y/25
                pt2[2]=x;
                pt2[1]=z-x*f->charc;
                pt2[0]=y-z*f->charc;
                pt2+=3;
            }
            pt2=bwa+(i*f->parms[5]+2)*f->cfmtcauld;
            for(j=0;j<f->bfmtcauld;j++)
            {
                y=*(pt1++);
                z=(y*f->bar48)>>48;   // z=y/5
                x=(z*f->bar48)>>48;   // x=y/25
                pt2[2]=x;
                pt2[1]=z-x*f->charc;
                pt2[0]=y-z*f->charc;
                pt2+=3;
            }
            pt2=bwa+(i*f->parms[5]+3)*f->cfmtcauld;
            for(j=0;j<f->bfmtcauld;j++)
            {
                y=*(pt1++);
                z=(y*f->bar48)>>48;   // z=y/5
                x=(z*f->bar48)>>48;   // x=y/25
                pt2[2]=x;
                pt2[1]=z-x*f->charc;
                pt2[0]=y-z*f->charc;
                pt2+=3;
            }
        }
    }
    return *b;
}

void BGrease(const FIELD * f, uint8_t * bwa, int sparsity)
{
    int i,r;
    if(sparsity==0) return;
    r=f->cfmtcauld;
    if((f->GreaseMagic)==1)
    {
        pcdasc(f->prog,bwa,f->parms);
        return;
    }
    if((f->GreaseMagic)==2)
    {
// this should use addition-chain technology
        if(f->mact[0]>='j')
        {
            for(i=0;i<8;i++)
            {
                pcjxor(bwa+ 3*r,bwa+2*r,bwa+1*r,r);
                pcjxor(bwa+ 5*r,bwa+4*r,bwa+1*r,r);
                pcjxor(bwa+ 6*r,bwa+4*r,bwa+2*r,r);
                pcjxor(bwa+ 7*r,bwa+4*r,bwa+3*r,r);
                pcjxor(bwa+ 9*r,bwa+8*r,bwa+1*r,r);
                pcjxor(bwa+10*r,bwa+8*r,bwa+2*r,r);
                pcjxor(bwa+11*r,bwa+8*r,bwa+3*r,r);
                pcjxor(bwa+12*r,bwa+8*r,bwa+4*r,r);
                pcjxor(bwa+13*r,bwa+8*r,bwa+5*r,r);
                pcjxor(bwa+14*r,bwa+8*r,bwa+6*r,r);
                pcjxor(bwa+15*r,bwa+8*r,bwa+7*r,r);
                bwa+=16*r;
            }
            return;
        }
        for(i=0;i<8;i++)
        {
            pcaxor(bwa+ 3*r,bwa+2*r,bwa+1*r,r);
            pcaxor(bwa+ 5*r,bwa+4*r,bwa+1*r,r);
            pcaxor(bwa+ 6*r,bwa+4*r,bwa+2*r,r);
            pcaxor(bwa+ 7*r,bwa+4*r,bwa+3*r,r);
            pcaxor(bwa+ 9*r,bwa+8*r,bwa+1*r,r);
            pcaxor(bwa+10*r,bwa+8*r,bwa+2*r,r);
            pcaxor(bwa+11*r,bwa+8*r,bwa+3*r,r);
            pcaxor(bwa+12*r,bwa+8*r,bwa+4*r,r);
            pcaxor(bwa+13*r,bwa+8*r,bwa+5*r,r);
            pcaxor(bwa+14*r,bwa+8*r,bwa+6*r,r);
            pcaxor(bwa+15*r,bwa+8*r,bwa+7*r,r);
            bwa+=16*r;
        }
        return;
    }
    if((f->GreaseMagic)==3)
    {
        for(i=0;i<3;i++)
        {
            pcad3(bwa+ 1*r,bwa+ 2*r,bwa+ 3*r);
            pcad3(bwa+ 1*r,bwa+ 3*r,bwa+ 4*r);
            pcad3(bwa+ 1*r,bwa+ 5*r,bwa+ 6*r);
            pcad3(bwa+ 1*r,bwa+ 6*r,bwa+ 7*r);
            pcad3(bwa+ 2*r,bwa+ 5*r,bwa+ 8*r);
            pcad3(bwa+ 1*r,bwa+ 8*r,bwa+ 9*r);
            pcad3(bwa+ 1*r,bwa+ 9*r,bwa+10*r);
            pcad3(bwa+ 2*r,bwa+ 8*r,bwa+11*r);
            pcad3(bwa+ 1*r,bwa+11*r,bwa+12*r);
            pcad3(bwa+ 1*r,bwa+12*r,bwa+13*r);
            pcad3(bwa+ 1*r,bwa+14*r,bwa+15*r);
            pcad3(bwa+ 1*r,bwa+15*r,bwa+16*r);
            pcad3(bwa+ 2*r,bwa+14*r,bwa+17*r);
            pcad3(bwa+ 1*r,bwa+17*r,bwa+18*r);
            pcad3(bwa+ 1*r,bwa+18*r,bwa+19*r);
            pcad3(bwa+ 2*r,bwa+17*r,bwa+20*r);
            pcad3(bwa+ 1*r,bwa+20*r,bwa+21*r);
            pcad3(bwa+ 1*r,bwa+21*r,bwa+22*r);
            pcad3(bwa+ 5*r,bwa+14*r,bwa+23*r);
            pcad3(bwa+ 1*r,bwa+23*r,bwa+24*r);
            pcad3(bwa+ 1*r,bwa+24*r,bwa+25*r);
            pcad3(bwa+ 2*r,bwa+23*r,bwa+26*r);
            pcad3(bwa+ 1*r,bwa+26*r,bwa+27*r);
            pcad3(bwa+ 1*r,bwa+27*r,bwa+28*r);
            pcad3(bwa+ 2*r,bwa+26*r,bwa+29*r);
            pcad3(bwa+ 1*r,bwa+29*r,bwa+30*r);
            pcad3(bwa+ 1*r,bwa+30*r,bwa+31*r);
            pcad3(bwa+ 5*r,bwa+23*r,bwa+32*r);
            pcad3(bwa+ 1*r,bwa+32*r,bwa+33*r);
            pcad3(bwa+ 1*r,bwa+33*r,bwa+34*r);
            pcad3(bwa+ 2*r,bwa+32*r,bwa+35*r);
            pcad3(bwa+ 1*r,bwa+35*r,bwa+36*r);
            pcad3(bwa+ 1*r,bwa+36*r,bwa+37*r);
            pcad3(bwa+ 2*r,bwa+35*r,bwa+38*r);
            pcad3(bwa+ 1*r,bwa+38*r,bwa+39*r);
            pcad3(bwa+ 1*r,bwa+39*r,bwa+40*r);
            bwa+=41*r;
        }
        return;
    }
}

void BwaMad(const FIELD *f, uint8_t * bwa, int sparsity, Afmt *af, Cfmt *c)
{
    if(sparsity==0) return;
    if(f->BwaMagic==1)
    {
        if(f->mact[0]>='j')
        {
            pcjas(af,bwa,c,f->parms);    // AVX2
            return;
        }
        pcaas(af,bwa,c,f->parms);        // SSE2
        return;
    }
// maybe ought to be different BwaMagic values . . . ?
    if(f->BwaMagic==2)
    {
        if(f->mact[0]>='j')
        {
            pcjb2(af,bwa,c);    // AVX2
            return;
        }
        pcab2(af,bwa,c);        // SSE2
        return;
    }
    if(f->BwaMagic==3)
    {
        if(f->mact[0]>='j')
        {
            pcjb3(af,bwa,c);    // AVX2
            return;
        }
        pcab3(af,bwa,c);        // SSE2
        return;
    }
    if(f->BwaMagic==4)
    {
        if(f->mact[0]>='j')
        {
            pcjat(af,bwa,c,f->parms);    // AVX2
            return;
        }
        pcdas(af,bwa,c,f->parms);        // SSE2
        return;
    }
}

void BrickMad(const FIELD *f, uint8_t *bwa,
              Afmt *a, Bfmt *b, Cfmt *c)
{
    int sparsity;
    sparsity = BSeed(f,bwa,b);
    BGrease(f,bwa,sparsity);
    BwaMad(f,bwa,sparsity,a,c);
}

void BwaInit(const FIELD *f, uint8_t *bwa)
{
    uint64_t *bwa64;
    uint64_t i;
    bwa64=(uint64_t *)bwa;
    for(i=0;i<(f->bwasize/8);i++)
        bwa64[i]=f->bzer;
}

void CZer(DSPACE * ds, Cfmt * c, uint64_t nor)
{
    uint64_t i,nowords;
    uint64_t * c64;
    const FIELD * f;

    f=ds->f;
    c64=(uint64_t *)c;
    nowords=((ds->noc+f->cauldron-1)/f->cauldron)*nor*f->cfmtcauld/8;
    for(i=0;i<nowords;i++) c64[i]=f->czer;
}

void CtoD(DSPACE * ds, Cfmt * c, Dfmt * d, uint64_t nor)
{
    long zlen,cpylen,z2,z0,bits,ix,i;
    uint8_t * sp;
    uint16_t * sp16;
    uint32_t * sp32;
    Dfmt * dp;
    Dfmt dbyte;
    uint64_t bu,bt;
    uint8_t * Thpc;
    uint64_t nz2;
    uint64_t x,y,z;
    const FIELD * f;

    f=ds->f;
    nz2=(ds->noc+f->cauldron-1)/f->cauldron;
    
    bu=0;    // stop compiler
    bits=0;
    bt=0;    // warnings
    Thpc=(uint8_t *)f + (f->Thpc);
    for(z2=0;z2<nz2;z2++)
    {
        zlen=((z2+1)*f->cauldron);
        zlen-=ds->noc;
        if(zlen<0) zlen=0;
        if(zlen>f->cauldron) zlen=f->cauldron;  
        zlen=zlen*f->pbytesper/f->pentbyte;

        cpylen=f->dfmtcauld-zlen;
        for(z0=0;z0<nor;z0++)
        {
            dp=d+z0*ds->nob+z2*f->dfmtcauld;
            sp=c+z0*f->cfmtcauld+z2*nor*f->cfmtcauld;

            if((f->CfmtMagic)==1)
                   memcpy(dp,sp,cpylen);
            if(f->CfmtMagic==2)
            {
                sp16=(uint16_t *)sp;
                for(i=0;i<cpylen;i++)
                {
                    x=*(sp16++);
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                }
            }
            if((f->CfmtMagic)==3)
            {
                bu=0;
                bits=0;
                bt=0;
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
                        bt=bt+((*(sp+(f->cfmtcauld/2)))<<bits);
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
            if(f->CfmtMagic==4)
            {
                sp32=(uint32_t *)sp;
                for(i=0;i<cpylen-2;i+=3)
                {
                    z=*(sp32++);
                    x=z&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                    x=(z>>10)&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                    x=(z>>20)&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                }
                if(i==cpylen-1)
                {
                    z=*(sp32++);
                    x=z&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                }
                if(i==cpylen-2)
                {
                    z=*(sp32++);
                    x=z&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                    x=(z>>10)&0x3ff;
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    *(dp++)=x;
                }
            }
            if(f->CfmtMagic==5)
            {
                for(i=0;i<cpylen;i++)
                {
                    x=*(sp++);
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    z=*(sp++);
                    y=(z*f->bar48)>>48;
                    z-=y*f->charc;
                    *(dp++)=x+z*f->charc;
                }
            }
            if(f->CfmtMagic==6)
            {
                for(i=0;i<cpylen;i++)
                {
                    x=*(sp++);
                    y=(x*f->bar48)>>48;
                    x-=y*f->charc;
                    z=*(sp++);
                    y=(z*f->bar48)>>48;
                    z-=y*f->charc;
                    x+=z*f->charc;
                    z=*(sp++);
                    y=(z*f->bar48)>>48;
                    z-=y*f->charc;
                    *(dp++)=x+z*f->charc*f->charc;
                }
            }
        }
    }
}

/* end of hpmi.c  */
