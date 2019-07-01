/*
      als1.c     meataxe-64 Search for Albrecht linear forms
      ======     R. A. Parker    16.4.19
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"

FIELD *f,*fq;
DSPACE ds;
DSPACE dst;
DSPACE dsk;
uint64_t dimt,noc;
Dfmt *teg,*tek,*ex,*lfa,*lfb,*ck,*te,*ww;
int charc,nor;

int checkit(Dfmt *m1, Dfmt *m2)
{
    uint64_t i,j,j1,j2;
    FELT h1,h2,h3,h4,h5,col;
    uint64_t piv[400];
    int x;

//  compute (te) as the tensor products of the rows of m1 and m2
//      a row of te is therefore noc*noc long
//
    memset(te,0,dst.nob*nor);
    for(i=0;i<nor;i++)    // nothing Ninja-class needed here!
        for(j1=0;j1<noc;j1++)
          for(j2=0;j2<noc;j2++)
          {
              h1=DUnpak(&ds,j1,m1+i*ds.nob);
              h2=DUnpak(&ds,j2,m2+i*ds.nob);
              h3=FieldMul(f,h1,h2);
              DPak(&dst,j1*noc+j2,te+i*dst.nob,h3);
          }

//
//  compute the matrix (ww) of what we want to compute
//


    memset(ww,0,dst.nob*noc);

    h1=1;
    h2=f->charc;
    for(i=0;i<noc;i++)
    {
        h3=h1;
        for(j1=0;j1<noc;j1++)
        {
            h4=h3;
            for(j2=0;j2<noc;j2++)
            {
                h5=h4%f->charc;
                DPak(&dst,i*noc+j1,ww+j2*dst.nob,h5);
                h4=h4/f->charc;
            }
            h3=FieldMul(fq,h3,h2);
        }
        h1=FieldMul(fq,h1,h2);
    }

//
//  Put the tensor matrix into echelon form
//

    memcpy(teg,te,dst.nob*nor);
//     set (tek) = keeptrack as the identity to start with

    memset(tek,0,dsk.nob*nor);
    for(i=0;i<nor;i++)  DPak(&dst,i,tek+i*dsk.nob,1);
    for(i=0;i<nor;i++)
    {
        col=DNzl(&dst,teg+i*dst.nob);
        if(col==ZEROROW)
            return 0;
        piv[i]=col;
        h1=DUnpak(&dst,col,teg+i*dst.nob);
        h2=FieldInv(f,h1);
        DSMul(&dst,h2,1,teg+i*dst.nob);
        DSMul(&dsk,h2,1,tek+i*dsk.nob);
        for(j=0;j<nor;j++)
        {
            if(j==i) continue;
            h1=DUnpak(&dst,col,teg+j*dst.nob);
            h2=FieldNeg(f,h1);
            DSMad(&dst,h2,1,teg+i*dst.nob,teg+j*dst.nob);
            DSMad(&dsk,h2,1,tek+i*dsk.nob,tek+j*dsk.nob);
        } 
    }

//
// Extract the pivots of tensors from the wanted matrix
//


    memset(ex,0,dsk.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dst,piv[j],ww+i*dst.nob);
            DPak(&dsk,j,ex+i*dsk.nob,h1);
        }
    }

//
//  Not sure what lfa is either
//

    memset(lfa,0,dsk.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,ex+i*dsk.nob);
            DSMad(&dsk,h1,1,tek+j*dsk.nob,lfa+i*dsk.nob);
        }
    }

//
//  This seems to be a transpose
//  lfb is the answer we seek

    memset(lfb,0,ds.nob*nor);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,lfa+i*dsk.nob);
            DPak(&ds,i,lfb+j*ds.nob,h1);
        }
    }

//
//  Check that it works
//

    memset(ck,0,dst.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,lfa+i*dsk.nob);
            DSMad(&dst,h1,1,te+j*dst.nob,ck+i*dst.nob);
        }
    }

    x=memcmp(ck,ww,dst.nob*noc);   // ck should be the same as ww
    if(x!=0) return 0;
    return 1;
}

int increm(int * a, int len, int toobig)
{
    int x;
//  first find the last x such a[x] can be incremented
    x=0;
    while(1)
    {
        x++;
        if( (a[len-x]) < (toobig-x) ) break;
        if(x==len) return 1;
    }
//  increment it
    a[len-x]++;
//  set the rest to increasing values
    while(x>0)
    {
        x--;
        a[len-x]=a[len-x-1]+1;
    }
    return 0;
}

void startit(int * a, int len)
{
    int x;
    for(x=0;x<len;x++) a[x]=x;
}

int main(int argc,  char **argv)
{

    uint64_t fdefp,norp;
    uint64_t fdefq;
    uint64_t x,col;
    int *vn1;
    int *vn2;
    FELT fel;
    long i,j;
    Dfmt * lsv;
    Dfmt *m1,*m2;

    if(argc!=4)
    {
        printf("usage als1 charc noc nor-matrices\n");
        exit(42);
    }
    charc=atoi(argv[1]);
    noc=atoi(argv[2]);
    nor=atoi(argv[3]);
    printf("als1 starting search - charc %d noc %lu nor %d\n",charc,noc,nor);
    fdefp=charc;
    f = malloc(FIELDLEN);
    FieldASet(fdefp,f);
    DSSet(f,noc,&ds);
    norp=0;
    fdefq=1;
    for(i=0;i<noc;i++)
    {
        norp+=fdefq;
        fdefq=fdefq*fdefp;
    }
    lsv=malloc(ds.nob*norp);
    m1=malloc(ds.nob*nor);
    m2=malloc(ds.nob*nor);
    i=-1;
    while((i+1)!=norp)
    {
        i++;
        if(i==0) memset(lsv,0,ds.nob);
          else   memcpy(lsv+i*ds.nob,lsv+(i-1)*ds.nob,ds.nob);
        x=noc-1;
        while(1)
        {
            if(i==0)
            {
                DPak(&ds,x,lsv+i*ds.nob,1);
                break;
            }
            fel=DUnpak(&ds,x,lsv+i*ds.nob);
            DPak(&ds,x,lsv+i*ds.nob,0);
            col=DNzl(&ds,lsv+i*ds.nob);
            if(col==ZEROROW)
            {
                x--;
                DPak(&ds,x,lsv+i*ds.nob,1);
                break;
            }
            fel++;
            if(fel<fdefp)
            {
                DPak(&ds,x,lsv+i*ds.nob,fel);
                break;
            }
            x--;
        }
        for(j=0;j<noc;j++)
            printf("%ld ",DUnpak(&ds,j,lsv+i*ds.nob));
        printf("\n");
    }
    printf("als1 field %ld with list of %ld vectors\n",fdefq,norp);
    vn1=malloc(nor*sizeof(int));
    vn2=malloc(nor*sizeof(int));
    startit(vn1,nor);
for(j=0;j<nor;j++) printf("%d ",vn1[j]);
printf("\n");
    startit(vn2,nor);
    x=0;
    dimt=noc*noc;
    DSSet(f,dimt,&dst);

    fq = malloc(FIELDLEN);
    FieldASet(fdefq,fq);
    DSSet(f,nor,&dsk);
    teg=malloc(dst.nob*nor);
    tek=malloc(dsk.nob*nor);
    ex=malloc(dsk.nob*noc);
    lfa=malloc(dsk.nob*noc);
    lfb=malloc(ds.nob*nor);
    ck=malloc(dst.nob*noc);
    te=malloc(dst.nob*nor);
    ww=malloc(dst.nob*noc);
    while(1)
    {
        x++;
        for(j=0;j<nor;j++)
            memcpy(m1+j*ds.nob,lsv+vn1[j]*ds.nob,ds.nob);
        for(j=0;j<nor;j++)
            memcpy(m2+j*ds.nob,lsv+vn2[j]*ds.nob,ds.nob);
        j=checkit(m1,m2);
        if(j==1)
        {
            printf("wow, found one\n");
            exit(21);
        }
        i=increm(vn2,nor,norp);
        if(i==0) continue;
        i=increm(vn1,nor,norp);
for(j=0;j<nor;j++) printf("%d ",vn1[j]);
printf("\n");
        if(i!=0) break;
        startit(vn2,nor);
    }

    printf("Search far completed, %ld cases considered\n",x);
    return 0;
}

/* end of als1.c */
