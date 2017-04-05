/*   linf.c Linear Forms module   */
/*   R. A. Parker 24.9.2016       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "hpmi.h"
#include "pcrit.h"
#include "slab.h"
#include "linf.h"

// interpreted programs for linear forms 

uint8_t lfAKa[] = { 2,0,1,2,  0 };  //  0 1 0+1
uint8_t lfCKa[] = { 2,0,1,2 , 8,1 ,  0 };
uint8_t lfZKa[] = { 3,2,0,2 , 4,1,0   , 5,1,2 , 1,2,1 , 0 };
uint8_t lfA8[]  = { 2,0,1,3 , 2,1,2,4 , 2,0,4,5 , 0 };  //  0 1 2 0+1 1+2 0+1+2
uint8_t lfC8[]  = { 2,0,1,3 , 2,2,3,5 , 8,1,  8,2,  8,4,  0 };  
uint8_t lfZ8[]  = { 2,4,3,3 , 2,3,5,5 , 2,0,1,1 , 2,3,0,0 , 2,2,5,5 ,
                    2,1,2,2 , 1,0,1   , 2,2,4,0 , 1,5,2   , 0       };
uint8_t lfA16[] = { 2,0,1,4 , 2,2,3,5 , 2,0,2,6 , 2,1,3,7 , 2,4,5,8 ,
                    0 };   // 0 1 2 3 0+1 2+3 0+2 1+3 0+1+2+3
uint8_t lfC16[] = { 2,0,1,4 , 2,0,2,6 , 2,2,4,8 , 2,3,8,8 , 
                    8,1, 8,2, 8,3, 8,5, 8,7, 0 };
uint8_t lfZ16[] = { 2,0,5,5 , 2,3,2,2 , 2,2,1,1 , 2,1,7,7 , 2,1,2,2 ,
                    2,4,1,1 , 2,4,3,3 , 2,5,1,1 , 2,5,6,6 , 2,6,2,2 ,
                    2,6,3,3 , 2,7,0,0 , 2,7,1,1 , 2,7,3,3 , 2,8,3,3 ,
                    0 };


void runlf(DSPACE * dp, uint8_t * pg, Dfmt * pmx, uint64_t nor)
{
    uint8_t * pgc;
    uint8_t opc;
    uint64_t mxs;
    Dfmt *dp1,*dp2,*dp3;
    FELT x;

    mxs=dp->nob*nor;
    pgc=pg;
    while(1)
    {
        opc=*(pgc++);
        switch(opc)
        {
          case 0:         // [ 0 ] end
            return;
          case 1:         // copy R1 R2
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            memcpy(dp2,dp1,dp->nob);
            break;
          case 2:          // [ 2 R1 R2 R3 ] Add R1,R2 giving R3
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            dp3=pmx+(*(pgc++))*mxs;
            DAdd(dp,1,dp1,dp2,dp3);
            break;
          case 3:          //  [ 3 R1 R2 R3 ] R1 - R2 -> R3
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            dp3=pmx+(*(pgc++))*mxs;
            DSub(dp,1,dp1,dp2,dp3);
            break;
          case 4:          //   [ 4 R1 R2 ] R2 += cp0*R1
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,(dp->f)->cp0,1,dp1,dp2);
            break;
          case 5:          //   [ 5 R1 R2 ] R2 +=  (cp1-1)*R1
            x=FieldSub(dp->f,(dp->f)->cp1,1);
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,x,1,dp1,dp2);
            break;
          case 6:           //  [ 6 sc R1 ] R1 = sc * R1
            x=*(pgc++);
            dp1=pmx+(*(pgc++))*mxs;
            DSMul(dp,x,1,dp1);
            break;
          case 7:           // [ 7 sc R1 R2 ] R2 += sc * r1
            x=*(pgc++);
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,x,1,dp1,dp2);
            break;
          case 8:           //  [ 8 R1 ] R1 = 0
            dp1=pmx+(*(pgc++))*mxs;
            memset(dp1,0,dp->nob);
            break;
          default:
            printf("Unknown op-code %d\n",(int)opc);
            exit(22);
        }
    }
}

void extract(DSPACE * dq, const Dfmt *mq, uint64_t nor,
                 Dfmt *mp)
{
    DSPACE dp;
    uint64_t i;
    uint8_t * prog;
    const FIELD * f;
    f=dq->f;
    if(f->pow==2) prog=lfAKa;
    if(f->fdef==8) prog=lfA8;
    if(f->fdef==16) prog=lfA16;
    PSSet(dq->f,dq->noc,&dp);
    for(i=0;i<nor;i++)
    {
        PExtract(dq,mq+i*dq->nob,mp+i*dp.nob,1,nor*dp.nob);
        runlf(&dp,prog,mp+i*dp.nob,nor);
    }
}

void CExtract(DSPACE * dq, const Dfmt *mq, uint64_t nor,
                 Dfmt *mp)
{
    DSPACE dp;
    uint64_t i;
    uint8_t * prog;
    const FIELD * f;
    f=dq->f;
    if(f->pow==2) prog=lfCKa;
    if(f->fdef==8) prog=lfC8;
    if(f->fdef==16) prog=lfC16;
    PSSet(dq->f,dq->noc,&dp);
    for(i=0;i<nor;i++)
    {
        PExtract(dq,mq+i*dq->nob,mp+i*dp.nob,1,nor*dp.nob);
        runlf(&dp,prog,mp+i*dp.nob,nor);
    }
}

void assemble(DSPACE * dq, Dfmt *mq, uint64_t nor,  Dfmt *mp)
{
    DSPACE dp;
    const FIELD * f;
    uint64_t i;
    uint8_t * prog;
    f=dq->f;
    if(f->pow==2) prog=lfZKa;
    if(f->fdef==8) prog=lfZ8;
    if(f->fdef==16) prog=lfZ16;
    PSSet(dq->f,dq->noc,&dp);

    for(i=0;i<nor;i++)
    {
        runlf(&dp,prog,mp+i*dp.nob,nor);
        PAssemble(dq,mp+i*dp.nob,mq+i*dq->nob,1,nor*dp.nob);
    }
}

void LLMul(const FIELD *f,const Dfmt *a,const Dfmt *b,Dfmt *c,
           uint64_t nora,uint64_t noca, uint64_t nocb)
{
    uint64_t nom,sza,szb,szc,i;
    Dfmt *pma,*pmb,*pmc;
    DSPACE dsa,dsb,dpa,dpb;

// set up spaces for extension field
    DSSet(f,noca,&dsa);
    DSSet(f,nocb,&dsb);
// and ground field
    PSSet(f,noca,&dpa);
    PSSet(f,nocb,&dpb);
// how many matrices
    nom=3;
    if(f->fdef==8) nom=6;
    if(f->fdef==16) nom=9;
// allocate matrices
    sza=dpa.nob*nora;
    szb=dpb.nob*noca;
    szc=dpb.nob*nora;
    pma=malloc(sza*nom);
    pmb=malloc(szb*nom);
    pmc=malloc(szc*nom);
// extract a
    extract(&dsa,a,nora,pma);
// extract b
    extract(&dsb,b,noca,pmb);
// call FLMul to muliply pairwise
    for(i=0;i<nom;i++)
    {
        PLMul(f,pma+i*sza,pmb+i*szb,pmc+i*szc,nora,noca,nocb);
    }
// assemble c
    assemble(&dsb,c,nora,pmc);
// free everything
    free(pma);
    free(pmb);
    free(pmc);
}

void linftab(FIELD * f)
{
    f->linfscheme=0; 
    if(f->pow==2)  f->linfscheme=1;
    if( (f->fdef==49) || (f->fdef==121) || (f->fdef==169) )
                   f->linfscheme=0;    // faster using pcbunf
    if(f->fdef==8) f->linfscheme=1;
    if(f->fdef==16) f->linfscheme=1;
}

/* end of linf.c  */
