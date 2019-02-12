/*
      zem.c     meataxe-64 Elementary Matrix factorization
      =====     R. A. Parker    10.4.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"

#define BEAM 5

uint64_t bhash(Dfmt * d, uint64_t bytes, uint64_t arraysize)
{
    uint64_t i,hash;
    hash=71;
    for(i=0;i<bytes;i++) hash=hash*65537+d[i]*17;
    return hash%arraysize;
}

uint64_t val(DSPACE * ds, Dfmt * m, uint64_t nor)
{
    uint64_t corr,vz;
    int i,j;
    FELT fel;

    corr=1;
    for(i=0;i<nor;i++)
    {
        vz=4;
        for(j=0;j<ds->noc;j++)
        {
            fel=DUnpak(ds,j,m+i*ds->nob);
            if(j<i)
            {
                if(fel!=0) vz=2;
                if(fel==0) corr+=vz;
            }
            if(j==i)
            {
                if(fel==0) break;
                if(fel==1) corr+=3;
                   else    corr+=2;
            }
            if(j>i)
            {
                if(fel==0) corr+=vz/2;
            }
        }
    }
    return corr;
}
 
int main(int argc,  char **argv)
{
    EFIL *e;
    FIELD * f;
    uint64_t hdr[5];
// j is a hash
    uint64_t j,fdef,nor,noc,siz,hash,work;
    uint64_t targetval;
    uint64_t r1,r2,z;
    int history,ftime,moves,done;     // ten types of move - surely enough.
    DSPACE ds;
    Dfmt *X,*Y;
    FILE *fo;
    uint64_t tarcost,tarhash;
// costs
    int maxcost,ncost,i,cost[10],curcost;
// values
    int *bval[20],*bvsav,value;
// history
    uint64_t *bhist[20],*bhsav;
// Matrices
    Dfmt * bmatrix[20],*bmsav;

// set up the costs of moves to be minimized.  
    moves=3;                       // just junk at the moment
// these must all be at least 1 and at most 20
    cost[0]=6;     // multiply-and-add
    cost[1]=4;     // add
    cost[2]=5;     // subtract
    cost[3]=1;     // swap
    history=0;
    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if ((argc != 3)&&(argc != 2))
    {
        LogString(80,"usage zem mtx or zem mtx fac");
        exit(21);
    }
    if(argc==3)
    {
        history=1;
        fo=fopen(argv[2],"wb");
    }
    e=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    targetval=1+4*nor*noc-noc*noc;

    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    siz=ds.nob*nor;
    X=malloc(siz);
    Y=malloc(siz);
    ERData(e,siz,X);
    ERClose(e);

// compute maxcost
    maxcost=0;
    for(i=0;i<moves;i++)
        if(maxcost<cost[i]) maxcost=cost[i];
// allocate beam structures
    for(i=0;i<=maxcost;i++)
    {
        bmatrix[i]=malloc(BEAM*siz);
        bval[i]  =malloc(BEAM*sizeof(int));
        bhist[i]=malloc(BEAM*6*sizeof(uint64_t));
    }

// main loop

    tarcost=0;  tarhash=0;  // compiler warnings
    ftime=1;
    while(1)
    {
        work=0;
        done=0;
// Start with input matrix in beam
        for(i=0;i<=maxcost;i++)
            for(j=0;j<BEAM;j++) bval[i][j]=0;
        hash=bhash(X,siz,BEAM);
        value=val(&ds,X,nor);
        memcpy(bmatrix[0]+hash*siz,X,siz);
        bval[0][hash]=value;
        bhist[0][6*hash]=0;    // no history
        curcost=0;
        while(1)
        {
#ifdef DEBUG
            for(j=0;j<BEAM;j++) printf("%5d",bval[0][j]);
            printf("\n");
            memcpy(Y,bmatrix[0],siz);
            for(i=0;i<nor;i++)
            {
              for(j=0;j<noc;j++)
                printf("%ld",DUnpak(&ds,j,Y+i*ds.nob));
              printf("\n");
            }
#endif
            for(j=0;j<BEAM;j++)
            {
                if(bval[0][j]==0) continue;   // empty slot
                if((bval[0][j]==targetval)&&(ftime==1)) done=1;
                if((ftime==0)&&(curcost==tarcost)
                       &&(j=tarhash)) done=1;
                if(done==1) break;
                work++;
// generate all matrices one step on
                for(r1=0;r1<nor;r1++)
                {
                  for(r2=0;r2<nor;r2++)
                  {
                    for(z=1;z<fdef;z++)
                    {
                      if((r1==r2)&&((z+1)==fdef)) continue;  // multiply by 0
                      ncost=cost[0];
                      if((z+1)==fdef) ncost=cost[2];
                      if(z==1) ncost=cost[1];
                      memcpy(Y,bmatrix[0]+j*siz,siz);
                      DSMad(&ds,z,1,Y+r1*ds.nob,Y+r2*ds.nob);
// Check if matrix is good.
                      hash=bhash(Y,siz,BEAM);
                      value=val(&ds,Y,nor);
                      if(value <= bval[ncost][hash]) continue;
// Put matrix in beam[ncost][hash]
                      memcpy(bmatrix[ncost]+hash*siz,Y,siz);
                      bval[ncost][hash]=value;
                      bhist[ncost][6*hash]=1;
                      bhist[ncost][6*hash+1]=r1;
                      bhist[ncost][6*hash+2]=r2;
                      bhist[ncost][6*hash+3]=z;
                      bhist[ncost][6*hash+4]=j;
                      bhist[ncost][6*hash+5]=curcost;
                    }
                  }
                }
// try all the swaps
                for(r1=0;r1<nor;r1++)
                {
                  for(r2=0;r2<nor;r2++)
                  {
                      if(r1==r2) continue;
                      ncost=cost[3];
                      memcpy(Y,bmatrix[0]+j*siz,siz);
                      memcpy(Y+r1*ds.nob,bmatrix[0]+j*siz+r2*ds.nob,ds.nob);
                      memcpy(Y+r2*ds.nob,bmatrix[0]+j*siz+r1*ds.nob,ds.nob);
// Check if matrix is good.
                      hash=bhash(Y,siz,BEAM);
                      value=val(&ds,Y,nor);
                      if(value <= bval[ncost][hash]) continue;
// Put matrix in beam[ncost][hash]
                      memcpy(bmatrix[ncost]+hash*siz,Y,siz);
                      bval[ncost][hash]=value;
                      bhist[ncost][6*hash]=2;
                      bhist[ncost][6*hash+1]=r1;
                      bhist[ncost][6*hash+2]=r2;
                      bhist[ncost][6*hash+3]=0;
                      bhist[ncost][6*hash+4]=j;
                      bhist[ncost][6*hash+5]=curcost;
                  }
                }
            }
            if(done==1) break;
            curcost++;
            if((curcost%20)==0)
                printf("Current cost is %d, work %ld\n",curcost,work);
            bmsav=bmatrix[0];
            bvsav=bval[0];
            bhsav=bhist[0];
            for(i=0;i<maxcost;i++)
            {
                bmatrix[i]=bmatrix[i+1];
                bval[i]=bval[i+1];
                bhist[i]=bhist[i+1];
            }
            bmatrix[maxcost]=bmsav;
            bval[maxcost]=bvsav;
            for(j=0;j<BEAM;j++) bval[maxcost][j]=0;
            bhist[maxcost]=bhsav;
        }
        if(ftime==1) printf("Found one cost %d, work %ld\n",curcost,work);
        if(history==0) break;
        ftime=0;
        if(bhist[0][6*j]==0) break;
        fprintf(fo,"%ld %ld %ld %ld\n",bhist[0][6*j],bhist[0][6*j+1],
                                       bhist[0][6*j+2],bhist[0][6*j+3]);
        tarcost=bhist[0][6*j+5];
        tarhash=bhist[0][6*j+4];
    }
    free(X);
    free(Y);
    free(f);
    for(i=0;i<=maxcost;i++)
    {
        free(bmatrix[i]);
        free(bval[i]);
        free(bhist[i]);
    }
    return 0;
}

/******  end of zem.c    ******/
