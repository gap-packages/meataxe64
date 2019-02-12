/*
      zmp.c     meataxe-64 simple make permutation
      =====     R. A. Parker 23.08.18
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
    uint64_t fdef,dim,j,k;
    uint64_t vecs;
    FIELD * f;
    DSPACE ds;
    Dfmt * vl;    // vector list
    Dfmt * v;
    Dfmt *vp1,*vp2;
    Dfmt * m;
    FELT fel;
//------------------------------------
    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zmp <veclist> <matrix> <permutation>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);    // vector list
    fdef=hdr[1];
    dim=hdr[3];
    vecs=hdr[2];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    vl=malloc(vecs*ds.nob);
    ERData(e,vecs*ds.nob,vl);          // read in vector list
    ERClose(e);
    e = ERHdr(argv[2],hdr);    // generator
    if( (hdr[1]!=fdef) || (hdr[2]!=dim) || (hdr[3]!=dim) )
    {
        printf("Matrices Incompatible\n");
        LogString(90,"Matrices Incompatible");
        return 18;
    }
    m=malloc(ds.nob*dim);
    ERData(e,ds.nob*dim,m);
    ERClose(e);
    hdr[0]=3;
    hdr[1]=1;
    hdr[2]=vecs;
    hdr[3]=vecs;
    e=EWHdr(argv[3],hdr);
    v=malloc(ds.nob);
    for(k=0;k<vecs;k++)
    {
        memset(v,0,ds.nob);   // make new vector v
        vp1=vl+k*ds.nob;
        for(j=0;j<dim;j++)   
        {
            vp2=DPAdv(&ds,j,m);
            fel=DUnpak(&ds,j,vp1);
            DSMad(&ds,fel,1,vp2,v);
        }
        
        for(j=0;j<vecs;j++)
            if(!memcmp(v,vl+j*ds.nob,ds.nob)) break;
        if(j==vecs)
        {
            printf("List not closed - abandoning\n");
            exit(1);
        }
        EWData(e,8,(uint8_t *)&j);
    }
    EWClose(e);

    free(f);
    free(m);
    free(v);
    return 0;
}

/*  end of zmp.c    */
