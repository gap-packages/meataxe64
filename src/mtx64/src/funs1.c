// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version funs1.c Simple Gaussian-related functions

// Contents
// fColumnExtract
// fRowRiffle
// fPivotCombine
// fColumnRiffleId

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tuning.h"
#include "field.h"
#include "pcrit.h"
#include "funs.h"
#include "io.h"
#include "bitstring.h"

// fColumnExtract
 
void fColumnExtract(const char *bs, int sbs, const char *in, int sin, 
          const char *sel, int ssel, const char * nsel, int snsel) 
{
    EFIL *e1,*e2,*e3,*e4;
    uint64_t hdr1[5],hdr2[5];
    uint64_t fdef,nor,noc,noc1,noc2;
    uint64_t bssiz;
    DSPACE ds,dssel,dsnon;
    FIELD * f;
    uint64_t RowsThatFit, RowsAtOnce,r,r1;
    Dfmt *d,*d1,*d2;
    uint64_t * bsdata;

    e1=ERHdr(bs,hdr1);   // bit string
    e2=ERHdr(in,hdr2);   // matrix
    fdef=hdr2[1];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    nor=hdr2[2];
    noc=hdr2[3];
    noc1=hdr1[3];
    noc2=hdr1[2]-hdr1[3];
    DSSet(f,noc,&ds);
    DSSet(f,noc1,&dssel);
    DSSet(f,noc2,&dsnon);
    if(noc==0) RowsThatFit=10000000;
       else    RowsThatFit=(1000000ul*MEGABYTES)/ds.nob;
    RowsAtOnce=RowsThatFit/5;
    if(RowsAtOnce==0) RowsAtOnce=1;
    if(RowsThatFit>100) RowsAtOnce=RowsThatFit/10;
    if(RowsThatFit>1000) RowsAtOnce=RowsThatFit/20;
    if(RowsThatFit>10000) RowsAtOnce=RowsThatFit/50;
    if(RowsThatFit>100000) RowsAtOnce=RowsThatFit/100;
    if(RowsThatFit>1000000) RowsAtOnce=10000;
    d=malloc(RowsAtOnce*ds.nob);
    d1=malloc(RowsAtOnce*dssel.nob);
    d2=malloc(RowsAtOnce*dsnon.nob);
// read in bitstring
    bssiz = 8*(2+(noc+63)/64);
    bsdata=malloc(bssiz);
    ERData(e1,bssiz,(uint8_t *)bsdata);
    ERClose1(e1,sbs);
// start writing output files
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor;
    hdr1[3]=noc1;
    hdr1[4]=0;
    e3 = EWHdr(sel,hdr1);
    hdr1[3]=noc2;
    e4 = EWHdr(nsel,hdr1);
    r=0;
    while(r!=nor)
    {
        r1=RowsAtOnce;
        if(r1>(nor-r)) r1=nor-r;
        ERData(e2,r1*ds.nob,d);
        BSColSelect(f,bsdata,r1,d,d1,d2);
        EWData(e3,r1*dssel.nob,d1);
        EWData(e4,r1*dsnon.nob,d2);
        r+=r1;
    }
    ERClose1(e2,sin);
    EWClose1(e3,ssel);
    EWClose1(e4,snsel);
    free(f);
    free(d);
    free(d1);
    free(d2);
    free(bsdata);
    return;
}

void fRowRiffle(const char *bs, int sbs, const char * ins, int sins,
                 const char * inn, int sinn, const char * out, int sout)
{
    EFIL *e1,*e2,*e3,*e4;
    uint64_t hdr1[5],hdr2[5],hdr3[5];
    uint64_t fdef,nor,noc,nor1,nor0;
    FIELD * f;
    DSPACE ds;
    Dfmt *v;
    uint64_t * bsdata;
    uint64_t i,siz;

    e1=ERHdr(bs,hdr1);   // bs
    e2=ERHdr(ins,hdr2);   // m1
    e3=ERHdr(inn,hdr3);   // m0
    fdef=hdr2[1];
    nor1=hdr2[2];
    noc=hdr2[3];
    nor0=hdr3[2];
    nor=nor0+nor1;
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    hdr1[0]=1;
    hdr1[1]=fdef;
    hdr1[2]=nor;
    hdr1[3]=noc;
    hdr1[4]=0;
    e4 = EWHdr(out,hdr1);
    v=malloc(ds.nob);
    siz = 8*(2+(nor+63)/64);
    bsdata=malloc(siz);
    ERData(e1,siz,(uint8_t *)bsdata);
    for(i=0;i<nor;i++)
    {
	if(BSBitRead(bsdata,i)==1) ERData(e2,ds.nob,v);
              else                   ERData(e3,ds.nob,v);
        EWData(e4,ds.nob,v);
    }
    ERClose1(e1,sbs);
    ERClose1(e2,sins);
    ERClose1(e3,sinn);
    EWClose1(e4,sout);
    free(f);
    free(v);
    free(bsdata);
}

void fPivotCombine(const char *b1, int sb1, const char *b2, int sb2,
                   const char *bc, int sbc, const char *br, int sbr)
{
    EFIL *e1,*e2,*e3,*e4;
    uint64_t hdr1[5],hdr2[5];
    uint64_t nor,nor2,noc1, noc2, noc;
    size_t siz,siz4;
    uint64_t *bs1,*bs2,*bs3,*bs4;

    e1=ERHdr(b1,hdr1);   // b1 old pivots
    e2=ERHdr(b2,hdr2);   // b2 new pivots
    nor=hdr1[2];         // total bits
    noc1=hdr1[3];        // old set bits
    nor2=hdr2[2];        // new total bits
    noc2=hdr2[3];        // additional new bits
    noc=noc1+noc2;       // resulting set bits
    hdr1[3]=noc;         // . . . in output 'bc'
    e3=EWHdr(bc,hdr1);   // otherwise like b1
    hdr1[2]=noc;         // riffle bits = set-bits
    hdr1[3]=noc1;        // riffle set bits = old
    e4=EWHdr(br,hdr1);   // write header for riffle

    siz = 8*(2+(nor+63)/64);  // read b1 into bs1
    bs1=malloc(siz);
    ERData(e1,siz,(uint8_t *)bs1);
    ERClose1(e1,sb1);

    siz = 8*(2+(nor2+63)/64); // read b2 into bs2
    bs2=malloc(siz);
    ERData(e2,siz,(uint8_t *)bs2);
    ERClose1(e2,sb2);

    siz = 8*(2+(nor+63)/64);  // set bs3 and bs4 to zero
    bs3=malloc(siz);
    memset(bs3,0,siz);
    siz4 = 8*(2+(noc+63)/64);
    bs4=malloc(siz4);
    memset(bs4,0,siz4);

    BSCombine(bs1,bs2,bs3,bs4);
    EWData(e3,siz ,(uint8_t *)bs3);
    EWData(e4,siz4,(uint8_t *)bs4);

    EWClose1(e3,sbc);
    EWClose1(e4,sbr);
    free(bs1);
    free(bs2);
    free(bs3);
    free(bs4);
}

uint64_t fColumnRiffleIdentity(const char *bs, int sbs, 
             const char *rm, int srm, const char *out, int sout)
{
    EFIL *ebs,*ei,*eo;   // bitstring, input, output
    uint64_t hdrbs[5],hdrio[5];
    uint64_t nor,noci,noco,fdef,siz;
    FIELD * f;
    DSPACE dsi,dso;    // input output
    Dfmt *mo,*mi;      // output input
    uint64_t * bst;

    ebs=ERHdr(bs,hdrbs);   // bitstring = 2 1 bits setb 0
    ei=ERHdr(rm,hdrio);    // remnant   = 1 fdef nor noc 0
    nor =hdrio[2];         // rows in
    noco=hdrbs[2];         // bits = output cols
    noci=hdrio[3];         // input cols
    if( (noci!=hdrbs[3]) || (nor!=(noco-noci)) )
    {
        LogString(80,"Inputs incompatible");
        exit(22);
    }
    fdef=hdrio[1];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    hdrio[3]=noco;
    eo=EWHdr(out,hdrio);

    DSSet(f,noci,&dsi);   // input space
    DSSet(f,noco,&dso);   // output space

    mi=malloc(dsi.nob*nor);   // input
    mo=malloc(dso.nob*nor);   // output

// read the bit string
    siz = 8*(2+(noco+63)/64);
    bst=malloc(siz);
    ERData(ebs,siz,(uint8_t *)bst);
    ERData(ei,dsi.nob*nor,mi);
    BSColRifZ(f,bst,nor,mi,mo);
    BSColPutS(f,bst,nor,1,mo);
    EWData(eo,dso.nob*nor,mo);
    
    free(mi);
    free(mo);
    free(f);
    free(bst);
    ERClose1(ebs,sbs);
    ERClose1(ei,srm);
    EWClose1(eo,sout);
    return nor;
}


/* end of funs1.c  */
