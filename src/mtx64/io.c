/*
         io.c   -   Meataxe-64 I/O routines version 1a
         ====       R. A. Parker   30.5.2015
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "field.h"
#include "tfarm.h"
#include "io.h"

/* ====  Xlock data and methods  ====*/

pthread_mutex_t xlock=PTHREAD_MUTEX_INITIALIZER;

void XLock(void)
{
    pthread_mutex_lock(&xlock);
}

void XUnlock(void)
{
    pthread_mutex_unlock(&xlock);
}

extern void LogCmd(int argc, char ** argv)
{
    int i;
    FILE * logfile;
    XLock();
    logfile=fopen("logfile","ab");
    fprintf(logfile,"01 ");
    for(i=0;i<argc;i++) fprintf(logfile,"%s ",argv[i]);
    fprintf(logfile,"\n");
    fclose(logfile);
    XUnlock();
}


void CLogCmd(int argc, const char *argv[])
{
    int i;
    FILE * logfile;
    XLock();
    logfile=fopen("logfile","ab");
    fprintf(logfile,"01 ");
    for(i=0;i<argc;i++) fprintf(logfile,"%s ",argv[i]);
    fprintf(logfile,"\n");
    fclose(logfile);
    XUnlock();
}

/* initialize the hash variables in the EFIL  */

static void bcsstart(EFIL * e)
{
    e->P1=0x1234123412341234;
    e->Q1=0x0123456789ABCDEF;
    e->P2=0x1234123412341234;
    e->Q2=0x0123456789ABCDEF;
    e->P3=0x1234123412341234;
    e->Q3=0x0123456789ABCDEF;
    e->P4=0x1234123412341234;
    e->Q4=0x0123456789ABCDEF;
}

/* update the hash for bk32 32-byte chunks at e->buck64  */

static void bcshash(EFIL * e, uint64 * bk, uint64 bk32)
{
    uint64 X1,X2,X3,X4,i;
    uint64 * d;
    uint64 P1,P2,P3,P4,Q1,Q2,Q3,Q4;
    d=bk;
    P1=e->P1;
    P2=e->P2;
    P3=e->P3;
    P4=e->P4;
    Q1=e->Q1;
    Q2=e->Q2;
    Q3=e->Q3;
    Q4=e->Q4;
    for(i=0;i<bk32;i++)
    {
        X1=*(d++);
        X2=*(d++);
        X3=*(d++);
        X4=*(d++);
        Q1+=0x3141592653589793;
        Q2+=0x3141592653589793;
        Q3+=0x3141592653589793;
        Q4+=0x3141592653589793;
        P1^=X1;
        P2^=X2;
        P3^=X3;
        P4^=X4;
        Q1+=((X1>>21) | (X1<<43));
        Q2+=((X2>>21) | (X2<<43));
        Q3+=((X3>>21) | (X3<<43));
        Q4+=((X4>>21) | (X4<<43));
        P1+=Q1;
        P2+=Q2;
        P3+=Q3;
        P4+=Q4;
        Q1+=((P1<<27) | (P1>>37));
        Q2+=((P2<<27) | (P2>>37));
        Q3+=((P3<<27) | (P3>>37));
        Q4+=((P4<<27) | (P4>>37));
        P1^=Q1;
        P2^=Q2;
        P3^=Q3;
        P4^=Q4;
    }
    e->P1=P1;
    e->P2=P2;
    e->P3=P3;
    e->P4=P4;
    e->Q1=Q1;
    e->Q2=Q2;
    e->Q3=Q3;
    e->Q4=Q4;
}

static uint64 hash1(EFIL * e)
{
    return 5*e->P1+11*e->P3+19*e->Q2+29*e->Q4;
}

static uint64 hash2(EFIL * e)
{
    return 7*e->P2+13*e->P4+17*e->Q1+23*e->Q3;
}

static uint64 getbytes(EFIL * e, uint8 * p, size_t len)
{
    int r,bt;
    if(e->nex==64)
    {
        r=fread(e->bk,1,64,e->f);
        bcshash(e,(uint64 *)e->bk,2);
        (void) r;
        if(e->bk[0]==10) e->nex=1;
             else        e->nex=2;
    }
    if(e->bk[0]==10)
    {
        bt=64-e->nex;
        if(bt>len) bt=len;
        memcpy(p,e->bk+e->nex,bt);
        e->nex+=bt;
        return bt;
    }
    if(e->bk[e->nex]!=0)
    {
        bt=e->bk[e->nex];
        if(bt>len) bt=len;
        e->bk[e->nex]-=bt;
        memset(p,0,bt);
        return bt;
    }
    e->nex+=2;
    *p=e->bk[e->nex-1];
    return 1;
}

static size_t putbytes(EFIL * e, const uint8 * p, size_t bytes)
{
    int i,zct,r,bt;
    uint8 t[64];
    if(e->bk[0]==10)
    {
        if(e->nex<63)
        {
            bt=63-e->nex;
            if(bt>bytes) bt=bytes;
            memcpy(e->bk+e->nex,p,bt);
            e->nex+=bt;
            return bt;
        }
        e->bk[e->nex]=*p;
        zct=0;
        for(i=1;i<64;i++) if(e->bk[i]==0) zct++;
        if(zct<=32)
        {
            bcshash(e,(uint64 *)e->bk,2);
            if(e->null!=1) r=fwrite(e->bk,1,64,e->f);
            e->bk[0]=10;
            e->nex=1;
            (void)r;
            return 1;
        }
        t[0]=11;
        t[1]=0;
        t[2]=0;
        e->nex=2;
        for(i=1;i<64;i++)
        {
            if(e->bk[i]==0) t[e->nex]++;
            else        
            {
                t[e->nex+1]=e->bk[i];
                e->nex+=2;
                t[e->nex]=0;
            }
        }
        for(i=0;i<64;i++) e->bk[i]=t[i];
        return 1;      
    }
    if( ((*p)==0) && (e->bk[e->nex]<255) )
    {
        bt=255 - e->bk[e->nex];
        if(bt>bytes) bt=bytes;
        for(i=1;i<bt;i++) if(p[i]!=0) break;
        e->bk[e->nex]+=i;
        return i;
    }
    e->bk[e->nex+1]=*p;
    e->nex+=2;
    if(e->nex<64) 
    {
        e->bk[e->nex]=0;
        return 1;
    }
    bcshash(e,(uint64 *)e->bk,2);
    if(e->null!=1) r=fwrite(e->bk,1,64,e->f);
    (void) r;
    e->bk[0]=10;
    e->nex=1;
    return 1;
}

char * fnamst(const char * fname)
{
    int i,j;
    char * fn;
    i=0;
    while(fname[i]!='\0') i++;
    fn=malloc(i+4);
    for(j=0;j<=i;j++) fn[j]=fname[j];
    return fn;
}

EFIL * ERHdr(const char * fname, uint64 * header)
{

    EFIL *e;
    FILE * f;
    uint64 blk[8];
    uint64 sav6,sav7;
    int r,i;
    f = fopen(fname,"rb");
    if(f == NULL)
    {
        printf("Cannot open input file %s\n",fname);
        exit(12);
    }
    e=malloc(sizeof(EFIL));
    e->f=f;
    e->bk=malloc(64);
    e->fn=fnamst(fname);
    r=fread(blk,1,64,e->f);
    (void) r;
    sav6=blk[6];
    sav7=blk[7];
    blk[6]=0x5A5A5A5A5A5A5A5A;
    blk[7]=0x5A5A5A5A5A5A5A5A;
    bcsstart(e);
    bcshash(e,blk,2);
    blk[6]=hash1(e);
    blk[7]=hash2(e);
    if( (sav6!=blk[6]) || (sav7!=blk[7]) )
    {
        printf("Checksum of header invalid\n");
        exit(21);
    }
    bcsstart(e);
    bcshash(e,blk,2);
    e->nex=64;
    for(i=0;i<5;i++) header[i]=blk[i+1];
    return e;
}


void EPeek(const char * fname, uint64 * header)
{
    EFIL *e;
    uint64 blk[8];
    uint64 sav6,sav7;
    int r,i;
    FILE * f;
    f = fopen(fname,"rb");
    if(f == NULL)
    {
        printf("Cannot open input file %s\n",fname);
        exit(12);
    }
    e=malloc(sizeof(EFIL));
    e->f=f;
    e->bk=malloc(64);
    r=fread(blk,1,64,e->f);
    (void) r;
    sav6=blk[6];
    sav7=blk[7];
    blk[6]=0x5A5A5A5A5A5A5A5A;
    blk[7]=0x5A5A5A5A5A5A5A5A;
    bcsstart(e);
    bcshash(e,blk,2);
    blk[6]=hash1(e);
    blk[7]=hash2(e);
    if( (sav6!=blk[6]) || (sav7!=blk[7]) )
    {
        printf("Checksum of header invalid\n");
        exit(21);
    }
    for(i=0;i<5;i++) header[i]=blk[i+1];
    fclose(f);
    free(e->bk);
    free(e);
}

EFIL * EWHdr(const char * fname, const uint64 * header)
{
    FILE *f;
    EFIL *e;
    uint64 blk[8];
    int i,nil;
    int r;
    nil=0;
    if(strcmp("NULL",fname)==0) nil=1;;
    f=NULL;
    if(nil!=1)
    {
        f = fopen(fname,"wb");
        if(f == NULL)
        {
            printf("Cannot open output file %s\n",fname);
            exit(11);
        }
    }
    e=malloc(sizeof(EFIL));
    e->f=f;
    e->bk=malloc(64);
    e->fn=fnamst(fname);
    bcsstart(e);
    blk[0]=1;    // header block
    for(i=0;i<5;i++) blk[i+1]=header[i];
    blk[6]=0x5A5A5A5A5A5A5A5A;
    blk[7]=0x5A5A5A5A5A5A5A5A;
    bcshash(e,blk,2);
    blk[6]=hash1(e);
    blk[7]=hash2(e);
    bcsstart(e);
    bcshash(e,blk,2);
    if(nil!=1) r=fwrite(blk,1,64,e->f);
    (void)r;
    e->bk[0]=10;
    e->nex=1;
    e->null=nil;
    return e;
}

void EWData(EFIL * e, size_t bytes, const uint8 * d)
{
    uint64 i;
    while(bytes!=0)
    {
        i=putbytes(e,d,bytes);
        d+=i;
        bytes-=i;
    }
    return;
}

void ERData(EFIL * e, size_t bytes, uint8 * d)
{
    uint64 i;
    while(bytes!=0)
    {
        i=getbytes(e,d,bytes);
        d+=i;
        bytes-=i;
    }
    return;
}  

int EWClose1(EFIL * e, int mode)
{
    FILE * logfile;
    uint64 * pt;
    int i,j,k,r;
    uint64 x;

/* first flush the last block  */
/* needs to be clean to get checksum correct  */
    if(e->bk[0]==10)
    {
        if(e->nex!=1)
        {
            while(e->nex<64) e->bk[e->nex++]=0;
            bcshash(e,(uint64 *)e->bk,2);
            if(e->null!=1) r=fwrite(e->bk,1,64,e->f);
            (void) r;
        }
    }
    else
    {
        e->nex++;
        while(e->nex<64) e->bk[e->nex++]=0;
        bcshash(e,(uint64 *)e->bk,2);
        if(e->null!=1) r=fwrite(e->bk,1,64,e->f);
        (void) r;
    }
    for(i=0;i<64;i++) e->bk[i]=0;
    e->bk[0]=20;
    pt=(uint64 *) e->bk;
    pt[1]=hash1(e);
    pt[2]=hash2(e);
    if(mode==0) 
    {
        XLock();
        logfile=fopen("logfile","ab");
        fprintf(logfile,"11 %s ",e->fn);
        for(i=1;i<=2;i++)
        {
            x=pt[i];
            for(j=0;j<16;j++)
            {
                k=(x>>(60-4*j))&15;
                if(k<10) k+='0';
                    else  k+='A'-10;
                fprintf(logfile,"%c",k);
            }
        }
        fprintf(logfile,"\n");
        fclose(logfile);
        XUnlock();
    }
    if(e->null!=1) r=fwrite(e->bk,1,64,e->f);
    (void) r;
    if(e->null!=1) fclose(e->f);
    free(e->bk);
    free(e->fn);
    free(e);
    return 0;
}

void EWClose(EFIL * e)
{
    int i;
    i = EWClose1(e,0);
    (void) i;
}

int ERClose1(EFIL * e, int mode)
{
    FILE * logfile;
    int r,i,j,k;
    uint64 x;
    uint64 * pt;
    XLock();
    logfile=fopen("logfile","ab");
    r=fread(e->bk,1,64,e->f);
    (void)r;
    pt=(uint64 *) e->bk;
    e->bck[0]=hash1(e);
    e->bck[1]=hash2(e);
    if( (e->bck[0]!=pt[1]) || (e->bck[1]!=pt[2]) )
    {
        fprintf(logfile,"90 %s ",e->fn);
        fprintf(stderr,"*** Checksum error on file %s *** \n",e->fn);
        mode=0;
    }
    else
    {
        if(mode==0)
            fprintf(logfile,"10 %s ",e->fn);
    }
    for(i=0;i<2;i++)
    {
        x=e->bck[i];
        for(j=0;j<16;j++)
        {
            k=(x>>(60-4*j))&15;
            if(k<10) k+='0';
                else  k+='A'-10;
            if(mode==0) fprintf(logfile,"%c",k);
        }
    }
    if(mode==0) fprintf(logfile,"\n");
    if( (e->bck[0]!=pt[1]) || (e->bck[1]!=pt[2]) )
        exit(90);
    fclose(e->f);
    free(e->bk);
    free(e->fn);
    free(e);
    fclose(logfile);
    XUnlock();
    return 0;
}

void ERClose(EFIL * e)
{
    int i;
    i = ERClose1(e,0);
    (void) i;
}

extern void LogString(int type, const char *string)
{
    FILE * logfile;
    XLock();
    logfile=fopen("logfile","ab");
    fprintf(logfile,"%d %s",type,string);
    fprintf(logfile,"\n");
    fclose(logfile);
    XUnlock();
    if((type>=80)&&(type<=89)) fprintf(stderr,"%s\n",string);
}

/* end of io.c  */
