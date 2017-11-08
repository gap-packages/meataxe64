/*
      zec1.c     meataxe-64 Echelize progam
                 New specification rewrite.
      ======     R. A. Parker 9.4.16
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
    EFIL *e1,*e2,*e3,*e4,*e5,*e6;    // e1 input matrix  
                                     // e2 row-select   e3 col select
                                     // e4 mult e5 cleaner   e6 remnant
    uint64 fdef,nor,noc;
    uint64 hdr[5];
    FIELD * f;
    int * piv;
    DSPACE dsm,dsq,ds4,ds5,ds6;
    Dfmt *m, *q, *vo, *junk;
    Dfmt *vm,*vq;
    size_t sbsr,sbsc;
    uint64 *bsr,*bsc;
    uint64 rank,i,j,col,fel;
    char st[200];

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
    piv=malloc(nor*sizeof(int));    // pivot array uses up to nor
    DSSet(f,noc,&dsm);              // SPACE with noc cols
    m=malloc(nor*dsm.nob);          // input (working) matrix
    vo=malloc(dsm.nob);             // vo has noc columns
    junk=malloc(dsm.nob);           // so does junk
    DSSet(f,nor,&dsq);              // keeptrack matrix q has nor cols
    q=malloc(nor*dsq.nob);          //    and nor rows also
    memset(q,0,nor*dsq.nob);        //    starts as zero

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
        vq=q+i*dsq.nob;             // vq = pointer to row in q
        col=DNzl(&dsm,vm);          // col = pivot
        if(col==ZEROROW) continue;  // nothing to do if row is zero
        piv[i]=col;                 // remember where pivot is
        fel=DUnpak(&dsm,col,vm);    // get the pivotal value
        fel=FieldInv(f,fel);        // invert it
        fel=FieldNeg(f,fel);        // negate it
        DSMul(&dsm,fel,1,vm);       // scalar multiply to make it -1
        DPak(&dsq,rank,vq,1);       // set entry = 1 in new column
        DSMul(&dsq,fel,1,vq);       // multiply that one also
        BSBitSet(bsr,i);            // this row is significant
        BSBitSet(bsc,col);          // as is the pivotal column
        rank++;                     // rank is now one more
        for(j=0;j<nor;j++)          // clear this col in all other rows
        {
            if(j==i) continue;      // but not this row itself!
            fel=DUnpak(&dsm,col,m+j*dsm.nob);   //get the entry
            DSMad(&dsm,fel,1,vm,m+j*dsm.nob);   // clean that entry
            DSMad(&dsq,fel,1,vq,q+j*dsq.nob);   // same op in q
        }
    }

//  output the answers

    hdr[0]=2;                      // row select bit string
    hdr[1]=1;
    hdr[2]=nor;
    hdr[3]=rank;
    hdr[4]=0;
    e2=EWHdr(argv[2],hdr);
    bsr[1]=rank;
    EWData(e2,sbsr,(uint8 *)bsr);
    EWClose(e2);

    hdr[2]=noc;                    // col select bit string
                                   // rest of header the same as rows
    e3=EWHdr(argv[3],hdr);
    bsc[1]=rank;
    EWData(e3,sbsc,(uint8 *)bsc);
    EWClose(e3);

//  need to permute the rows of the multiplier and remnant

    hdr[0]=1;
    hdr[1]=fdef;
    hdr[2]=rank;
    hdr[3]=rank;
    hdr[4]=0;
    DSSet(f,rank,&ds4);
    e4=EWHdr(argv[4],hdr);     // multiplier noc = rank
    hdr[3]=noc-rank;
    e6=EWHdr(argv[6],hdr);     // remnant    noc = noc-rank
    DSSet(f,noc-rank,&ds6);
    for(i=0;i<noc;i++)
    {
        if(BSBitRead(bsc,i)==0) continue;
        for(j=0;j<nor;j++)
        {
            if(BSBitRead(bsr,j)==0) continue;
            if(piv[j]!=i) continue;
            DCut(&dsq,1,0,q+j*dsq.nob,&ds4,vo);
            EWData(e4,ds4.nob,vo);
            BSColSelect(f,bsc,1,m+j*dsm.nob,junk,vo);
            EWData(e6,ds6.nob,vo);
        }
    }
    EWClose(e4);
    EWClose(e6);
    hdr[2]=nor-rank;
    hdr[3]=rank;
    DSSet(f,rank,&ds5);
    e5=EWHdr(argv[5],hdr);
    for(i=0;i<nor;i++)
    {
        if(BSBitRead(bsr,i)==1) continue;
        DCut(&dsq,1,0,q+i*dsq.nob,&ds5,vo);
        EWData(e5,ds5.nob,vo);
    }
    EWClose(e5);
    sprintf(st,"Rank of %s is %d",argv[1],(int)rank);
    printf("%s\n",st);
    LogString(20,st);
    free(f);
    free(piv);
    free(m);
    free(vo);
    free(junk);
    free(q);
    free(bsr);
    free(bsc);

    return 0;
}

/*  end of zec1.c    */
