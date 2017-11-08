/*
      zsb1.c     meataxe-64 simple standard base program
      ======     R. A. Parker 24.08.15
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    uint64 fdef,dim,j,col;
    long g,vec,k,rank;
    int ngens,pfxlen,dig;
    EFIL *e;
    FIELD * f;
    FELT fel;
    DSPACE ds;
    uint64 hdr[5];
    Dfmt ** m;
    char fn[200];
    int * piv;
    Dfmt * m1;      // standard base
    Dfmt * m2;      // semicleaned standard base
    Dfmt * v1;      // new standard vector
    Dfmt * v2;      // cleaned new standard vector
    Dfmt * vp1;     // vector being multiplied.
    Dfmt * vp2;     // matrix row to Mad in
        
    LogCmd(argc,argv);
    if (argc != 5) 
    {
        LogString(80,"usage zsb <vec> <genpfx> #gens <output>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);    // vector
    fdef=hdr[1];
    dim=hdr[3];
    if (hdr[2]!=1) 
    {
        LogString(80,"Only one input vector allowed in zsb");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    v1=malloc(ds.nob);            // work vector
    v2=malloc(ds.nob);            // cleaned work vector
    ERData(e,ds.nob,v1);          // put first vector in
    ERClose(e);
    ngens=atoi(argv[3]);
    m=malloc(ngens*sizeof(Dfmt *));
    strcpy(fn,argv[2]);
    pfxlen=0;
    while(fn[pfxlen]!='\0') pfxlen++;
    for(g=0;g<ngens;g++)
    {
/*  make the filename in fn  */
        k=g+1;
        dig=1;
        while(k>9)
        {
            dig++;
            k=k/10;
        }
        fn[pfxlen+dig]='\0';
        k=g+1;
        while(k>0)
        {
            fn[pfxlen+dig-1]='0'+(k%10);
            dig--;
            k=k/10;
        }
        m[g]=malloc(ds.nob*dim);
        e = ERHdr(fn,hdr);    // generator
        if( (hdr[1]!=fdef) || (hdr[2]!=dim) || (hdr[3]!=dim) )
        {
            printf("Matrices Incompatible\n");
            LogString(90,"Matrices Incompatible");
            return 18;
        }
        ERData(e,ds.nob*dim,m[g]);
        ERClose(e);
    }
    piv=malloc(dim*sizeof(int));
    m1=malloc(ds.nob*dim);        // standard base
    m2=malloc(ds.nob*dim);        // semicleaned standard base
    g=ngens-1;
    vec=-1;
    rank=0;
    while(1)
    {
//  copy v1 to v2 ready for cleaning
        memcpy(v2,v1,ds.nob);
//  clean v2 with m2
        for(j=0;j<rank;j++)
        {
            fel=DUnpak(&ds,piv[j],v2);
            DSMad(&ds,fel,1,m2+j*ds.nob,v2);
        }
//  if it isn't zero, include it in m1 and m2
        col=DNzl(&ds,v2);
        if(col!=ZEROROW)
        {
            piv[rank]=col;
            fel=DUnpak(&ds,col,v2);
            fel=FieldInv(f,fel);
            fel=FieldNeg(f,fel);
            DSMul(&ds,fel,1,v2);
            DCpy(&ds,v1,1,m1+rank*ds.nob);
            DCpy(&ds,v2,1,m2+rank*ds.nob);
            rank++;
        }
//  check to see if we've finished
//  make next vector
        g++;
        if(g>=ngens)
        {
            g=0;
            vec++;
        }
        if(vec>=rank) break;
        memset(v1,0,ds.nob);
        vp1=m1+vec*ds.nob;
        for(j=0;j<dim;j++)   
        {
            vp2=DPAdv(&ds,j,m[g]);
            fel=DUnpak(&ds,j,vp1);
            DSMad(&ds,fel,1,vp2,v1);
        }
    }
    e = EWHdr(argv[4],hdr);    // output the resulting standard base
    EWData(e,ds.nob*dim,m1);
    EWClose(e);
    free(f);
    for(g=0;g<ngens;g++) free(m[g]);
    free(m);
    free(piv);
    free(v1);
    free(v2);
    free(m1);
    free(m2);
    return 0;
}

/*  end of zsb1.c    */
