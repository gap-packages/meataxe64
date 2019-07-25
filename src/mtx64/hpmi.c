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
        f->AfmtMagic=2;   // Always Table lookup on byte

// CfmtMagic (2,16-bit) (4,10 bit) (5,8 bit 128) (6 8 bit 126)
        f->CfmtMagic=2;
        if( (f->charc>=17) && (f->charc<=61) ) f->CfmtMagic=4; // 10 bits?
        if( (f->charc>=7) && (f->charc<=13) ) f->CfmtMagic=5;
        if(f->charc==5) f->CfmtMagic=6;
        if( (f->mact[0]<'d') && (f->CfmtMagic==4) ) f->CfmtMagic=2;  // no SSE4.1

        f->BfmtMagic=1;   // always just Dfmt

        f->BwaMagic=1;   // 16 bit multiply
        if((f->CfmtMagic==4) || (f->mact[0] > 'l') ) f->BwaMagic=4; //32 bit mult.
        f->GreaseMagic=1;  // follow addition chain
        f->SeedMagic=1;     // one row 16 bit
        if(f->CfmtMagic==4) f->SeedMagic=4;  // one row 10 bits
        if(f->CfmtMagic==5) f->SeedMagic=5;  // two rows 8 bits
        if(f->CfmtMagic==6) f->SeedMagic=6;  // three rows mod 5
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
    if(f->ppaktyp==1233)  // ppaktup==0 new stuff excluded for now
//  because it doesn't work!
    {
        f->p90=pcpmad(f->charc,0x400000000000, 0x100000000000, 0);
        f->AfmtMagic=4;
        f->BfmtMagic=1;
        f->GreaseMagic=4;
        f->bzer=0;
        f->czer=0;
        f->alcove=21;
        f->alcovebytes=169;
        f->dfmtcauld=584;
        f->bfmtcauld=584;
        f->cfmtcauld=1168;
        f->bbrickbytes=12265;
        f->bwasize=12264;
        f->cauldron=73;
        f->CfmtMagic=7;
        f->BwaMagic=6;
        f->SeedMagic=8;
    }
    return;
}

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
    if(f->AfmtMagic==4)   // Just copy Dfmt in copybytes at a time
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
    if(f->SeedMagic==2)    // Characteristic 2
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
// 7 used to be new mod 2 using 51-code - now removed
    if(f->SeedMagic==8)    // just copy it in
    {
        memcpy(bwa,pt1,f->alcove*f->bfmtcauld);
    }
    return *b;
}

uint8_t pg2[] = { 163 , 2 , 84, 165 , 1 , 3 , 1 ,
                  88 , 169, 1 , 3 , 1 , 7 , 1 , 3 , 1 , 255 };

//               0001 3/0011 4/0012 10/0112 11/0120 12/0121 13/0122
uint8_t pg3[] = { 163,  2   ,  1   ,170,5  ,   3   ,   1   ,   1 ,

// 6/0101 7/0102 8/0110 9/0111 27/1111 28/1112 29/1120 30/1121 31/1122
   166,4 , 1    ,   3  ,   1  ,187,14  ,    1  ,   3   ,   1   ,   1  ,

// 32/1200 33/1201 34/1202 35/1210 36/1211 37/1212 38/1220 39/1221
      9   ,   1   ,   1   ,   3   ,   1   ,   1   ,   3   ,   1  ,

// 40/1222 15/1001 16/1002 17/1010 18/1011 19/1012 20/1020 21/1021
      1   ,175,10 ,   1   ,   3   ,   1   ,   1   ,   3   ,   1  ,   

// 22/1022 23/1100 24/1101 25/1102 26/1110
      1   ,   9   ,   1   ,   1   ,   3   , 255 };

void BGrease(const FIELD * f, uint8_t * bwa, int sparsity)
{
    if(sparsity==0) return;
    if((f->GreaseMagic)==4) return;   // no grease - prime to large.
    if((f->GreaseMagic)==1)
    {
        pc5aca(f->prog,bwa,f->parms);
        return;
    }
    if((f->GreaseMagic)==2)
    {
        if(f->mact[0]>='m')
        {
            pc2acm(pg2,bwa,2048);
            return;
        }
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
            pc2acj(pg2,bwa,2048);
        else
            pc2aca(pg2,bwa,2048);
        return;
    }
    if((f->GreaseMagic)==3)
    {
        if(f->mact[0]>='m')
        {
            pc3acm(pg3,bwa,5248);
            return;
        }
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
            pc3acj(pg3,bwa,5248);
        else
            pc3aca(pg3,bwa,5248);
        return;
    }
    printf("GreaseMagic error!\n");
    exit(91);
}

void BwaMad(const FIELD *f, uint8_t * bwa, int sparsity, Afmt *af, Cfmt *c)
{
    if(sparsity==0) return;
// AS codes 5-193 16-bit multiply
    if(f->BwaMagic==1)
    {
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pc5bmwj(af,bwa,c,f->parms);    // AVX2 16-bit
            return;
        }
        pc5bmwa(af,bwa,c,f->parms);        // SSE2 16-bit
        return;
    }
// Mod 2 xor
    if(f->BwaMagic==2)
    {
        if(f->mact[0]>='m')
        {
            pc2bmm(af,bwa,c);    // AVX512
            return;
        }
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pc2bmj(af,bwa,c);    // AVX2
            return;
        }
        pc2bma(af,bwa,c);        // SSE2
        return;
    }
// Mod 3 Logic 6
    if(f->BwaMagic==3)
    {
        if(f->mact[0]>='m')
        {
            pc3bmm(af,bwa,c);        // AVX512
            return;
        }
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pc3bmj(af,bwa,c);    // AVX2
            return;
        }
        pc3bma(af,bwa,c);        // SSE2
        return;
    }
// AS codes 32-bit multiply
    if(f->BwaMagic==4)
    {
        if(f->mact[0]>='m')
        {
            pc5bmdm(af,bwa,c,f->parms);    // AVX2512 32-bit
            return;
        }
        if( (f->mact[0]=='j')||(f->mact[0]>='l') )
        {
            pc5bmdj(af,bwa,c,f->parms);    // AVX2 32-bit
            return;
        }
        pc5bmdd(af,bwa,c,f->parms);        // SSE2 32-bit
        return;
    }
// 64-bit scalar multiply
    if(f->BwaMagic==6)
    {
        pc6bma(af,bwa,c,f->p90);      // scalar 64-bit
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

void CtoD(DSPACE * ds, Cfmt * c, Dfmt * d, uint64_t nor, uint64_t stride)
{
    long zlen,cpylen,z2,z0,bits,ix,i;
    uint8_t * sp;
    uint16_t * sp16;
    uint32_t * sp32;
    uint64_t * sp64;
    Dfmt * dp;
    uint64_t * dp64;
    Dfmt dbyte;
    uint64_t bu,bt;
    uint8_t * Thpc;
    uint64_t nz2;
    uint64_t x,y,z;
    const FIELD * f;

    f=ds->f;
    nz2=(ds->noc+f->cauldron-1)/f->cauldron;    // number of cauldrons
    
    bu=0;    // stop compiler
    bits=0;
    bt=0;    // warnings
    Thpc=(uint8_t *)f + (f->Thpc);
    for(z2=0;z2<nz2;z2++)
    {
        zlen=((z2+1)*f->cauldron);    // first entry not in this cauldron
        zlen-=ds->noc;                // entries to ignore this cauldron
        if(zlen<0) zlen=0;            // now correct
        if(zlen>f->cauldron) zlen=f->cauldron;    // don't think this happns
        zlen=zlen*f->pbytesper/f->pentbyte;   // Dfmt bytes to ignore

        cpylen=f->dfmtcauld-zlen;    // Dfmt bytes wanted
        for(z0=0;z0<nor;z0++)
        {
            dp=d+z0*stride+z2*f->dfmtcauld;
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
            if(f->CfmtMagic==7)
            {
                dp64=(uint64_t *)dp;
                sp64=(uint64_t *)sp;
                for(i=0;i<cpylen;i+=8)
                {
                    *(dp64++)=pcrem(f->charc,*sp64,*(sp64+1));
                    sp64+=2;
                }
            }
        }
    }
}

/* end of hpmi.c  */
