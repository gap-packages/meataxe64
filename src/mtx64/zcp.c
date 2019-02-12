/*
      zcp.c     meataxe-64 simple characteristic polynomial
      =====     R. A. Parker 13.1.19
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "io.h"

int main(int argc,  char **argv)
{
    EFIL *e;
    uint64_t fdef,dim,gvcnt,oldgv,i,stpiv,col;
    uint64_t hdr[5];
    FIELD * f;
    DSPACE ds;
    DSPACE dsp;
    Dfmt * mx;  // input matrix whose polynomial is wanted
    Dfmt * gv;  // Gaussian vectors
    Dfmt * tm;  // transformation matrix
    Dfmt *v1,*v2, *v3;  // working vectors
    Dfmt *vp1;   // pointer - no memory allocated
    uint64_t * piv;  // pivot array row->piv
    char * hadpiv;
    FELT fel;
    Dfmt * polys;
    uint64_t polix;
    uint64_t * polystart;
    uint64_t * polydeg;
    uint64_t nopolys;

    LogCmd(argc,argv);
    if (argc != 3) 
    {
        LogString(80,"usage zcp <mtx> <poly>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);    // Input matrix
    fdef=hdr[1];
    dim=hdr[3];
    if (hdr[2]!=dim) 
    {
        LogString(80,"Only square matrices have a characteristic poly");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,2,&ds);      // degree 1 polynomial
    polys=malloc(ds.nob*dim);   // maximum size of polynomial list
    polystart=malloc(dim*sizeof(uint64_t));  // index of first byte
    polydeg=malloc(dim*sizeof(uint64_t));   // degrees
    DSSet(f,dim,&ds);
    mx=malloc(ds.nob*dim);
    ERData(e,ds.nob*dim,mx); // read input matrix
    ERClose(e);
    gv=malloc(ds.nob*dim);
    tm=malloc(ds.nob*dim);
    piv=malloc(dim*sizeof(uint64_t));
    hadpiv=malloc(dim);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    v3=malloc(ds.nob);
    gvcnt=0;
    for(i=0;i<dim;i++) hadpiv[i]=0;
    stpiv=0;
    nopolys=0;
    polix=0;
    while(gvcnt<dim)
    {
        oldgv=gvcnt;
        while(hadpiv[stpiv]!=0)stpiv++;   // make stpiv a non-pivot
        memset(v1,0,ds.nob);
        DPak(&ds,stpiv,v1,1);
        while(1)
        {
            memset(v2,0,ds.nob);
            DCpy(&ds,v1,1,v3);
            for(i=0;i<gvcnt;i++)    // clean new vector with old one
            {
                fel=DUnpak(&ds,piv[i],v1);
                DSMad(&ds,fel,1,gv+i*ds.nob,v1);
                DSMad(&ds,fel,1,tm+i*ds.nob,v2);
            }
//  if it isn't zero, include it in m1 and m2
            col=DNzl(&ds,v1);
            if(col==ZEROROW) break;
            piv[gvcnt]=col;
            hadpiv[col]=1;
            fel=DUnpak(&ds,col,v1);
            fel=FieldInv(f,fel);
            fel=FieldNeg(f,fel);
            DPak(&ds,gvcnt,v2,1);
            DSMul(&ds,fel,1,v1);
            DSMul(&ds,fel,1,v2);
            DCpy(&ds,v1,1,gv+gvcnt*ds.nob);
            DCpy(&ds,v2,1,tm+gvcnt*ds.nob);
            gvcnt++;
            memset(v1,0,ds.nob);
            for(i=0;i<dim;i++)    // multiply by generator
            {
                fel=DUnpak(&ds,i,v3);
                vp1=DPAdv(&ds,i,mx);
                DSMad(&ds,fel,1,vp1,v1);
            }
        }
        DSSet(f,gvcnt-oldgv+1,&dsp);
        polystart[nopolys]=polix; 
        memset(polys+polix,0,dsp.nob);
        DCut(&ds,1,oldgv,v2,&dsp,polys+polix);
        DPak(&dsp,gvcnt-oldgv,polys+polix,1);
        polydeg[nopolys]=gvcnt-oldgv;
        polix+=dsp.nob;
        nopolys++;
    }
//  all finished - now write out the partially factorized
//  characteristic polynomial

    hdr[0]=4;   // polynomial
    hdr[1]=fdef;
    hdr[2]=dim;
    hdr[3]=nopolys;
    hdr[4]=0;  // not used
    e=EWHdr(argv[2],hdr);
    for(i=0;i<nopolys;i++)
    {
        EWData(e,8,(uint8_t *)(polydeg+i));
        DSSet(f,polydeg[i]+1,&dsp);
        EWData(e,dsp.nob,polys+polystart[i]);
    }
    EWClose(e);

    free(f);
    free(mx);
    free(gv);
    free(tm);
    free(piv);
    free(v1);
    free(v2);
    free(v3);
    free(hadpiv);
    free(polys);
    free(polystart);
    free(polydeg);
}

/*  end of zcp.c    */
