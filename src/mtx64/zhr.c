/*
      zhr.c     meataxe-64 Holt/Rees root-finding program
      =====     R. A. Parker 15.1.19
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "io.h"

FELT lindiv(FIELD * f, FELT r, Dfmt * p, DSPACE * ds, Dfmt * q)
{
    FELT val,c;
    long i;

    memset(q,0,ds->nob);
    val=0;
    for(i=ds->noc-1;i>=0;i--)
    {
        DPak(ds,i,q,val);
        val=FieldMul(f,val,r);
        c=DUnpak(ds,i,p);
        val=FieldAdd(f,val,c);
    }
    return val;
}

int main(int argc,  char **argv)
{
    EFIL *e;
    uint64_t hdr[5];
    uint64_t fdef,deg,nfact;
    FIELD * f;
    uint64_t maxroots,rootssofar;
    FELT * root;
    FELT r,rem;
    uint64_t * multiplicity;
    uint64_t factdeg,i,j;
    DSPACE ds;
    Dfmt * pol;
    Dfmt *v1,*v2;
    Dfmt *vp;

    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage zhr <poly>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);    // Input polynomial
    fdef=hdr[1];
    deg=hdr[2];
    nfact=hdr[3];
    if (hdr[0]!=4) 
    {
        LogString(80,"Input to zhr must be a polynomial");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    maxroots=fdef;
    if(maxroots>deg) maxroots=deg;
    root=malloc(maxroots*sizeof(FELT));
    multiplicity=malloc(maxroots*sizeof(uint64_t));
    DSSet(f,deg+1,&ds);
    pol=malloc(ds.nob);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    rootssofar=0;
    for(i=0;i<nfact;i++)
    {
        ERData(e,8,(uint8_t *)&factdeg);
        DSSet(f,factdeg+1,&ds);
        ERData(e,ds.nob,pol);
        for(r=0;r<fdef;r++)
        {
            DCpy(&ds,pol,1,v1);
            while(1)
            {
                rem=lindiv(f,r,v1,&ds,v2);   // maybe reduce degree?
                if(rem!=0) break;
                for(j=0;j<rootssofar;j++)
                {
                    if(root[j]==r)
                    {
                        multiplicity[j]++;
                        break;
                    }
                }
                if(j==rootssofar)
                {
                    root[j]=r;
                    multiplicity[j]=1;
                    rootssofar++;
                }
                vp=v1;
                v1=v2;
                v2=vp;
            }
        }
    }

    EWClose(e);
    if(rootssofar==0)
        printf("No roots in field\n");
    else
    {
        for(i=0;i<rootssofar;i++)
            printf(" root %ld, multiplicity %ld\n",root[i],multiplicity[i]);
    }

    free(root);
    free(f);
    free(multiplicity);
    free(v1);
    free(v2);
}

/*  end of zhr.c    */
