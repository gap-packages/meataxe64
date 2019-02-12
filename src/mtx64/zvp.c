/*
      zvp.c     meataxe-64 simple vector permute
      =====     R. A. Parker 22.08.18
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
    uint64_t fdef,dim,maxvecs,j;
    uint64_t vecs,done;
    FIELD * f;
    DSPACE ds;
    Dfmt * vl;    // vector list
    Dfmt * v;
    Dfmt *vp1,*vp2;
    int ngens,pfxlen,dig;
    Dfmt ** m;
    char fn[200],st[200];
    long g,k;
    FELT fel;
//------------------------------------
    LogCmd(argc,argv);
    if (argc != 5) 
    {
        LogString(80,"usage zvp <vec> <genpfx> #gens <output>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);    // vector
    fdef=hdr[1];
    dim=hdr[3];
    if (hdr[2]!=1) 
    {
        LogString(80,"Only one input vector allowed in zvp");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    v=malloc(ds.nob);
    maxvecs=10000;     // need to improve this later
    vl=malloc(maxvecs*ds.nob);
    ERData(e,ds.nob,vl);          // put first vector in
    ERClose(e);
    vecs=1;
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
    done=0;
    while(done<vecs)
    {
        for(g=0;g<ngens;g++)
        {
            memset(v,0,ds.nob);   // make new vector v
            vp1=vl+done*ds.nob;
            for(j=0;j<dim;j++)   
            {
                vp2=DPAdv(&ds,j,m[g]);
                fel=DUnpak(&ds,j,vp1);
                DSMad(&ds,fel,1,vp2,v);
            }
            for(j=0;j<vecs;j++)
                if(!memcmp(v,vl+j*ds.nob,ds.nob)) break;
            if(j==vecs)
            {
                memcpy(vl+vecs*ds.nob,v,ds.nob);
                vecs++;
                if(vecs==maxvecs)
                {
                    printf("Maximum %lu vectors reached - abandoning\n",vecs);
                    exit(1);
                }
            }
        }
        done++;
    }
    sprintf(st,"Orbit of %lu vectors found",vecs);
    printf("%s\n",st);
    LogString(20,st);
    hdr[2]=vecs;
    e = EWHdr(argv[4],hdr);    // output the resulting standard base
    EWData(e,ds.nob*vecs,vl);
    EWClose(e);
    free(f);
    for(g=0;g<ngens;g++) free(m[g]);
    free(m);
    free(v);
    return 0;
}

/*  end of zvp.c    */
