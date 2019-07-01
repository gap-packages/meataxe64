/*
      zzs.c     meataxe-64 convert to sparsified format
      =====     R. A. Parker    16.2.2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"

int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    uint64_t j,piv,piv1;
    int sparse,lastsparse;
    FELT fel;
    FIELD * f;
    DSPACE ds;
    Dfmt *v,*m;
    long curde,cursp,i;
    uint64_t denvec;
    uint64_t *spsc, *splen, *spdest;
    uint64_t *desc, *delen;
    char * hadit;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 3)
    {
        LogString(80,"usage zzs <input> <output>");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);
    if (hdr[0] != 1)
    {
        LogString(80,"zzs reads a flat matrix");
        exit(22);
    }
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    v=malloc(ds.nob);
    m=malloc(ds.nob*nor);   // matrix may be entirely dense
// set up structure
    spsc=malloc(8*nor);
    splen=malloc(8*nor);
    spdest=malloc(8*nor);
    desc=malloc(8*nor);    // don't think these can be this big
    delen=malloc(8*nor);
    hadit=malloc(noc);
    memset(hadit,0,noc);
    lastsparse=2;    // neither sparse nor dense!
    cursp=-1;
    curde=-1;
    denvec=0;
// read through matrix
    for(i=0;i<nor;i++)
    {
	ERData(e1,ds.nob,v);
// I am not proud of the following code.  There probably is some
// way of seeing it as structured, but I have failed to spot it.
// It seems to naturally be a FSA, readily implemented using goto.
// goto and label are entirely limited to the following lump of code.
// ========================================================== //
  // is the row sparse or dense ?
        sparse=2;             // zero so far
        piv=DNzl(&ds,v);
        if(piv==ZEROROW) goto zerorow;
        fel=DUnpak(&ds,piv,v);
        if(fel!=1) goto dense;
        DPak(&ds,piv,v,0);
        piv1=DNzl(&ds,v);
        DPak(&ds,piv,v,1);   // put it back!
        if(piv1!=ZEROROW) goto dense;
        if(hadit[piv]==1) goto dense;
        sparse=1;
        hadit[piv]=1;
  // is sparse row a continuation of current sparse block ?     
        if(lastsparse!=1) goto newsparseblock;
        if( (spdest[cursp]+splen[cursp]) != piv) goto newsparseblock;
        splen[cursp]++;
        lastsparse=sparse;
        continue;
newsparseblock:
        cursp++;
        spsc[cursp]=i;
        splen[cursp]=1;
        spdest[cursp]=piv;
        lastsparse=sparse;
        continue;
dense:
        sparse=0;
        memcpy(m+denvec*ds.nob,v,ds.nob);
        if(sparse!=lastsparse)
        {
            curde++;
            desc[curde]=i;
            delen[curde]=1;
        }
        else
        {
            delen[curde]++;
        }
        denvec++;
zerorow:
        lastsparse=sparse;
        continue;

// ================================================== //
    }
// write out answer
    hdr[0]=5;
    e2 = EWHdr(argv[2],hdr);
    j=(uint64_t) cursp+1;
    EWData(e2,8,(uint8_t *)&j);
    j=(uint64_t) curde+1;
    EWData(e2,8,(uint8_t *)&j);
    for(i=0;i<=cursp;i++)
    {
        EWData(e2,8,(uint8_t *)(spsc+i));
        EWData(e2,8,(uint8_t *)(splen+i));
        EWData(e2,8,(uint8_t *)(spdest+i));
    }
    for(i=0;i<=curde;i++)
    {
        EWData(e2,8,(uint8_t *)(desc+i));
        EWData(e2,8,(uint8_t *)(delen+i));
    }
    EWData(e2,ds.nob*denvec,m);
    ERClose(e1);
    EWClose(e2);
    free(v);
    free(m);
    free(f);
    free(desc);
    free(spsc);
    free(delen);
    free(splen);
    free(spdest);
    return 0;
}

/******  end of zzs.c    ******/
