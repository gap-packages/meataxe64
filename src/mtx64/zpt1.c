/*
      zpt1.c     meataxe-64 simple pre-thrash program
      ======     R. A. Parker 5.10.16
*/

#define MAXSPACE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"

int main(int argc,  char **argv)
{
    uint64 hdr[5];
    uint64_t wdone[MAXSPACE];
    uint64_t piv[MAXSPACE];
    EFIL *e1;
    uint64 fdef,noc,nvec,ngens,dim,nextgen,tvec;
    uint64_t i,j,col,k,dig;
    FIELD * f;
    FELT fel;
    DSPACE ds;
    Dfmt *mnew,*morb,*mgau,*v1;
    char fn[200];
    uint64_t pfxlen;

    LogCmd(argc,argv);
    if (argc != 5) 
    {
        LogString(80,"usage zpt <startspace> <genpfx> #gens <space>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);    // startspace
    fdef=hdr[1];
    nvec=hdr[2];              // dim startspace
    noc=hdr[3];
    tvec=0;
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    morb=malloc(MAXSPACE*ds.nob);   // invariant subspace orbital
    mgau=malloc(MAXSPACE*ds.nob);   // Gaussian invariant subspace
    mnew=malloc(MAXSPACE*ds.nob);   // new vectors
     v1 =malloc(MAXSPACE*ds.nob);   // working vector
    if(nvec>MAXSPACE)
    {
        LogString(80,"Too many input vectors");
// maybe copy input to output
        exit(15);
    }
    ERData(e1,nvec*ds.nob,mnew);
    ERClose(e1);
    ngens=atoi(argv[3]);
    dim=0;
    nextgen=1;
    for(i=1;i<=ngens;i++) wdone[i]=0;
    strcpy(fn,argv[2]);              // input prefix
    pfxlen=0;
    while(fn[pfxlen]!='\0') pfxlen++;
    while(1)
    {
        if(nvec!=tvec)
        {
            memcpy(v1,mnew+tvec*ds.nob,ds.nob);
            for(i=0;i<dim;i++)
            {
                fel=DUnpak(&ds,piv[i],v1);
                DSMad(&ds,fel,1,mgau+i*ds.nob,v1);
            }
            col=DNzl(&ds,v1);
            if(col!=ZEROROW)
            {
                piv[dim]=col;
                fel=DUnpak(&ds,col,v1);
                fel=FieldInv(f,fel);
                fel=FieldNeg(f,fel);
                DSMul(&ds,fel,1,v1);
                DCpy(&ds,v1,1,mgau+dim*ds.nob);
                DCpy(&ds,mnew+tvec*ds.nob,1,morb+dim*ds.nob);
                dim++;
                if(dim>=MAXSPACE) break;

            }
            tvec++;
            continue;
        }
        if( (dim+(dim-wdone[nextgen]))>MAXSPACE) break;
        if(dim==wdone[nextgen])
        {
            sprintf(fn,"Invariant subspace dimension %lu",dim);
            printf("%s\n",fn);
            LogString(20,fn);
            break;
        }

/*  make the filename in fn  */
        k=nextgen;
        dig=1;
        while(k>9)
        {
            dig++;
            k=k/10;
        }
        fn[pfxlen+dig]='\0';
        k=nextgen;
        while(k>0)
        {
            fn[pfxlen+dig-1]='0'+(k%10);
            dig--;
            k=k/10;
        }
        e1 = ERHdr(fn,hdr);    // generator
        if( (hdr[1]!=fdef) || (hdr[2]!=noc) || (hdr[3]!=noc) )
        {
            printf("Matrices Incompatible\n");
            LogString(90,"Matrices Incompatible");
            return 18;
        }
        memset(mnew,0,(dim-wdone[nextgen])*ds.nob);
// read in one row at a time and make the new vectors
        for(i=0;i<noc;i++)
        {
            ERData(e1,ds.nob,v1);
            for(j=wdone[nextgen];j<dim;j++)
            {
                fel=DUnpak(&ds,i,morb+j*ds.nob);
                DSMad(&ds,fel,1,v1,mnew+(j-wdone[nextgen])*ds.nob);
            }
        }
        ERClose(e1);
        nvec=dim-wdone[nextgen];
        tvec=0;
        wdone[nextgen]=dim;
        nextgen++;
        if(nextgen>ngens) nextgen=1;
    }
    hdr[2]=dim;
    e1 = EWHdr(argv[4],hdr);
    EWData(e1,dim*ds.nob,morb);
    EWClose(e1);
    printf("Next generator is %lu\n",nextgen);
    return 0;
}
/*    end of zpt1.c   */
