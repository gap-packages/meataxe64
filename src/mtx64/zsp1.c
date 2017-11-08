/*
      zsp1.c     meataxe-64 simple split program
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
    uint64 hdr[5];
    EFIL *e,*e1;
    int vec,rank,j,i,readvec,dimvec;
    uint64 fdef,dim,col;
    FIELD * f;
    DSPACE ds;
    DSPACE dso;     // output subspace
    Dfmt * v1;      // new vector
    Dfmt * v2;      // output vector
    Dfmt * vp1;     // pointer
    Dfmt * vp2;     // another pointer
    Dfmt ** m;      // generators
    Dfmt * m1;      // invariant subspace
    char fn[200];
    long pfxlen,g,ngens,k,dig;
    int * piv;
    int * nonpiv;
    FELT fel;
    char st[200];

    LogCmd(argc,argv);
    if (argc != 6) 
    {
        LogString(80,"usage zsp <vec> <genpfx> #gens <subpfx> <quotpfx>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);    // vector
    fdef=hdr[1];
    dim=hdr[3];
    dimvec=hdr[2];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    v1=malloc(ds.nob);            // work vector
    v2=malloc(ds.nob);            // another work vector
    ERData(e1,ds.nob,v1);          // put first vector in
    ERClose(e1);
    readvec=1;
    ngens=atoi(argv[3]);
    m=malloc(ngens*sizeof(Dfmt *));  // Array of pointers for generators
    strcpy(fn,argv[2]);              // input prefix
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
    nonpiv=malloc(dim*sizeof(int));
    m1=malloc(ds.nob*dim);        // invariant subspace (semicleaned)
    g=ngens-1;
    vec=-1;
    rank=0;
    while(1)                // make invariant subspace
    {
        for(j=0;j<rank;j++)  // clean v1
        {
            fel=DUnpak(&ds,piv[j],v1);
            DSMad(&ds,fel,1,m1+j*ds.nob,v1);
        }
        col=DNzl(&ds,v1);
        if(col!=ZEROROW)
        {
            piv[rank]=col;
            fel=DUnpak(&ds,col,v1);
            fel=FieldInv(f,fel);
            fel=FieldNeg(f,fel);
            DSMul(&ds,fel,1,v1);
            DCpy(&ds,v1,1,m1+rank*ds.nob);
            rank++;
            if(rank==dim)
            {
                printf("Whole Space\n");
                LogString(20,"Whole Space");
                free(f);
                for(g=0;g<ngens;g++) free(m[g]);
                free(m);
                free(piv);
                free(v1);
                free(v2);
                free(m1);
                free(nonpiv);
                return 0;
            }
        }
//  get next vector into v1
//  read it if readvec<dimvec
        if(readvec<dimvec)
        {
            ERData(e1,ds.nob,v1);          // read vector
            readvec++;
            continue;
        }
        g++;                 // make next vector
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
    sprintf(st,"Subspace dim %ld, Quotient dim %ld",(long)rank,(long)(dim-rank));
    printf("%s\n",st);
    LogString(20,st);
    ERClose(e1);
//  Invariant subspace now in m1.
//  'rank' holds its dimension.
//  Make and output action on subspace
    strcpy(fn,argv[4]);              // subspace prefix
    pfxlen=0;
    while(fn[pfxlen]!='\0') pfxlen++;
    hdr[2]=rank;
    hdr[3]=rank;
    DSSet(f,rank,&dso);
    for(g=0;g<ngens;g++)
    {
/*  make the subspace filename in fn  */
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
        e = EWHdr(fn,hdr);    // generator g
        for(vec=0;vec<rank;vec++)
        {
            memset(v2,0,dso.nob);  // clear output vector
            memset(v1,0,ds.nob);   // make vector in v1
            vp1=m1+vec*ds.nob;
            for(j=0;j<dim;j++)   
            {
                vp2=DPAdv(&ds,j,m[g]);
                fel=DUnpak(&ds,j,vp1);
                DSMad(&ds,fel,1,vp2,v1);
            }
            for(j=0;j<rank;j++)  // clean v1
            {
                fel=DUnpak(&ds,piv[j],v1);
                DSMad(&ds,fel,1,m1+j*ds.nob,v1);
                fel=FieldNeg(f,fel);
                DPak(&dso,j,v2,fel);
            }
            EWData(e,dso.nob,v2);
        }
        EWClose(e);
    }
//  make and output action on quotient space
    strcpy(fn,argv[5]);              // quotient space prefix
    pfxlen=0;
    while(fn[pfxlen]!='\0') pfxlen++;
    hdr[2]=dim-rank;
    hdr[3]=dim-rank;
    DSSet(f,dim-rank,&dso);
// find the non-pivots
    k=0;
    for(i=0;i<dim;i++)     // is i a non-pivot
    {
        for(j=0;j<rank;j++)
            if(piv[j]==i) break;
        if(j==rank) nonpiv[k++]=i;
    }
    for(g=0;g<ngens;g++)
    {
/*  make the quotient space filename in fn  */
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
        e = EWHdr(fn,hdr);    // generator g
        for(vec=0;vec<dim-rank;vec++)
        {
            memcpy(v1,m[g]+nonpiv[vec]*ds.nob,ds.nob);  //nonpivotal row of generator
            for(j=0;j<rank;j++)  // clean it
            {
                fel=DUnpak(&ds,piv[j],v1);
                DSMad(&ds,fel,1,m1+j*ds.nob,v1);
            }
            memset(v2,0,dso.nob);  // clear output vector
            for(j=0;j<dim-rank;j++)  // get insignificant entries
            {
                fel=DUnpak(&ds,nonpiv[j],v1);
                DPak(&ds,j,v2,fel);
            }
            EWData(e,dso.nob,v2);
        }
        EWClose(e);
    }
    free(f);
    for(g=0;g<ngens;g++) free(m[g]);
    free(m);
    free(piv);
    free(v1);
    free(v2);
    free(m1);
    free(nonpiv);
    return 0;
}

/*  end of zsp1.c    */
