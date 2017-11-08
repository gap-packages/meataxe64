/*
      ziv1.c     meataxe-64 Simple Invert program
      ======     R. A. Parker 23.08.15
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64 hdr[5];
    uint64 fdef,dim,i,j,col;
    FIELD * f;
    DSPACE ds;
    Dfmt *m1,*m2;
    Dfmt *vw;
    Dfmt *v1,*v2,*v1a,*v2a;
    FELT fel;

    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage ziv <input> <inverse>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    dim=hdr[2];
    if (hdr[2]!=hdr[3]) 
    {
        LogString(80,"Inverting non-square matrix");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    m1=malloc(ds.nob*dim);   // Input matrix
    m2=malloc(ds.nob*dim);   // Inverse
    vw=malloc(ds.nob);       // vector work area
    ERData(e1,ds.nob*dim,m1);  // read in entire matrix
    ERClose(e1);
    memset(m2,0,ds.nob*dim);   // zeroize inverse
    v2=m2;
    for(i=0;i<dim;i++)
    {
        DPak(&ds,i,v2,1);      // set matrix 2 as identity
        v2=DPInc(&ds,v2);
    }
    for(i=0;i<dim;i++)         // make top left i x i = 1.
    {
        v1=DPAdv(&ds,i,m1);      // set up main pointers to i'th row
        v2=DPAdv(&ds,i,m2);
        for(j=i;j<dim;j++)     // find non-zero entry in col i
        {
            v1a=DPAdv(&ds,j,m1);
            col=DNzl(&ds,v1a);
            if(col==i) break;
        }
        if(j==dim)
        {
            LogString(80,"Matrix not invertible");
            free(f);
            free(m1);
            free(m2);
            free(vw);
            exit(16);
        }
        if(j!=i)               // swap i and j rows round
        {
            v1a=DPAdv(&ds,j,m1);
            v2a=DPAdv(&ds,j,m2);
            DCpy(&ds,v1a,1,vw);
            DCpy(&ds,v1,1,v1a);
            DCpy(&ds,vw,1,v1);
            DCpy(&ds,v2a,1,vw);
            DCpy(&ds,v2,1,v2a);
            DCpy(&ds,vw,1,v2);
        }
        fel=DUnpak(&ds,i,v1);  // make leading term 1
        fel=FieldInv(f,fel);
        DSMul(&ds,fel,1,v1);
        DSMul(&ds,fel,1,v2);
        for(j=0;j<dim;j++)       // clear out that column
        {
            if(j==i) continue;   // except the entry itself
            v1a=DPAdv(&ds,j,m1);
            v2a=DPAdv(&ds,j,m2);
            fel=DUnpak(&ds,i,v1a);
            fel=FieldNeg(f,fel);
            DSMad(&ds,fel,1,v1,v1a);
            DSMad(&ds,fel,1,v2,v2a);
        }
    }
    e2=EWHdr(argv[2],hdr);     // same parameters as input
    EWData(e2,ds.nob*dim,m2);  // write out entire matrix
    EWClose(e2);               // close the output
    free(f);
    free(m1);
    free(m2);
    free(vw);
    return 0;                  // job done
}

/*  end of ziv1.c    */
