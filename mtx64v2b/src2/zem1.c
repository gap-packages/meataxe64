/*
      zem.c     meataxe-64 Elementary Matrix factorization
                outputs a factorization of the inverse of S 
                where S.M is Identity followed by rows of zeros
      =====     R. A. Parker    3.4.2019
*/

// only prints if less than this
#define INTEREST 1000000


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"

typedef struct
{
    FELT sc;
    long val;
    uint16_t R1;
    uint16_t R2;
    uint16_t R3;
    uint8_t cmd;
    uint64_t old;
}   HIS;

HIS * hist;       // history [cost][slot] 
Dfmt * mats;      // matrices[cost][slot]
int * bct;        // number in beam[cost] so far

int costcmd[12];

int beam;
uint64_t siz;
long target;
uint64_t fdef,nor,noc;
FIELD * f;
DSPACE ds;

void answer(FILE * f1, uint64_t slot)
{
    uint64_t s;
    s=slot;
    while(hist[s].cmd!=100)
    {
        switch (hist[s].cmd)
        {
          case 1:
            fprintf(f1," %d %lu %d %d\n",1,hist[s].sc,hist[s].R1,hist[s].R2);
            break;
          case 2:
            fprintf(f1," %d %d %d\n",2,hist[s].R1,hist[s].R2);
            break;
          case 3:
            fprintf(f1," %d %d %d\n",3,hist[s].R1,hist[s].R2);
            break;
          case 4:
            fprintf(f1," %d %lu %d\n",4,hist[s].sc,hist[s].R1);
            break;
          case 5:
            fprintf(f1," %d %d %d %d\n",5,hist[s].R1,hist[s].R2,hist[s].R3);
            break;
          case 6:
            fprintf(f1," %d %d %d %d\n",6,hist[s].R1,hist[s].R2,hist[s].R3);
            break;
          case 7:
            fprintf(f1," %d %lu %d %d\n",7,hist[s].sc,hist[s].R1,hist[s].R2);
            break;
          case 8:
            fprintf(f1," %d %d %d\n",8,hist[s].R1,hist[s].R2);
            break;
          case 9:
            fprintf(f1," %d %d %d\n",9,hist[s].R1,hist[s].R2);
            break;
          case 10:
            fprintf(f1," %d %d %d\n",10,hist[s].R1,hist[s].R2);
            break;
          case 11:
            fprintf(f1," %d %d %d\n",11,hist[s].R1,hist[s].R2);
            break;
          default:
            printf("System is broken\n");
            exit(3);
        }
        s=hist[s].old;
    }
    fprintf(f1," 0\n");
}

uint64_t bhash(Dfmt * d)
{
    uint64_t i,hash;
    hash=71;
    for(i=0;i<siz;i++) hash=hash*65537+d[i]*17;
    return hash%beam;
}

long val(Dfmt * m)
{
    long corr,vz;
    int i,j;
    FELT fel;

    corr=target;
    for(i=0;i<nor;i++)
    {
        vz=4;
        for(j=0;j<ds.noc;j++)
        {
            fel=DUnpak(&ds,j,m+i*ds.nob);
            if(j<i)
            {
                if(fel!=0) vz=2;
                if(fel==0) corr-=vz;
            }
            if(j==i)
            {
                if(fel==0) break;
                if(fel==1) corr-=3;
                   else    corr-=2;
            }
            if(j>i)
            {
                if(fel==0) corr-=vz/2;
            }
        }
    }
    return corr;
}

int main(int argc,  char **argv)
{
    EFIL *e;
    FILE *f1;
    uint64_t hdr[5];
    uint64_t maxtotcost;
    uint64_t i,sloto,slot;
    uint64_t newbase,newslot;
    uint64_t R1,R2,R3;
    long nval;
    int ccode;
    FELT fel,lambda;
    Dfmt *X, *Y;
    int junk=0;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zem mtx fac");
        exit(21);
    }
    e=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    siz=ds.nob*nor;
    X=malloc(siz);
    Y=malloc(siz);
    ERData(e,siz,X);
    ERClose(e);
    junk+=scanf("%d",&beam);
    for(i=1;i<=11;i++) junk+=scanf("%d",costcmd+i);
    (void)junk;
//  Allocate history structure
    maxtotcost=0;
    for(i=1;i<=9;i++)
        if( (costcmd[i]>maxtotcost) && (costcmd[i]<=12) )
            maxtotcost=costcmd[i];
    maxtotcost=maxtotcost*nor*noc + 30;
    hist=malloc(beam*maxtotcost*sizeof(HIS));
// may need to recycle these a bit
    mats=malloc(beam*maxtotcost*siz);
//          all 4    3 on diag  2 after diag
    target= 4*nor*noc  - noc  -  noc*(noc-1);

    for(i=0;i<beam*maxtotcost;i++)
    {
        hist[i].cmd=0;    // empty
        hist[i].val=target+1000;    // high values
    }
    hist[0].cmd=100;       // end of chain.
    hist[0].val=val(X);
    memcpy(mats,X,siz);
    sloto=0;
    while(1)
    {
        slot=sloto;
        sloto++;
        if((sloto+12*beam)>=beam*maxtotcost)
        {
            printf("Run out of space in beam\n");
            exit(1);
        }
        if(hist[slot].cmd==0) continue;
        if(hist[slot].val==0)           /* *** write out answer *** */
        {
if((slot/beam) < INTEREST)
printf("Beam %d,  charc %d, deg %d, Cost %d\n",
        (int)beam,(int)fdef,(int)noc, (int)(slot/beam));
            f1=fopen(argv[2],"wb");
            answer(f1,slot);
            fclose(f1);
            return 0;
        }
        memcpy(X,mats+slot*siz,siz);
// types=1,2,3,7,8,9
        for(R1=0;R1<nor;R1++)
        {
            if(DNzl(&ds,X+R1*ds.nob)==ZEROROW) continue;
            for(R2=0;R2<nor;R2++)
            {
                if(DNzl(&ds,X+R2*ds.nob)==ZEROROW) continue;
                if(R1==R2) continue;
                for(fel=1;fel<fdef;fel++)
                {
                    lambda=FieldNeg(f,fel);
                    memcpy(Y,X,siz);
                    DSMad(&ds,fel,1,Y+R1*ds.nob,Y+R2*ds.nob);
                    ccode=1;
                    if(DNzl(&ds,Y+R2*ds.nob)==ZEROROW)
                        ccode=7;
                    nval=val(Y);

                    if( (fel==1)&&(f->charc!=2) ) ccode+=2;
                    if( (fel+1)==f->charc ) ccode+=1;
                    if(costcmd[ccode]>99) continue;
                    newbase=slot-(slot%beam)+costcmd[ccode]*beam;
                    newslot=newbase+bhash(Y);
                    if(hist[newslot].val<=nval) continue;
                    hist[newslot].val=nval;
                    hist[newslot].R1=R1;
                    hist[newslot].R2=R2;
                    hist[newslot].sc=lambda;
                    hist[newslot].cmd=ccode;
                    hist[newslot].old=slot;
                    memcpy(mats+newslot*siz,Y,siz);
                }
            }
        }
// type=4
        newbase=slot-(slot%beam)+costcmd[4]*beam;
        for(R1=0;R1<nor;R1++)
        {
            if(costcmd[4]>99) continue;
            if(DNzl(&ds,X+R1*ds.nob)==ZEROROW) continue;
            for(fel=2;fel<fdef;fel++)
            {
                memcpy(Y,X,siz);
                DSMul(&ds,fel,1,Y+R1*ds.nob);
                nval=val(Y);
                newslot=newbase+bhash(Y);
                if(hist[newslot].val<=nval) continue;
                hist[newslot].val=nval;
                hist[newslot].R1=R1;
                hist[newslot].sc=FieldInv(f,fel);
                hist[newslot].cmd=4;
                hist[newslot].old=slot;
                memcpy(mats+newslot*siz,Y,siz);
            }
        }
// type=5
        newbase=slot-(slot%beam)+costcmd[5]*beam;
        for(R1=0;R1<nor;R1++)
        {
            if(costcmd[5]>99) continue;
            if(DNzl(&ds,X+R1*ds.nob)==ZEROROW) continue;
            for(R2=0;R2<nor;R2++)
            {
                if(DNzl(&ds,X+R2*ds.nob)==ZEROROW) continue;
                for(R3=0;R3<nor;R3++)
                {
                    if(R3==R1) continue;
                    if(R3==R2) continue;
                    memcpy(Y,X,siz);
                    DSub(&ds,1,Y+R3*ds.nob,X+R1*ds.nob,Y+R3*ds.nob);
                    DSub(&ds,1,Y+R3*ds.nob,X+R2*ds.nob,Y+R3*ds.nob);
                    if(DNzl(&ds,Y+R3*ds.nob)!=ZEROROW) continue;
                    nval=val(Y);
                    newslot=newbase+bhash(Y);;
                    if(hist[newslot].val<=nval) continue;
                    hist[newslot].val=nval;
                    hist[newslot].R1=R1;
                    hist[newslot].R2=R2;
                    hist[newslot].R3=R3;
                    hist[newslot].cmd=5;
                    hist[newslot].old=slot;
                    memcpy(mats+newslot*siz,Y,siz);
                }
            }
        }
// type=6
        newbase=slot-(slot%beam)+costcmd[6]*beam;
        for(R1=0;R1<nor;R1++)
        {
            if(costcmd[6]>99) continue;
            if(DNzl(&ds,X+R1*ds.nob)==ZEROROW) continue;
            for(R2=0;R2<nor;R2++)
            {
                if(DNzl(&ds,X+R2*ds.nob)==ZEROROW) continue;
                for(R3=0;R3<nor;R3++)
                {
                    if(R3==R1) continue;
                    if(R3==R2) continue;
                    memcpy(Y,X,siz);
                    DSub(&ds,1,Y+R3*ds.nob,X+R1*ds.nob,Y+R3*ds.nob);
                    DAdd(&ds,1,X+R2*ds.nob,Y+R3*ds.nob,Y+R3*ds.nob);
                    if(DNzl(&ds,Y+R3*ds.nob)!=ZEROROW) continue;
                    nval=val(Y);
                    newslot=newbase+bhash(Y);
                    if(hist[newslot].val<=nval) continue;
                    hist[newslot].val=nval;
                    hist[newslot].R1=R1;
                    hist[newslot].R2=R2;
                    hist[newslot].R3=R3;
                    hist[newslot].cmd=6;
                    hist[newslot].old=slot;
                    memcpy(mats+newslot*siz,Y,siz);
                }
            }
        }
// type=10, 11

        for(R1=0;R1<nor;R1++)
        {
            ccode=10;
            if(DNzl(&ds,X+R1*ds.nob)==ZEROROW) ccode=11;
            newbase=slot-(slot%beam)+costcmd[ccode]*beam;
            if(costcmd[ccode]>99) continue;
            for(R2=0;R2<nor;R2++)
            {
                if(R1==R2) continue;
                memcpy(Y,X,siz);
                memcpy(Y+R2*ds.nob,X+R1*ds.nob,ds.nob);
                memcpy(Y+R1*ds.nob,X+R2*ds.nob,ds.nob);
                nval=val(Y);
                newslot=newbase+bhash(Y);
                if(hist[newslot].val<=nval) continue;
                hist[newslot].val=nval;
                hist[newslot].R1=R1;
                hist[newslot].R2=R2;
                hist[newslot].cmd=ccode;
                hist[newslot].old=slot;
                memcpy(mats+newslot*siz,Y,siz);
            }
        }
    }
    free(X);
    free(hist);
    free(mats);
    free(bct);
    printf("Hello World\n");
    return 0;
}

/******  end of zem.c    ******/
