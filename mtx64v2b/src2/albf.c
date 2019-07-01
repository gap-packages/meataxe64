/*
      albf.c     meataxe-64 Albrecht linear forms
      ======     R. A. Parker    6.10.2017
*/

/* input is matrix with d columns and n>d rows */
/* the columns correspond to the powers of x, zeroth first */
/* the rows are the linear forms we propose to make */

/* output is the matrix of the linear forms */
/* we need to do to assemble the output     */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"

int main(int argc,  char **argv)
{
    uint64_t fdef,fdefq,nor,noc,dimt,col;
    FIELD *f,*fq;
    DSPACE ds,dst,dsk;
    EFIL *e1,*e2,*e3;
    uint64_t hdr[5];
    Dfmt *m1,*m2,*te,*ww,*teg,*tek,*ex,*lfa,*lfb,*ck;
    uint64_t i,j,j1,j2;
    FELT h1,h2,h3,h4,h5;
    uint64_t piv[400];
    int x;

    if(argc!=4)
    {
        printf("usage albf in1 in2 out\n");
        exit(42);
    }
    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);

    m1=malloc(ds.nob*nor);
    ERData(e1,ds.nob*nor,m1);
    ERClose(e1);
    if(argc>2) e2=ERHdr(argv[2],hdr);  // allow two . . .
     else      e2=ERHdr(argv[1],hdr);  // or one input matrix
    if( (fdef!=hdr[1]) || (nor!=hdr[2]) || (noc!=hdr[3]) )
    {
        printf("Matrices incompatible\n");
        exit(14);
    }
    m2=malloc(ds.nob*nor);
    ERData(e2,ds.nob*nor,m2);
    ERClose(e2);

//  compute (te) as the tensor products of the rows of m1 and m2
//      a row of te is therefore noc*noc long
//
    dimt=noc*noc;
    DSSet(f,dimt,&dst);
    te=malloc(dst.nob*nor);
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

    ww=malloc(dst.nob*noc);
    memset(ww,0,dst.nob*noc);
    fq = malloc(FIELDLEN);
    fdefq=1;
    for(i=0;i<noc;i++) fdefq=fdefq*f->charc;
    FieldASet(fdefq,fq);
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
            printf("Tensors not linearly independent\n");
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

//
// Extract the pivots of tensors from the wanted matrix
//

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

//
//  Not sure what lfa is either
//

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

//
//  This seems to be a transpose
//  lfb is the answer we seek

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

//
//  Check that it works
//

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

    x=memcmp(ck,ww,dst.nob*noc);   // ck should be the same as ww
    if(x!=0)
    {
        printf("No expression exists\n");
        exit(16);
    }

    e3=EWHdr(argv[3],hdr);
    EWData(e3,ds.nob*nor,lfb);
    EWClose(e3);

    return 0;
}


/* end of albf.c */
