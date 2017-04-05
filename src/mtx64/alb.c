/*
      alb.c     meataxe-64 Albrecht linear forms
      =====     R. A. Parker    27.6.2016
*/

/* input is matrix with d columns and n>d rows */
/* the columns correspond to the powers of x, zeroth first */
/* the rows are the linear forms we propose to make */

//  uncomment this if you want no output unless it works
//#define SEARCH 1

//  uncomment this if you want loads of output for debugging etc.
//#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"


void printmatrix(Dfmt * m, DSPACE * ds, uint64_t nor)
{
    uint64 i,j;
    FELT e;
    for(i=0;i<nor;i++)
    {
        for(j=0;j<ds->noc;j++)
        {
            e=DUnpak(ds,j,m+i*ds->nob);
            printf("%2d",(int)e);
        }
        printf("\n");
    }
    printf("\n");
}

void print3tens(Dfmt * m, DSPACE * ds, uint64_t nor)
{
    uint64 i,j,k,nod;
    FELT e;
    nod=0;
    while(nod*nod<ds->noc) nod++;
    if((nod*nod)!=ds->noc)
    {
        printf("Matrix not a tensor - dim = %d\n",(int)ds->noc);
        exit(13);
    }
    for(i=0;i<nor;i++)
    {
        for(j=0;j<nod;j++)
        {
            for(k=0;k<nod;k++)
            {
                e=DUnpak(ds,j*nod+k,m+i*ds->nob);
                printf("%2d",(int)e);
            }
            printf("\n");
        }
        printf("\n");
    }
}

uint64_t mkprog(Dfmt * m, DSPACE * ds, uint64_t nor, uint64_t * prog)
{
    uint64_t bsf,bid,i,j,k,col,coli,colj,pct;
    FELT h1,h2,h3;
    uint64_t cmd[4];
    Dfmt * v1;
    v1=malloc(ds->nob);
    pct=0;
    while(1)
    {
        bsf=0;          // no moves found so far
        for(i=0;i<nor;i++)
        {
            for(j=0;j<nor;j++)
            {
// consider moves that change i using j
                if(i==j)
                {
// single row.  Maybe scale it to make it clean
                    col=DNzl(ds,m+i*ds->nob);
                    if(col==ZEROROW) continue;
                    if(col!=i) continue;
                    h1=DUnpak(ds,i,m+i*ds->nob);
                    if(h1==1) continue;
                    bid=nor-i;
                    if(bid<=bsf) continue;
                    bsf=5000-i;
                    h2=FieldInv(ds->f,h1);
                    cmd[0]=6;    // scale
                    cmd[1]=i;    // row i
                    cmd[2]=h2;   // by h2
                    cmd[3]=0;
                    continue; 
                }
// else i and j are different
                coli=DNzl(ds,m+i*ds->nob);
                colj=DNzl(ds,m+j*ds->nob);
                if(colj==ZEROROW) continue;
                if( ((coli>i)||(coli==ZEROROW)) && (colj==i) )
                {
                    h1=DUnpak(ds,colj,m+j*ds->nob);
                    h3=FieldInv(ds->f,h1);
                    bid=10000 - i;
                    if(bid<=bsf) continue;
                    bsf=bid;
                    cmd[0]=7;    // scale
                    cmd[1]=j;    // row j
                    cmd[2]=h3;   // by h3
                    cmd[3]=i;    // and add into row i
                    continue; 
                }
                if(coli==ZEROROW) continue;
                if(coli>colj) continue;
                if(coli<colj)
                {
                    if(coli!=i) continue;
                    h1=DUnpak(ds,coli,m+i*ds->nob);
                    if(h1!=1) continue;
                    for(k=coli+1;k<colj;k++)
                    {
                        h1=DUnpak(ds,k,m+i*ds->nob);
                        if(h1!=0) break;
                    }
                    if(k!=colj) continue;
                    h1=DUnpak(ds,colj,m+i*ds->nob);
                    if(h1==0) continue;
                    h2=DUnpak(ds,colj,m+j*ds->nob);
                    h3=FieldNeg(ds->f,h2);
                    h3=FieldDiv(ds->f,h1,h3);
                    memcpy(v1,m+i*ds->nob,ds->nob);
                    DSMad(ds,h3,1,m+j*ds->nob,v1);
                    for(k=colj;k<ds->noc;k++)
                    {
                        h1=DUnpak(ds,k,v1);
                        if(h1!=0) break;
                    }
                    bid=((k-colj) * 3000)-i;
                    if(bid<=bsf) continue;
                    bsf=bid;
                    cmd[0]=7;    // scale
                    cmd[1]=j;    // row j
                    cmd[2]=h3;   // by h3
                    cmd[3]=i;    // and add into row i
                    continue;
                }
                if(colj==coli)
                {
                    h1=DUnpak(ds,coli,m+i*ds->nob);
                    h2=DUnpak(ds,coli,m+j*ds->nob);
                    h3=FieldNeg(ds->f,h2);
                    h3=FieldDiv(ds->f,h1,h3);
                    memcpy(v1,m+i*ds->nob,ds->nob);
                    DSMad(ds,h3,1,m+j*ds->nob,v1);
                    col=DNzl(ds,v1);
                    if(col==ZEROROW) col=ds->noc;
                    if(col>i) continue;
                    bid=((col-coli) * 3000) - i;
                    if(bid<=bsf) continue;
                    bsf=bid;
                    cmd[0]=7;    // scale
                    cmd[1]=j;    // row j
                    cmd[2]=h3;   // by h3
                    cmd[3]=i;    // and add into row i
                    continue;
                }
            }
        }
        if(bsf==0) return pct/4;
        prog[pct++]=cmd[0];
        prog[pct++]=cmd[1];
        prog[pct++]=cmd[2];
        prog[pct++]=cmd[3];
        if(cmd[0]==6)
        {
            DSMul(ds,cmd[2],1,m+cmd[1]*ds->nob);
        }
        if(cmd[0]==7)
        {
            DSMad(ds,cmd[2],1,m+cmd[1]*ds->nob,m+cmd[3]*ds->nob);
        }
#ifdef DEBUG
        printmatrix(m,ds,nor);
#endif
    }
}

void pgout(DSPACE * ds, uint64_t * prog, uint64_t steps, FILE * ff)
{
    uint64_t i,j,cd,op1,op2,op3,len;
    const FIELD * f;
    f=ds->f;
    fprintf(ff,"uint8_t lfZxx[] = {");
    for(i=0;i<steps;i++)
    {
        j=4*i;
        cd=prog[j];
        len=0;
        if(cd==7)   // Multiply and add
        {
            if((prog[j+2]+1)==f->charc)
            {
                cd=2;
                op1=prog[j+3];
                op2=prog[j+1];
                op3=prog[j+1];
                len=4;
            }
            if( (len==0) && (prog[j+2]==1) )
            {
                cd=3;
                op1=prog[j+3];
                op2=prog[j+1];
                op3=prog[j+1];
                len=4;
            }
            if(len==0)
            {
                cd=7;
                op1=prog[j+2];
                op2=prog[j+3];
                op3=prog[j+1];
            }
        }
        if(len==0)
        {
            printf("Error in code generation\n");
            exit(17);
        }

        if(len==4) fprintf(ff," %lu,%lu,%lu,%lu ,",cd,op1,op2,op3);
        if(i%5==4) fprintf(ff,"\n                   ");
    }
    fprintf(ff," 0 };\n");
}


int main(int argc,  char **argv)
{
    uint64_t fdef,fdefq,nor,noc,dimt,col,steps;
    uint64_t hdr[5];
    uint64_t i,j,j1,j2;
    uint64_t piv[400];
    uint64_t prog[1000];
    FILE * ff;
    int x;
    FIELD *f,*fq;
    EFIL *e1,*e2;
    DSPACE ds,dst,dsk;
    Dfmt *m1,*m2,*te,*teg,*tek,*ww,*ex,*lfa,*ck,*lfb;
    FELT h1,h2,h3,h4,h5;


    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc,&ds);
//
// Read into (m1,m2) the two tensor matrices - often equal
//
    m1=malloc(ds.nob*nor);
    ERData(e1,ds.nob*nor,m1);
    ERClose(e1);
#ifdef DEBUG
    printmatrix(m1,&ds,nor);
#endif

    if(argc>2) e2=ERHdr(argv[2],hdr);
     else      e2=ERHdr(argv[1],hdr);
    if( (fdef!=hdr[1]) || (nor!=hdr[2]) || (noc!=hdr[3]) )
    {
        printf("Matrices incompatible\n");
        exit(14);
    }
    m2=malloc(ds.nob*nor);
    ERData(e2,ds.nob*nor,m2);
#ifdef DEBUG   
    printmatrix(m2,&ds,nor);
#endif
//
//  compute (te) as the tensor products of the rows of m1 and m2
//      a row of te is therefore noc*noc long
//
    dimt=noc*noc;
    DSSet(f,dimt,&dst);
    te=malloc(dst.nob*nor);
    memset(te,0,dst.nob*nor);
    for(i=0;i<nor;i++)
        for(j1=0;j1<noc;j1++)
          for(j2=0;j2<noc;j2++)
          {
              h1=DUnpak(&ds,j1,m1+i*ds.nob);
              h2=DUnpak(&ds,j2,m2+i*ds.nob);
              h3=FieldMul(f,h1,h2);
              DPak(&dst,j1*noc+j2,te+i*dst.nob,h3);
          }
#ifdef DEBUG
    print3tens(te,&dst,nor);
#endif

//
//  compute the matrix (ww) of what we want to compute
//

    ww=malloc(dst.nob*noc);
    memset(ww,0,dst.nob*noc);
    fq = malloc(FIELDLEN);
    fdefq=1;
    for(i=0;i<noc;i++) fdefq=fdefq*f->charc;
    FieldSet(fdefq,fq);
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
#ifdef DEBUG
    print3tens(ww,&dst,noc);
    printmatrix(te,&dst,nor);
    printmatrix(ww,&dst,noc);
#endif
//
//  Put the tensor matrix into echelon form
//
    teg=malloc(dst.nob*nor);
    memcpy(teg,te,dst.nob*nor);
//     set (tek) = keeptrack as the identity to start with
    DSSet(f,nor,&dsk);
    tek=malloc(dsk.nob*nor);
    memset(tek,0,dsk.nob*nor);
    for(i=0;i<nor;i++)  DPak(&dst,i,tek+i*dsk.nob,1);
    for(i=0;i<nor;i++)
    {
        col=DNzl(&dst,teg+i*dst.nob);
        if(col==ZEROROW)
        {
#ifndef SEARCH
            printf("Tensors not linearly independent\n");
#endif
            exit(15);
        }
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
#ifdef DEBUG
    printmatrix(teg,&dst,nor);
    printmatrix(tek,&dsk,nor);
#endif
    ex=malloc(dsk.nob*noc);
    memset(ex,0,dsk.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dst,piv[j],ww+i*dst.nob);
            DPak(&dsk,j,ex+i*dsk.nob,h1);
        }
    }
#ifdef DEBUG
    printmatrix(ex,&dsk,noc);
#endif
    lfa=malloc(dsk.nob*noc);
    memset(lfa,0,dsk.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,ex+i*dsk.nob);
            DSMad(&dsk,h1,1,tek+j*dsk.nob,lfa+i*dsk.nob);
        }
    }
#ifdef DEBUG
    printmatrix(lfa,&dsk,noc);
#endif
    ck=malloc(dst.nob*noc);
    memset(ck,0,dst.nob*noc);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,lfa+i*dsk.nob);
            DSMad(&dst,h1,1,te+j*dst.nob,ck+i*dst.nob);
        }
    }
#ifdef DEBUG
    printmatrix(ck,&dst,noc);
#endif
    x=memcmp(ck,ww,dst.nob*noc);
    if(x!=0)
    {
#ifndef SEARCH
        printf("No expression exists\n");
#endif
        exit(16);
    }
#ifdef SEARCH
    printf(" worked %d\n");
    printmatrix(m1,&ds,nor);
    return 0;
#endif
    lfb=malloc(ds.nob*nor);
    memset(lfb,0,ds.nob*nor);
    for(i=0;i<noc;i++)
    {
        for(j=0;j<nor;j++)
        {
            h1=DUnpak(&dsk,j,lfa+i*dsk.nob);
            DPak(&ds,i,lfb+j*ds.nob,h1);
        }
    }
#ifdef DEBUG
    printmatrix(lfb,&ds,nor);
#endif
#ifndef SEARCH
    ff=fopen("prog","wb");
    mkprog(m1,&ds,nor,prog);
    mkprog(m2,&ds,nor,prog);
    steps = mkprog(lfb,&ds,nor,prog);
    pgout(&ds,prog,steps,ff);
#endif
    
    return 0;
}

/* end of alb.c */
