/*
      zde2.c     meataxe-64 Testing SLEch program - temporary
      ======     R. A. Parker 24.02.18
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "slab.h"
 
int main(int argc,  char **argv)
{
    uint64_t hdr[5];
    EFIL *e;
    FELT det;
    uint64_t fdef,nor,noc,rank;
    uint64_t *rs, *cs;
    DSPACE ds;
    FIELD * f;
    Dfmt *a,*m,*c,*r;
    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage zde <input>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    if (nor!=noc) 
    {
        LogString(80,"Determinant of non-square matrix");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    a=malloc(ds.nob*nor);   // Input matrix
    m=malloc(SLSizeM(f,nor,noc));
    r=malloc(SLSizeR(f,nor,noc));
    c=malloc(SLSizeC(f,nor,noc));
    rs=malloc(16+((nor+63)/64)*8);
    cs=malloc(16+((noc+63)/64)*8);
    ERData(e,ds.nob*nor,a);  // read in entire matrix
    ERClose(e);
    rank=SLEch(&ds,a,rs,cs,&det,m,c,r,nor);
    det=FieldInv(f,det);
    if(rank%2==1) det=FieldMul(f,f->charc-1,det);
    if(rank==nor) printf("Determinant of %s is %ld\n",argv[1],det);
    else printf("Determinant is zero\n");
    free(f);
    free(a);
    free(m);
    free(c);
    free(r);
    return 0;                  // job done
}

/*  end of zde2.c    */
