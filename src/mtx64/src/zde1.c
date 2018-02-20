/*
      zde1.c     meataxe-64 Simple Determinant program
      ======     R. A. Parker 25.08.15
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    uint64_t hdr[5];
    EFIL *e;
    FELT det,fel;
    uint64_t fdef,dim,i,j,col;
    DSPACE ds;
    FIELD * f;
    Dfmt *m;
    Dfmt *vw;
    Dfmt *v;
    Dfmt *vpt;
    char st[200];
    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage zde <input>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    dim=hdr[2];
    if (hdr[2]!=hdr[3]) 
    {
        LogString(80,"Determinant of non-square matrix");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    m=malloc(ds.nob*dim);   // Input matrix
    vw=malloc(ds.nob);      // temp for swapping
    ERData(e,ds.nob*dim,m);  // read in entire matrix
    ERClose(e);
    det=1;

    for(i=0;i<dim;i++)         // make top left i x i = 1.
    {
        v=DPAdv(&ds,i,m);      // set up main pointer to i'th row
        for(j=i;j<dim;j++)     // find non-zero entry in col i
        {
            vpt=DPAdv(&ds,j,m);
            col=DNzl(&ds,vpt);
            if(col==i) break;
        }
        if(j==dim)
        {
            LogString(80,"Determinant is zero");
            return 0;
        }
        if(j!=i)               // swap i and j rows round
        {
            vpt=DPAdv(&ds,j,m);
            DCpy(&ds,vpt,1,vw);
            DCpy(&ds,v,1,vpt);
            DCpy(&ds,vw,1,v);
            if( ((j-1)%2)==1 ) det=FieldNeg(f,det);
        }
        fel=DUnpak(&ds,i,v);  // make leading term 1
        det=FieldMul(f,det,fel);
        fel=FieldInv(f,fel);
        DSMul(&ds,fel,1,v);
        for(j=i+1;j<dim;j++)       // clear out that column
        {
            vpt=DPAdv(&ds,j,m);
            fel=DUnpak(&ds,i,vpt);
            fel=FieldNeg(f,fel);
            DSMad(&ds,fel,1,v,vpt);
        }
    }
    sprintf(st,"Determinant of %s is %ld",argv[1],det);
    printf("%s\n",st);
    LogString(20,st);
    free(f);
    free(m);
    free(vw);
    return 0;                  // job done
}

/*  end of zde1.c    */
