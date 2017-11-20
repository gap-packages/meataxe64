/*
      zor1.c     meataxe-64 simple matrix order program
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
    long order,orgord,k,rank;
    char st[200];
    uint64_t hdr[5];
    EFIL *e;
    uint64_t fdef,dim,col;
    uint64_t i,j;
    FIELD * f;
    DSPACE ds;
    int * piv;
    int lindep;
    Dfmt * m1;
    Dfmt * m2;
    Dfmt * v1;
    Dfmt * v2;
    Dfmt * v3;
    Dfmt * vp1;
    FELT fel;

    LogCmd(argc,argv);
    if (argc != 2) 
    {
        LogString(80,"usage zor <m1>");
        exit(14);
    }
    e = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    dim=hdr[2];
    if (hdr[2]!=hdr[3]) 
    {
        LogString(80,"Matrix not square in zor");
        exit(15);
    }
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,dim,&ds);
    piv=malloc(dim*sizeof(int));
    m1=malloc(ds.nob*dim);        // input matrix
    ERData(e,ds.nob*dim,m1);
    ERClose(e);
    m2=malloc(ds.nob*dim);        // space spanned so far
    v1=malloc(ds.nob);            // first work vector
    v2=malloc(ds.nob);            // second work vector
    v3=malloc(ds.nob);            // third one
    order=1;
    rank=0;
    for(i=0;i<dim;i++)
    {
        for(j=0;j<rank;j++)       // first check i is no pivot
           if(i==piv[j]) break;
        if(j!=rank) continue;     // don't spin it if it is
        memset(v1,0,ds.nob);
        DPak(&ds,i,v1,1);         // set v1 as initial vector
        k=0;
        lindep=0;
        memcpy(v2,v1,ds.nob);
        while(1)
        {
            k++;
            if(lindep==0)
            {
                memcpy(v3,v2,ds.nob);
                for(j=0;j<rank;j++)
                {
                    fel=DUnpak(&ds,piv[j],v3);
                    DSMad(&ds,fel,1,m2+j*ds.nob,v3);
                }
                col=DNzl(&ds,v3);
                if(col!=ZEROROW)
                {
                    piv[rank]=col;
                    fel=DUnpak(&ds,col,v3);
                    fel=FieldInv(f,fel);
                    fel=FieldNeg(f,fel);
                    DSMul(&ds,fel,1,v3);
                    DCpy(&ds,v3,1,m2+rank*ds.nob);
                    rank++;
                }
                else
                    lindep=1;
            }
            memset(v3,0,ds.nob);
            for(j=0;j<dim;j++)   
            {
                vp1=DPAdv(&ds,j,m1);
                fel=DUnpak(&ds,j,v2);
                DSMad(&ds,fel,1,vp1,v3);
            }
            if(memcmp(v3,v1,ds.nob)==0) break;
            vp1=v2;
            v2=v3;
            v3=vp1;
        }
        orgord=order;
        while(order%k!=0)
        {
            if(order>1000000000000)
            {
                sprintf(st,"Order of %s is > 1000000000000",argv[1]);
                printf("%s\n",st);
                LogString(20,st);
                return 0;
            }
            order+=orgord;
        }
    }
    sprintf(st,"Order of %s is %ld",argv[1],order);
    printf("%s\n",st);
    LogString(20,st);
    free(f);
    free(piv);
    free(m1);
    free(m2);
    free(v1);
    free(v2);
    free(v3);
    return 0;
}

/*  end of zor1.c    */
