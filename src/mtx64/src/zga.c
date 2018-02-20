/*
      zga.c     meataxe-64 make group algebra element program
      =====     R. A. Parker 03.09.15
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "slab.h"

Dfmt ** m;
FIELD * f;
DSPACE ds;

void g1(int a)
{
    Dfmt *ma;
    FELT min1;
    uint64_t i;
    min1=FieldNeg(f,1);
    memset(m[a],0,ds.noc*ds.nob);
    ma=m[a];
    for(i=0;i<ds.noc;i++)
    {
        DPak(&ds,i,ma,min1);
        ma=DPInc(&ds,ma);
    }
}

void mu(int a, int b, int c)
{
    SLMul(f,m[a],m[b],m[c],ds.noc,ds.noc,ds.noc);
}

void ad(int a, int b, int c)
{
    DAdd(&ds,ds.noc,m[a],m[b],m[c]);
}

void sb(int a, int b, int c)
{
    DSub(&ds,ds.noc,m[a],m[b],m[c]);
}

void mkef(int eff)
{
    switch(eff)
    {
// first the ones that take a single multiplication
// I think this is all the really robust ones
      case 1:
        ad(0,1,2);
        mu(2,2,3);
        g1(4);
        ad(3,4,0);
        ad(0,1,2);
        break;
      case 2:
        mu(1,1,2);
        ad(0,2,3);
        ad(1,3,2);
        g1(4);
        ad(4,2,2);
        break;
// Then the quadratic ones.
// Two multiplies but they can be done in parallel
      case 3:
        mu(0,1,2);
        mu(1,0,3);
        ad(2,3,4);
        ad(0,1,3);
        ad(3,4,2);
        g1(4);
        ad(4,2,2);
        break;
// Now those that need two dependent multiplies
      case 4:
        mu(0,1,2);
        mu(2,2,3);
        ad(2,3,4);
        ad(0,4,2);
        g1(4);
        ad(4,2,2);
        break;
      case 5:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(6,4,3);
        sb(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(4,5,2);
        sb(0,2,2);
        break;
      case 6:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(6,4,3);
        ad(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(1,5,2);
        ad(4,2,2);
        break;
      case 7:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(4,2,2);
        ad(1,2,2);
        break;
      case 8:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(1,2,2);
        break;
      case 9:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(4,5,2);
        ad(0,2,2);
        break;
      case 10:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,0,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(1,2,2);
        ad(0,2,2);
        break;
      case 11:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,1,3);
        mu(2,3,5);
        ad(1,5,2);
        ad(4,2,2);
        break;
      case 12:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,1,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(0,2,2);
        break;
      case 13:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,1,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(4,2,2);
        ad(0,2,2);
        break;
      case 14:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,1,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(4,2,2);
        ad(0,2,2);
        ad(1,2,2);
        break;
      case 15:
        ad(0,1,2);
        g1(6);
        ad(2,6,3);
        mu(2,3,4);
        ad(0,4,3);
        ad(1,3,2);
        ad(2,1,3);
        mu(2,3,5);
        ad(6,5,2);
        ad(0,2,2);
        break;
     case 16:
        g1(3);
        ad(3,0,4);
        mu(4,1,5);
        ad(0,1,4);
        mu(5,4,2);
        ad(3,2,2);
        ad(1,2,2);
        break;
     case 17:
        ad(0,1,2);
        mu(2,1,3);
        mu(3,2,4);
        ad(4,0,2);
        break;
      case 18:
        ad(0,1,3);
        mu(0,3,4);
        mu(4,3,2);
        ad(0,2,2);
        ad(4,2,2);
        break;
      case 19:
        ad(0,1,3);
        mu(3,3,4);
        mu(3,4,2);
        ad(0,2,2);
        break;
      case 20:
        ad(0,1,3);
        mu(3,3,4);
        mu(3,4,2);
        ad(1,2,2);
        break;
      case 21:
        ad(0,1,3);
        mu(3,3,4);
        mu(3,4,2);
        ad(0,2,2);
        ad(4,2,2);
        break;
      case 22:
        ad(0,1,3);
        mu(3,3,4);
        mu(3,4,2);
        ad(1,2,2);
        ad(4,2,2);
        break;
      case 23:
        ad(0,1,3);
        mu(3,3,4);
        g1(2);
        ad(2,1,1);
        mu(1,4,5);
        ad(5,2,2);
        ad(3,2,2);
        break;
      case 24:
        ad(0,1,3);
        g1(2);
        ad(1,2,2);
        mu(2,3,4);
        mu(4,3,2);
        ad(1,2,2);
        ad(4,2,2);
        break;
      case 25:
        mu(1,0,4);
        g1(2);
        ad(4,2,2);
        ad(0,1,3);
        mu(2,3,5);
        ad(4,5,2);
        break;
      case 26:
        ad(0,1,3);
        mu(1,3,4);
        g1(2);
        ad(4,2,2);
        mu(2,3,5);
        ad(4,5,2);
        ad(1,2,2);
        break;
      case 27:
        ad(0,1,3);
        mu(0,3,4);
        g1(2);
        ad(4,2,2);
        mu(2,3,5);
        ad(4,5,2);
        ad(0,2,2);
        break;
      case 28:
        mu(0,1,2);
        mu(2,2,3);
        g1(2);
        ad(3,2,2);
        ad(0,2,2);
        break;
      case 29:
        ad(0,1,2);
        mu(2,2,3);
        mu(3,3,2);
        ad(0,2,2);
        break;
      case 30:
        ad(0,1,2);
        mu(2,2,3);
        ad(0,3,3);
        mu(3,2,4);
        ad(4,0,2);
        break;

      default:
        LogString(80,"Invalid group algebra element number");
        exit(22);
    }
}

int main(int argc,  char **argv)
{
    int ngens,g,k,dig,pfxlen,eff;

    char fn[200];
    EFIL *e;
    uint64_t hdr[5];
    uint64_t fdef,dim;

    LogCmd(argc,argv);
    if (argc != 5) 
    {
        LogString(80,"usage zga <genpfx> #gens <elt-number> <output>");
        exit(14);
    }
    ngens=atoi(argv[2]);
    m=malloc((ngens+7)*sizeof(Dfmt *));  // allow 7 work areas
    dim=0;                         // avoid compiler warnings
    strcpy(fn,argv[1]);
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

        e = ERHdr(fn,hdr);    // generator
        if(g==0)
        {
            fdef=hdr[1];
            dim=hdr[3];
            f = malloc(FIELDLEN);
            FieldSet(fdef,f);
            DSSet(f,dim,&ds);
        }
        m[g]=malloc(ds.nob*dim);
// TBD ought to do sanity check
        ERData(e,ds.nob*dim,m[g]);
        ERClose(e);
    }
    for(g=ngens;g<ngens+5;g++)
        m[g]=malloc(ds.nob*dim);
    eff=atoi(argv[3]);
    mkef(eff);

// result is in m[ngens]
    e = EWHdr(argv[4],hdr);    // output the resulting algebra element
    EWData(e,ds.nob*dim,m[ngens]);
    EWClose(e);
    free(f);
    for(g=0;g<ngens+5;g++) free(m[g]);
    free(m);
    return 0;
}

/*  end of zga.c    */
