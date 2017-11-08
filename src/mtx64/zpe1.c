/*
      zpe1.c     meataxe-64 Echelize progam
      ======     R. A. Parker 25.4.16
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
#include "pcrit.h"

int main(int argc,  char **argv)
{
    EFIL *e1,*e2,*e3;                // e1 input matrix  
                                     // e2 row-select   e3 remnant
    uint64 fdef,nor,noc;
    uint64 hdr[5];
    FIELD * f;
    int * piv;
    DSPACE dsm,ds3;
    Dfmt *m, *vo, *junk;
    Dfmt *vm;
    size_t sbsr,sbsc;
    uint64 *bsr,*bsc;
    uint64 rank,i,j,col,fel;
    char st[200];

    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zpe m cs rem");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);           // create FIELD
    FieldASet(fdef,f);
    piv=malloc(nor*sizeof(int));    // pivot array uses up to nor
    DSSet(f,noc,&dsm);              // SPACE with noc cols
    m=malloc(nor*dsm.nob);          // input (working) matrix
    vo=malloc(dsm.nob);             // vo has noc columns
    junk=malloc(dsm.nob);           // so does junk

    sbsr=8*(2+(nor+63)/64);         // allocate string for row select
    bsr=malloc(sbsr);
    memset(bsr,0,sbsr);
    bsr[0]=nor;                     // nor bits, set ones undefined
    sbsc=8*(2+(noc+63)/64);         // allocate string for col select
    bsc=malloc(sbsc);
    memset(bsc,0,sbsc);
    bsc[0]=noc;                     // noc bits, set left undefined

    rank = 0;                       // rank is zero so far
    ERData(e1,nor*dsm.nob,m);       // read in entire matrix
    ERClose(e1);                    // might as well close it now

    for(i=0;i<nor;i++)              // process each row in turn
    {
        vm=m+i*dsm.nob;             // vm = pointer to current row in m
        col=DNzl(&dsm,vm);          // col = pivot
        if(col==ZEROROW) continue;  // nothing to do if row is zero
        BSBitSet(bsr,i);            // row i is significant
        piv[i]=col;                 // remember where pivot is
        fel=DUnpak(&dsm,col,vm);    // get the pivotal value
        fel=FieldInv(f,fel);        // invert it
        fel=FieldNeg(f,fel);        // negate it
        DSMul(&dsm,fel,1,vm);       // scalar multiply to make it -1
        BSBitSet(bsc,col);          // as is the pivotal column
        rank++;                     // rank is now one more
        for(j=0;j<nor;j++)          // clear this col in all other rows
        {
            if(j==i) continue;      // but not this row itself!
            fel=DUnpak(&dsm,col,m+j*dsm.nob);   //get the entry
            DSMad(&dsm,fel,1,vm,m+j*dsm.nob);   // clean that entry
        }
    }

//  output the answers

    hdr[0]=2;
    hdr[1]=1;
    hdr[2]=noc;                    // col select bit string
    hdr[3]=rank;
    hdr[4]=0;
    e2=EWHdr(argv[2],hdr);
    bsc[1]=rank;
    EWData(e2,sbsc,(uint8 *)bsc);
    EWClose(e2);

//  need to permute the rows of the remnant

    hdr[0]=1;
    hdr[1]=fdef;
    hdr[2]=rank;
    hdr[3]=noc-rank;
    hdr[4]=0;
    e3=EWHdr(argv[3],hdr);     // remnant    noc = noc-rank
    DSSet(f,noc-rank,&ds3);
    for(i=0;i<noc;i++)
    {
        if(BSBitRead(bsc,i)==0) continue;
        for(j=0;j<nor;j++)
        {
            if(BSBitRead(bsr,j)==0) continue;
            if(piv[j]!=i) continue;
            BSColSelect(f,bsc,1,m+j*dsm.nob,junk,vo);
            EWData(e3,ds3.nob,vo);
        }
    }
    EWClose(e3);
    sprintf(st,"Rank of %s is %d",argv[1],(int)rank);
    printf("%s\n",st);
    LogString(20,st);

    free(f);
    free(piv);
    free(m);
    free(vo);
    free(junk);
    free(bsr);
    free(bsc);

    return 0;
}

/*  end of zpe1.c    */
