/*   hpmitab.c                      */
/*   R. A. Parker 14.10.2017        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"

/* NOTE - prog only 40 entries in field.h but longest is 33 so it's OK */
uint8_t achain[] = {
  5,23,4,   164,2,2,1,6,83,1,8,6,10,83,2,9,166,6,168,8,164,86,6,172,9,255,
  7,10,3,   163,1,1,82,2,2,4,83,5,255,
  11,21,4,  163,1,2,4,5,5,2,8,2,85,1,8,83,163,3,165,85,9,168,9,255,
  13,26,4,  163,1,3,2,4,82,2,1,8,1,8,10,12,12,164,84,13,
                           170,10,172,84,7,88,168,13,255,
  17,6,5,   1,1,2,163,3,255,
  19,6,5,   1,1,3,163,3,255,
  23,7,5,   1,1,1,3,164,4,255,
  29,6,5,   1,2,3,1,5,255,
  31,9,5,   1,2,1,4,162,2,164,4,255,
  37,10,7,  1,2,1,1,5,162,2,165,5,255,
  41,9,7,   1,1,1,1,5,5,165,5,255,
  43,10,6,  1,2,1,83,3,4,4,164,4,255,
  47,11,6,  1,2,1,3,4,4,162,2,164,3,255,
  53,11,6,  1,2,1,83,3,4,4,4,164,4,255,
  59,12,6,  1,1,3,1,5,1,5,163,3,165,5,255,
  61,14,6,  1,1,2,2,2,5,4,167,7,162,5,165,5,255,
  67,12,9,  1,1,3,4,2,85,4,6,6,166,6,255,
  71,14,9,  1,1,1,4,4,2,86,6,1,164,7,167,7,255,
  73,19,9,  1,1,1,3,2,4,2,164,3,167,7,162,84,4,164,4,166,6,255,
  79,18,8,  1,1,2,3,3,1,85,5,6,162,2,85,9,166,7,83,3,255,
  83,12,8,  1,2,3,1,5,6,7,8,2,165,9,255,
  89,17,9,  1,2,1,1,5,2,2,8,8,168,1,89,162,9,165,5,255,
  97,13,9,  1,1,3,4,4,2,4,7,7,2,170,11,255,
 101,16,9,  1,1,1,2,5,1,4,86,6,5,7,167,7,165,6,255,
 103,18,7,  1,1,1,2,1,85,5,6,8,8,8,87,166,7,89,165,5,255,
 107,18,7,  1,1,3,1,3,1,86,4,4,2,7,163,8,167,91,11,5,255,
 109,17,7,  1,2,1,4,4,2,3,8,4,2,162,9,164,5,82,2,255,
 113,22,7,  1,2,3,1,84,4,1,3,2,88,4,89,7,167,90,8,6,164,6,162,2,255,
 127,22,7,  1,2,1,4,3,1,2,4,88,6,2,10,170,9,164,7,167,2,162,92,2,255,
 131,22,10, 1,2,3,3,1,4,4,8,5,6,6,163,7,166,7,170,90,8,162,82,8,255,
 137,18,9,  1,1,1,4,4,5,1,4,8,8,8,7,165,87,7,168,7,255,
 139,20,9,  1,2,2,1,3,6,6,8,86,2,87,4,10,12,165,5,164,84,12,255,
 149,26,9,  1,1,2,4,3,5,5,3,83,3,9,10,86,1,163,89,
                       7,165,6,170,13,162,2,173,4,255,
 151,19,9,  1,1,3,3,3,2,4,86,4,4,8,8,8,8,168,8,167,7,255,
 157,19,9,  1,1,1,4,4,5,5,7,5,8,88,1,4,12,165,12,172,12,255,
 163,25,9,  1,1,1,1,4,4,3,7,6,9,6,10,89,8,166,8,169,12,88,168,5,92,165,5,255,
 167,25,9,  1,1,2,2,2,4,3,7,8,10,2,8,3,163,3,165,5,162,2,170,6,167,89,4,255,
 173,25,8,  1,1,1,1,1,1,7,8,88,4,9,11,2,89,4,6,172,92,14,168,12,174,95,9,255,
 179,22,8,  1,1,1,1,1,3,2,6,9,7,7,1,11,8,168,12,92,167,7,172,13,255,
 181,25,8,  1,1,2,2,2,2,2,6,8,6,11,9,6,4,162,1,91,171,7,94,174,9,169,9,255,
 191,29,8,  1,1,3,4,4,1,7,2,9,9,2,7,2,9,
            90,170,6,85,165,7,91,169,7,90,167,7,167,9,255,
 193,33,8,  1,1,1,3,4,4,6,8,5,10,3,4,6,
            84,9,163,11,167,6,95,175,8,165,5,168,88,1,173,13,170,90,12,255,
                 0 };

void hpmitabas(FIELD * f)
{
    Dfmt ASCode[32];
    Dfmt res;
    DSPACE ds;
    int slot,slots,pct,i,j,plen,shift,bitlen,glev,vals;
    Dfmt acc[1];
    uint64_t bias,pow2,pow2a,mask,x;
    uint8_t * ftab8;
    uint8_t * f8;
    uint8_t  * Thpa;

    f->parms[4]=128;
    f->parms[6]=7;
    pct=0;
    while(achain[pct]!=0)
    {
        if(f->charc==achain[pct]) break;
        pct+=achain[pct+1]+3;
    }
    if(achain[pct]==0)
    {
        printf("no code found for %ld\n",f->charc);
        exit(13);
    }
    plen=achain[pct+1];
    shift=achain[pct+2];
    pct+=3;
// copy the addition chain into prog
    for(i=0;i<plen;i++)
        f->prog[i]=achain[pct++];
//  run the addition chain to populate ASCode
//  initialize the fake BWA
    glev=1;
    vals=f->charc;
    if(f->charc<=13)
    {
        glev=2;
        vals=f->charc*f->charc;
    }
    if(f->charc==5)
    {
        glev=3;
        vals=125;
    }
    PSSet(f,glev,&ds);
    ASCode[0]=0;
    ASCode[1]=1;
    if(glev>=2)ASCode[2]=f->charc;
    if(glev==3) ASCode[3]=25;
    acc[0]=1;
    slot=2;
    pct=0;
//  now run it, and count highest slot number
    slots=0;
    while(f->prog[pct]!=255)
    {
        if(f->prog[pct]<80)
        {
            DAdd(&ds,1,&ASCode[f->prog[pct]],acc,&ASCode[slot]);
            acc[0]=ASCode[slot];
            slot++;
            if(slot>slots) slots=slot;
            pct++;
            continue;
        }
        if(f->prog[pct]<160)
        {
            acc[0]=ASCode[f->prog[pct]-80];
            pct++;
            continue;
        }
        if(f->prog[pct]<240)
        {
            slot=f->prog[pct]-160;
            pct++;
            continue;
        }

    }
    f->parms[1]=shift;
    f->parms[5]=slots;
    bias=7*f->charc;
    if(f->charc<=7) bias=6*f->charc;
    bitlen=16;
    if(f->CfmtMagic==4) bitlen=10;
    if(f->CfmtMagic>=5) bitlen=8;
    if(bitlen==16)
        f->czer=bias+(bias<<16)+(bias<<32)+(bias<<48);
    if(bitlen==10)
        f->czer=bias+(bias<<10)+(bias<<20)
               +(bias<<32)+(bias<<42)+(bias<<52);
    if(bitlen==8)
        f->czer=bias+(bias<<8)+(bias<<16)+(bias<<24)
               +(bias<<32)+(bias<<40)+(bias<<48)+(bias<<56);
    f->parms[8]=f->czer;
    pow2=1;
    pow2=(pow2<<shift);
    x=pow2-f->charc;
    if(bitlen==16)
        f->parms[3]=x+(x<<16)+(x<<32)+(x<<48);
    if(bitlen==10)
        f->parms[3]=x+(x<<10)+(x<<20)
               +(x<<32)+(x<<42)+(x<<52);
    if(bitlen==8)
        f->parms[3]=x+(x<<8)+(x<<16)+(x<<24)
               +(x<<32)+(x<<40)+(x<<48)+(x<<56);
    x=f->charc;
    if(bitlen==16)
        f->parms[0]=x+(x<<16)+(x<<32)+(x<<48);
    if(bitlen==10)
        f->parms[0]=x+(x<<10)+(x<<20)
               +(x<<32)+(x<<42)+(x<<52);
    if(bitlen==8)
        f->parms[0]=x+(x<<8)+(x<<16)+(x<<24)
               +(x<<32)+(x<<40)+(x<<48)+(x<<56);
    x=pow2%f->charc;
    if(f->BwaMagic==4) f->parms[7]=x+(x<<32);
          else      f->parms[7]=x+(x<<16)+(x<<32)+(x<<48);
    pow2a=1;
    pow2a=pow2a<<bitlen;
    mask=(pow2a-1)^(pow2-1);
    if(bitlen==16)
        f->parms[2]=mask+(mask<<16)+(mask<<32)+(mask<<48);
    if(bitlen==10)
        f->parms[2]=mask+(mask<<10)+(mask<<20)
               +(mask<<32)+(mask<<42)+(mask<<52);
    if(bitlen==8)
        f->parms[2]=mask+(mask<<8)+(mask<<16)+(mask<<24)
               +(mask<<32)+(mask<<40)+(mask<<48)+(mask<<56);
    f8=(uint8_t *) f;
    ftab8=f8+f->hwm;
    Thpa=ftab8;
    f->Thpa=ftab8-f8;
    ftab8+=f->charc;
    for(i=0;i<vals;i++) Thpa[i]=0;
    for(i=0;i<slots;i++)
      for(j=0;j<slots;j++)
      {
          DSub(&ds,1,&ASCode[i],&ASCode[j],&res);
          Thpa[res]=(i<<4)+j;
      }
    f->hwm=ftab8-f8;  
}

void hpmitab3(FIELD * f)
{
    uint8_t * ftab8;
    uint8_t * f8;
    uint16_t * Thpv;
    uint8_t  * Thpa;
    uint64_t * Thpb;
    uint8_t  * Thpc;
    uint16_t s1,s2,s3,s4;
    uint64_t x,y,i,j,k;

    f8=(uint8_t *) f;
    ftab8=f8+f->hwm;
/* four tables, Thpv, Thpa, Thpb and Thpc   */
/*  First Thpv  . . . align 16 bits  */
    while( (((long)ftab8)&1) != 0 ) ftab8++;
    f->Thpv=ftab8-f8;
    Thpv=(uint16_t *) ftab8;
    for(i=0;i<243;i++)
    {
        s1=0;
        k=i;
        for(j=0;j<5;j++)
        {
            s1=(s1<<2) + (k/81);
            k*=3;
            k=k%243;
        }
        Thpv[i]=s1;
    }
    ftab8+=2*243;
//   Now Thpa - already aligned
    Thpa=ftab8;
    f->Thpa=ftab8-f8;
    ftab8+=256;
    for(i=0;i<256;i++) Thpa[i]=255;
    for(i=2;i<=80;i+=2)
    {
        s1=0;
        j=0;
        if(i>=4) j=(i-4)/2;
        if(i>=10) j=(i-10)/2;
        if(i>=28) j=(i-28)/2;
        s4=j%3;
        j=j/3;
        s3=j%3;
        j=j/3;
        s2=j%3;
        if(i>=28) s1=1;
        if( (i>=10) && (i<28) ) s2=1;
        if( (i>=4) && (i<10) ) s3=1;
        if( (i>=2) && (i<4) ) s4=1;
        Thpa[64*s1+16*s2+4*s3+s4]=(uint8_t) i;
        if(s1!=0) s1=3-s1;
        if(s2!=0) s2=3-s2;
        if(s3!=0) s3=3-s3;
        if(s4!=0) s4=3-s4;
        Thpa[64*s1+16*s2+4*s3+s4]=(uint8_t) (i+1);           
    }
    Thpa[0]=0;

/* Thpb - must align to 64 bits  */
    while( (((long)ftab8)&7) != 0 ) ftab8++;        
    Thpb=(uint64_t *) ftab8;
    f->Thpb=ftab8-f8;
    ftab8 += 8*243;
/* and finally Thpc  */
    Thpc=ftab8;
    f->Thpc=ftab8-f8;
    ftab8 += 1024; 
/* make both Thpb and Thpc at the same time  */
    for(i=0;i<1024;i++) Thpc[i]=243;
    for(i=0;i<243;i++)
    {
        x=0;
        y=0;
        k=i;
        for(j=0;j<5;j++)
        {
            if((k%3)!=1) x+=(1<<j);
            if((k%3)!=2) y+=(1<<j);
            k=k/3;
        }
        Thpb[i]=(x<<32)+y;
        Thpc[(x<<5)+y]=i;
    }
    f->hwm=ftab8-f8;
}


/* end of hpmitab.c  */
