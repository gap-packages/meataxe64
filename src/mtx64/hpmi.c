// Copyright (C) Richard Parker   2017
//  hpmi.c Meataxe64 Nikolaus version

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"
#include "tabmake.h"

// Uncomment for Gray mod 2, commented out grease level 4
// #define NEW2 1

void hpmiset(FIELD * f)
{
    uint64_t bias;
    f->cauldron=0;
    f->recbox=1024;
    f->abase=1;       // usually right
    if(f->mact[1]=='1') f->recbox=3072;
    if(f->mact[1]=='2') f->recbox=5000;
    if(f->charc==2)
    {
#ifdef NEW2
// this bit good
        f->SeedMagic=7;
        f->AfmtMagic=4;
        f->abase=700;    // get this right eventually
        f->GreaseMagic=4;
        f->BwaMagic=5;
        f->bwasize=32768;
// I think this is probably correct
        f->cauldron=1024;
        f->bfmtcauld=128;
        f->cfmtcauld=128;
        f->dfmtcauld=128;
        f->CfmtMagic=1;
        f->alcove=32;
        f->czer=0;
        f->bzer=0;
        f->bbrickbytes=4097;
        f->alcovebytes=6;
        f->BfmtMagic=1;
#else
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
        f->czer=0;
        f->bzer=0;
        f->bbrickbytes=4097;
        f->bwasize=16384;
#endif

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
        f->bzer=0xffffffffffffffff;
        f->czer=0xffffffffffffffff;
        f->bbrickbytes=1537;
        f->bwasize=15744;
        hpmitab3(f);
    }
    if( (f->charc>=5)&&(f->charc<=193) )
    {
        f->AfmtMagic=2;

// CfmtMagic (2,16-bit) (4,10 bit) (5,8 bit 128) (6 8 bit 126)
        f->CfmtMagic=2;
        if( (f->charc>=17) && (f->charc<=61) ) f->CfmtMagic=4;
        if( (f->charc>=7) && (f->charc<=13) ) f->CfmtMagic=5;
        if(f->charc==5) f->CfmtMagic=6;
        if( (f->mact[0]<'d') && (f->CfmtMagic==4) ) f->CfmtMagic=2;  // no SSE4.1

        f->BfmtMagic=1;

        f->BwaMagic=1;
        if(f->CfmtMagic==4) f->BwaMagic=4;
        f->GreaseMagic=1;
        f->SeedMagic=1;
        if(f->CfmtMagic==4) f->SeedMagic=4;
        if(f->CfmtMagic==5) f->SeedMagic=5;
        if(f->CfmtMagic==6) f->SeedMagic=6;
        f->cauldron=64;
        if(f->CfmtMagic==4) f->cauldron=96;
        if(f->CfmtMagic==5) f->cauldron=128;
        if(f->CfmtMagic==6) f->cauldron=126;
        f->bfmtcauld=f->cauldron;
        if(f->charc<=13) f->bfmtcauld=f->cauldron/2;  //entbyte=2
        if(f->charc<=5) f->bfmtcauld=f->cauldron/3;   //entbyte=3
        f->cfmtcauld=128;
        f->dfmtcauld=f->bfmtcauld;
        f->alcove=7;
        if( (f->charc>=7) && (f->charc<=13) ) f->alcove=14;
        if (f->charc==5) f->alcove=21;
        f->alcovebytes=8;
        bias=7*f->charc;
        if(f->charc<=7) bias=6*f->charc;
        if(f->CfmtMagic==2)
            f->czer=bias+(bias<<16)+(bias<<32)+(bias<<48);
        if(f->CfmtMagic==4)
            f->czer=bias+(bias<<10)+(bias<<20)
                   +(bias<<32)+(bias<<42)+(bias<<52);
        if(f->CfmtMagic>=5)
            f->czer=bias+(bias<<8)+(bias<<16)+(bias<<24)
                   +(bias<<32)+(bias<<40)+(bias<<48)+(bias<<56);
        f->bzer=0;
        f->bbrickbytes=1+f->alcove*f->bfmtcauld;
        f->bwasize=16384;
        hpmitabas(f);
    }
    if(f->charc==2) f->basestrass=10000;
    if(f->charc==3) f->basestrass=10000;
    if(f->pow>1) f->basestrass+=(f->basestrass/2);
    return;
}


extern void hpmi2a(DSPACE * ds, const Dfmt * d, Afmt * a,
            uint64_t nora, uint64_t stride);
void DtoA(DSPACE * ds, uint64_t * ix, const Dfmt * d, Afmt * a,
            uint64_t nora, uint64_t stride)
{
    int i,j,k;
    uint64_t nz1;
    uint8_t orc,byte,byte1;
    const uint8_t *pt1,*pt2;
    const uint32_t * pt32;
    uint32_t v32, *qt32;
    int s;
    int bits,nbits;
    int copybytes;
    uint8_t  * Thpa;
    uint16_t * Thpv;
    uint64_t alen;
    const FIELD * f;
    uint8_t * f8;
    f=ds->f;
    if(f->AfmtMagic==4)   // new characteristic 2
    {

        hpmi2a(ds,d,a,nora,stride);    // ix not used
        return;
    }
    f8=(uint8_t *)f;
    nz1=(ds->noc+f->alcove-1)/f->alcove;
    alen=f->abase+nora*f->alcovebytes;
    for(j=0;j<nz1;j++)
    {
        ix[j]=j*alen;
        a[j*alen]=0;  // skip no rows at the start.
    }

    copybytes=f->alcovebytes-1;
    pt1=d;
    Thpa=f8+f->Thpa;
    if(f->AfmtMagic==1)   // characteristic 2
    {
        for(i=0;i<nora;i++)  // for each row of the matrix
        {
            pt32=(uint32_t *) pt1;
            for(j=0;j<nz1-1;j++)   // j indexes the alcove - all but last
            {                      // fastpath
                v32=*(pt32++);
                if( (v32==0) && (a[ix[j]]<250) )  // sparsity
                    a[ix[j]]++;                   // skip one more row
                else
                {
                    qt32=(uint32_t *) &a[ix[j]+1];
                    *qt32=v32;
                    ix[j]+=f->alcovebytes;  // keep that block
                    a[ix[j]]=1;             // advance pointer
                }
            }
// now last value of j - slowpath
            orc=0;               // OR of the bytes
            pt2=pt1+copybytes*j;         // point to block of bytes
            for(k=1;k<=copybytes;k++)    // for each byte of block
            {
                if( (copybytes*j+k) > ds->nob) byte=0;
                              else            byte=*pt2;
                orc|=byte;      // OR in the byte for sparsity
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
            pt1+=stride;
        }

    }
    if(f->AfmtMagic==2)  // characteristic 5-193
    {
        for(i=0;i<nora;i++)  // for each row of the matrix
        {
            for(j=0;j<nz1;j++)   // j indexes the alcove
            {
                orc=0;               // OR of the bytes
                pt2=pt1+copybytes*j;         // point to block of bytes
                for(k=1;k<=copybytes;k++)    // for each byte of block
                {
                    if( (copybytes*j+k) > ds->nob) byte=0;
                                  else            byte=*pt2;
                    orc|=byte;      // OR in the byte for sparsity
                    byte=Thpa[byte];
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
            pt1+=stride;
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
                        if( s >= ds->nob) byte=0;
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
            pt1+=stride;
        }

    }
    for(j=0;j<nz1;j++)
        a[ix[j]]=255;  // Put in the terminators
}

uint64_t DtoB(DSPACE * ds, const Dfmt * d, Bfmt * b, 
              uint64_t nor, uint64_t stride)
{
    uint64_t i,k;
    const Dfmt *pt0, *pt1, *ad;
    Dfmt xd;
    Bfmt *pt2;
    Bfmt sp;
    uint64_t sp64,*pt164,*pt264;
    uint64_t * Thpb;
    int ubits,tbits,nbits;
    uint8_t *bu,*bt;
    uint64_t m,mu,mt;
    const FIELD * f;

    f=ds->f;
    pt0=d;
    if(f->BfmtMagic==1)
    {
        sp64=0;
        for(i=0;i<f->alcove;i++)
        {
            pt1=pt0;
            pt2=b+i*f->bfmtcauld+1;
            if(i<nor)
            {
                pt164=(uint64_t *) pt1;
                pt264=(uint64_t *) pt2;
                for(k=0;(k+7)<ds->nob;k+=8)
                {
                    sp64|=*(pt164);
                    *(pt264++) = *(pt164++);
                }
                pt1=(Bfmt *) pt164;
                pt2=(Bfmt *) pt264;
                for(;k<ds->nob;k++)
                {
                    sp64|=(uint64_t) (*(pt1));
                    *(pt2++) = *(pt1++);
                }
                for(k=ds->nob;k<f->bfmtcauld;k++)   *(pt2++)=0;
            }
            else memset(pt2,0,f->bfmtcauld);
            pt0+=stride;
        }
        if(sp64==0)
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
            ad=d+i*stride;
            for(k=0;k<f->dfmtcauld;k++)
            {
                if(k>=ds->nob) xd=0;
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
    if(f->SeedMagic==2)    // OLD characteristic 2
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
    if(f->SeedMagic==7)    // New characteristic 2
    {
        memcpy(bwa+f->bfmtcauld,pt1,32*f->bfmtcauld);
    }
    return *b;
}

uint8_t pg2[] = { 1, 2, 3, 1, 4, 5,
                  2, 4, 6, 1, 6, 7,
                  1, 8, 9, 2, 8,10,
                  1,10,11, 4, 8,12,
                  1,12,13, 2,12,14,
                  1,14,15, 0 ,0,     0,0,0,0,0,0,0,0};

uint8_t grease2[] = { 22,23,99,   22,24,100,  23,24,101,  24,99,102,
                      22,25,111,  24,25,112,  25,100,113, 26,99,128,
                      28,102,122, 27,22,123,  23,26,124,  23,25,125,
                      25,99,126,  26,22,127,  29,23,154,  27,23,155,
                      29,99,156,  26,100,157, 27,101,158, 28,23,159,
                      25,101,186, 27,99,187,  30,22,188,  26,101,189,
                      27,102,190, 29,22,191,  25,102,218, 26,24,219,
                      26,102,221, 28,22,222,  27,24,223,  27,100,250,
                      28,99,251,  28,24,253,  28,100,254, 28,101,255,

                      7,9,33,     10,4,34,    7,1,35,     6,3,62,
                      62,5,36,    36,4,37,    36,34,38,   5,3,63,
                      63,10,64,   64,7,39,    39,6,40,    10,8,65,
                      65,39,41,   8,2,107,    107,41,42,  107,64,43,
                      33,1,105,   105,43,55,  55,2,56,    105,63,57,
                      55,42,58,   55,8,59,    55,41,60,   55,40,61,
                      62,61,62,   62,63,63,   36,35,110,  110,107,64,
                      64,105,65,  64,7,77,    65,7,78,    3,2,109,
                      107,3,108,  108,40,79,  79,5,80,    80,3,81,
                      80,63,82,   80,62,83,   38,39,106,  106,105,84,
                      84,36,85,   84,10,86,   85,10,87,
                      10,6,104,   109,4,103,  103,1,103,  103,104,104,
                      57,55,109,  109,104,105,109,103,106,57,36,110,
                      110,104,107,110,105,108,110,106,109,110,103,110,
                      11,12,12,


                      11,17,17,
                      11,19,19,
                      11,20,20,
                      11,21,21,
                      18,20,44,   21,15,45,   18,12,46,   17,14,73,

                      11,14,14,

                      73,16,47,   47,15,48,   47,45,49,   16,14,74,
                      11,16,16,
                      11,49,49,   11,18,18, 
                      74,21,75,   75,18,50,   50,17,51,   21,19,76,
                      11,51,51,
                      76,50,52,   19,13,118,  118,52,53,  118,75,54,
                      11,53,53,
                      44,12,116,  116,54,66,  66,13,67,   116,74,68,
                      11,13,13,   11,66,66,
                      66,53,69,   66,19,70,   66,52,71,   66,51,72,
                      11,66,66,
                      73,72,73,   73,74,74,   47,46,121,  121,118,75,
                      75,116,76,  75,18,88,   76,18,89,   14,13,120,
                      11,88,88,   11,89,89,
                      118,14,119, 119,51,90,  90,16,91,   91,14,92,
                      11,73,73,
                      91,74,93,   91,73,94,   49,50,117,  117,116,95,
                      95,47,96,   95,21,97,   96,21,98,
                      11,91,91,   11,95,95,   11,98,98,
                      21,17,115,  120,15,114, 114,12,114, 114,115,115,
                      11,15,15, 
                      68,66,120,  120,115,116,120,114,117,68,47,121,
                      121,115,118,121,116,119,121,117,120,121,114,121,
  
                      0,0};

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
// using "Gray code" addition chain technology, mainly to test it.
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            for(i=0;i<8;i++)
            {
                pch2j(pg2,bwa,NULL);
                bwa+=16*r;
            }
            return;
        }
        for(i=0;i<8;i++)
        {
                pch2a(pg2,bwa,NULL);
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
    if((f->GreaseMagic)==4)    // new characteristic 2 grease
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pch2j(grease2,bwa,NULL);
            return;
        }
        pch2a(grease2,bwa,NULL);
    }
    return;
}

void BwaMad(const FIELD *f, uint8_t * bwa, int sparsity, Afmt *af, Cfmt *c)
{
    if(sparsity==0) return;
// AS codes 5-193 (except 17-61 mact>='d' uses BwaMagic==4)
    if(f->BwaMagic==1)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pcjas(af,bwa,c,f->parms);    // AVX2 16-bit
            return;
        }
        pcaas(af,bwa,c,f->parms);        // SSE2 16-bit
        return;
    }
// Mod 2 xor
    if(f->BwaMagic==2)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pcjb2(af,bwa,c);    // AVX2
            return;
        }
        pcab2(af,bwa,c);        // SSE2
        return;
    }
// Mod 3 Logic 6
    if(f->BwaMagic==3)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pcjb3(af,bwa,c);    // AVX2
            return;
        }
        pcab3(af,bwa,c);        // SSE2
        return;
    }
// AS codes 17-61 in 10 bit (needs SSE4.1)
    if(f->BwaMagic==4)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pcjat(af,bwa,c,f->parms);    // AVX2 32-bit
            return;
        }
        pcdas(af,bwa,c,f->parms);        // SSE2 32-bit
        return;
    }
// New characteristic 2 
    if(f->BwaMagic==5)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pch2j(af,bwa,c);    // AVX2
            return;
        }
        pch2a(af,bwa,c);        // SSE2
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
