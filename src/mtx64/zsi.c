/*
      zsi.c     meataxe-64 Sparsity initiate
      =====     R. A. Parker 17.3.19
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "io.h"
#include "funs.h"

int main(int argc,  char **argv)
{
    uint64_t N1,N2,fdef,noc,nor,i,j;
    uint64_t hdr[5];
    char ft1[]="temp1";
    char ft2[]="temp2";
    char *ptf1,*ptf2;
    EFIL *e1,*e2;;
    FIELD * f;
    DSPACE ds;
    Dfmt *v;

// avoid compiler warnings
    ptf1=NULL;
    ptf2=NULL;

    LogCmd(argc,argv);
    if (argc != 5) 
    {
        LogString(80,"usage zsi m1 m2 N1 N2");
        exit(21);
    }
    N1=strtoul(argv[3],NULL,0);
    N2=strtoul(argv[4],NULL,0);
    nor=N1*N2;
    EPeek(argv[1],hdr);
    fdef=hdr[1];
    noc=hdr[3];
    fRandomMatrix(ft1, 1, fdef, N1, noc);
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,NOMUL);
    DSSet(f,noc,&ds);
    v=malloc(ds.nob);
    hdr[2]=nor;
    hdr[4]=N1;
    e2 = EWHdr(argv[2],hdr);
    for(i=0;i<N2;i++)
    {
        if((i%2)==0)
        {
            ptf1=ft1;
            ptf2=ft2;
        }
        else
        {
            ptf1=ft2;
            ptf2=ft1;
        }

        e1=ERHdr(ptf1,hdr);
        for(j=0;j<N1;j++)
        {
            ERData(e1,ds.nob,v);
            EWData(e2,ds.nob,v);
        }
        ERClose1(e1,1);
        if( (i+1) != N2) fMultiply("tempa",ptf1,1,argv[1],1,ptf2,1);
    }
    EWClose(e2);
    free(v);
    return 0;
}

/*  end of zsi.c    */
