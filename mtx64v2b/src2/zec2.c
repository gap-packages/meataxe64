/*
      zec2.c     meataxe-64 Echelize progam
                 Using Gauss module
      ======     R. A. Parker 11.5.19
*/

#define REPEATS 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
#include "gauss.h"

int main(int argc,  char **argv)
{
    GAUSS * gs;
    Dfmt *a;
    Dfmt *m,*c,*r;
    uint64_t *rs,*cs;
    FELT det;
    uint64_t nor,noc,fdef,rank,maxrank;
    uint64_t lenrs,lencs;
    EFIL *e1,*e2,*e3,*e4,*e5,*e6;
    uint64_t hdr[5];
    FIELD * f;
    DSPACE dsa,dsmax,dsm,dsrem;
    char st[200];
    long i;

    LogCmd(argc,argv);
    if (argc != 7) 
    {
        LogString(80,"usage zec m rs cs mul cln rem");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);           // create FIELD
    FieldASet(fdef,f);
    gs=GaussCreate(f);

    DSSet(f,noc,&dsa);              // SPACE with noc cols
    a=malloc(nor*dsa.nob);          // input (working) matrix
    ERData(e1,nor*dsa.nob,a);       // read in entire matrix
    ERClose(e1);                    // might as well close it now
    maxrank=nor;
    if(maxrank>noc) maxrank=noc;
    DSSet(f,maxrank,&dsmax);              // SPACE with maxrank cols
    m=malloc(maxrank*dsmax.nob);          // multiplier max size
    c=malloc(nor*dsa.nob);           // cleaner max size (too big!)
    r=malloc(nor*dsa.nob);           // remnant max size

    lenrs=8*(2+(nor+63)/64);         // allocate string for row select
    rs=malloc(lenrs);
    memset(rs,0,lenrs);
    rs[0]=nor;
    lencs=8*(2+(noc+63)/64);         // allocate string for row select
    cs=malloc(lencs);
    memset(rs,0,lencs);
    cs[0]=noc;
    for(i=0;i<REPEATS;i++)
        rank=BCEch(gs,&dsa,a,rs,cs,&det,m,c,r,nor,0);

//  output the answers

    hdr[0]=2;                      // row select bit string
    hdr[1]=1;
    hdr[2]=nor;
    hdr[3]=rank;
    hdr[4]=0;
    e2=EWHdr(argv[2],hdr);
    rs[1]=rank;
    EWData(e2,lenrs,(uint8_t *)rs);
    EWClose(e2);

    hdr[2]=noc;                    // col select bit string
    e3=EWHdr(argv[3],hdr);
    cs[1]=rank;
    EWData(e3,lencs,(uint8_t *)cs);
    EWClose(e3);

    hdr[0]=1;                 // multiplier
    hdr[1]=fdef;
    hdr[2]=rank;
    hdr[3]=rank;
    hdr[4]=0;
    DSSet(f,rank,&dsm);
    e4=EWHdr(argv[4],hdr);
    EWData(e4,rank*dsm.nob,m);
    EWClose(e4);

    hdr[2]=nor-rank;           // cleaner
    hdr[3]=rank;
    e5=EWHdr(argv[5],hdr);
    EWData(e5,(nor-rank)*dsm.nob,c);
    EWClose(e5);

    hdr[2]=rank;                // remnant
    hdr[3]=noc-rank;
    e6=EWHdr(argv[6],hdr);
    DSSet(f,noc-rank,&dsrem);
    EWData(e6,rank*dsrem.nob,r);
    EWClose(e6);

    sprintf(st,"Rank of %s is %d",argv[1],(int)rank);
    printf("%s\n",st);
    LogString(20,st);

    free(a);
    free(m);
    free(c);
    free(r);
    free(cs);
    free(rs);
    free(f);
    return 0;
}

/*  end of zec2.c    */
