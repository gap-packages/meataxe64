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
uint8_t lfZKa[] = { 3,2,0,2 , 3,2,1,2 , 9,1,2   , 4,2,0 , 5,2,1 , 0};
uint8_t lfACu[] = { 2,0,2,3 , 1,3,4   , 3,3,1,3 , 2,4,1,1 , 3,4,3,4 ,
                    2,4,1,4 , 7,3,2,4 , 0};
uint8_t lfCCu[] = { 0};   // not written yet
uint8_t lfZCu[] = { 3,1,3,1 ,11,4,3,4 , 10,2,1    , 2,3,1,3 , 7,3,0,4,
                    3,3,0,3 , 3,3,2,3 , 11,12,2,4 , 11,6,1,4 ,
               10,6,4 , 3,1,4,1 , 9,2,3  , 4,4,0 , 4,3,1 , 5,4,1 , 5,3,2, 0};
uint8_t lfA8[]  = { 2,0,1,3 , 2,1,2,4 , 2,0,4,5 , 0 };  //  0 1 2 0+1 1+2 0+1+2
uint8_t lfC8[]  = { 2,0,1,3 , 2,2,3,5 , 8,1,  8,2,  8,4,  0 }; 
uint8_t lfZ8[] =  { 2,1,0,0 , 2,4,0,0 , 2,3,1,1 , 2,3,4,4 , 2,4,5,5 ,
                    2,0,1,1 , 2,2,0,0 , 2,5,2,2 , 0 }; 
uint8_t lfA16[] = { 2,0,1,4 , 2,2,3,5 , 2,0,2,6 , 2,1,3,7 , 2,4,5,8 ,
                    0 };   // 0 1 2 3 0+1 2+3 0+2 1+3 0+1+2+3
uint8_t lfC16[] = { 2,0,1,4 , 2,0,2,6 , 2,2,4,8 , 2,3,8,8 , 
                    8,1, 8,2, 8,3, 8,5, 8,7, 0 };
uint8_t lfZ16[] = { 2,1,0,0 , 2,3,2,2 , 2,7,0,0 , 2,0,5,5 , 2,2,0,0 ,
                    9,2,5,    2,2,4,4 , 2,4,1,1 , 2,7,2,2 , 2,4,3,3 ,
                    2,5,3,3 , 2,6,2,2 , 2,6,3,3 , 2,8,3,3 , 0 };
uint8_t lfA27[] = { 2,0,1,3 , 2,2,3,4 , 2,1,4,5 , 0 };
uint8_t lfC27[] = { 0};   // not written yet
uint8_t lfZ27[] = { 9,0,3,    3,5,4,5 , 9,1,5,    3,0,5,0 , 9,2,4,   
                    3,0,1,0 , 3,2,3,2 , 3,2,1,2 , 3,1,4,1 , 0 };
uint8_t lfA32[] = { 2,0,4,6 , 2,2,6,7 , 2,1,7,7 , 2,3,6,6 , 2,1,4,1 ,
                    2,3,2,2 , 2,0,2,5 , 2,6,7,8 , 2,5,7,9 , 2,1,6,11,
                    2,0,8,10, 2,11,2,12 , 0 };
uint8_t lfC32[] = { 0};   // not written yet
uint8_t lfZ32[] = { 9,0,1,    2,6,7,7 , 2,12,2,2 , 2,2,8,8 , 2,10,2,2 ,
                    2,4,12,12 , 2,8,0,0 , 2,1,9,9 , 2,5,0,0 , 2,7,12,12 ,
                    2,7,8,8 , 2,12,1,1 , 2,5,2,2 , 2,5,4,4 , 2,6,4,4 ,
                    2,11,0,0 , 2,11,1,1 , 2,11,4,4 , 2,12,4,4 , 2,1,4,4 ,
                    2,9,1,1 , 2,2,4,4 , 2,9,2,2 , 2,8,9,9 , 2,3,4,4 ,
                    2,9,3,3 , 0 };
uint8_t lfA64[] = { 2,0,1,6 , 2,0,2,7 , 2,0,3,8 , 2,0,4,9 , 2,0,5,10 ,
                    2,1,2,11, 2,1,3,12, 2,1,4,13, 2,1,5,14, 2,2,3,15 ,
                    2,2,4,16, 2,2,5,17, 2,3,4,18, 2,3,5,19, 2,4,5,20, 0};
uint8_t lfC64[] = { 0};   // not written yet
uint8_t lfZ64[] = { 2,14,16,16 , 2,17,18,18 , 2,19,0,0 , 2,20,1,1 , 2,4,0,0 ,
                    2,1,0,0 , 2,0,5,5 , 2,18,5,5 , 9,2,7,    2,7,0,0 ,
                    2,3,1,1 , 2,5,1,1 , 2,5,2,2 , 2,8,11,11 , 2,11,18,18 ,
                    2,18,19,19 , 9,4,7,    2,9,4,4 , 2,12,4,4 , 2,16,1,1 ,
                    2,16,0,0 , 2,20,4,4 , 2,7,5,5 , 2,10,5,5 , 2,13,5,5 ,
                    2,15,5,5 , 2,1,3,3 , 2,6,1,1 , 2,6,5,5 , 2,1,5,5 ,
                    2,3,4,4 , 2,19,3,3 , 2,19,5,5 , 2,3,5,5 , 0 };
uint8_t lfA81[] = { 2,0,1,4 , 2,2,3,5 , 2,0,2,6 , 2,1,3,7 , 2,4,5,8 ,
                    0 };   // 0 1 2 3 0+1 2+3 0+2 1+3 0+1+2+3
uint8_t lfC81[] = { 0};   // not written yet
uint8_t lfZ81[] = { 2,1,3,3 , 3,0,3,0 , 3,3,4,3 , 2,0,2,2 , 2,7,0,0 ,
                    2,2,3,3 , 2,3,1,1 , 2,5,0,0 , 3,1,5,1 , 3,2,6,2 ,
                    3,3,6,3 , 2,8,3,3 , 6,2,1,    6,2,2,    0 };
uint8_t lfA128[]= { 2,0,1,7 , 2,0,2,8 , 2,0,3,9 , 2,0,4,10 , 2,0,5,11,
                    2,0,6,12, 2,1,2,13, 2,1,3,14, 2,1,4,15, 2,1,5,16,
                    2,1,6,17, 2,2,3,18, 2,2,4,19, 2,2,5,20, 2,2,6,21,
                    2,3,4,22, 2,3,5,23, 2,3,6,24, 2,4,5,25, 2,4,6,26,
                    2,5,6,27, 0};
uint8_t lfC128[] = { 0};   // not written yet
uint8_t lfZ128[]= { 2,17,20,20 , 2,20,22,22 , 2,1,0,0 , 2,3,2,2 , 2,5,4,4 ,
                    2,21,23,23 , 2,7,1,1 , 2,24,25,25 , 2,6,4,4 , 2,4,2,2 ,
                    2,26,4,4 , 2,9,3,3 , 2,13,3,3 , 2,2,0,0 , 2,23,2,2 ,
                    2,2,1,1 , 2,8,2,2 , 2,10,14,14 , 2,4,3,3 , 2,14,4,4 ,
                    2,11,5,5 , 2,15,5,5 , 2,18,5,5 , 2,0,6,6 , 2,0,25,25 ,
                    2,22,0,0 , 2,22,4,4 , 2,6,5,5 , 2,12,6,6 , 2,16,6,6 ,
                    2,19,6,6 , 2,25,2,2 , 2,25,3,3 , 2,27,4,4 , 2,27,5,5 ,
                    2,0,1,1 , 2,0,4,4 , 0 };
uint8_t lfA243[] = {2,0,1,6, 3,6,2,10, 3,10,4,10, 3,1,3,8, 2,0,4,7,
                    3,7,2,7, 2,0,2,5,  2,5,4,5,   3,5,1,5,  3,5,3,5,
                    3,2,3,2, 2,1,2,2,  2,1,3,3,   3,5,3,3, 2,2,10,11,
                    2,7,8,9, 0};
uint8_t lfC243[] = { 0};   // not written yet
uint8_t lfZ243[] = { 3,1,6,1 , 3,9,7,9 , 3,9,8,9 , 9,0,5,    3,1,9,1 ,
                    2,2,10,10 , 3,2,11,2 , 2,5,2,2 , 9,4,8,    3,0,1,0 ,
                    3,4,5,4 , 3,4,7,4 , 2,8,10,10 , 3,2,8,2 , 3,9,8,9 ,
                    3,9,0,9 , 3,0,3,0 , 2,3,10,10 , 3,2,3,2 , 3,3,1,3 ,
                    3,1,10,1 , 2,0,2,2 , 2,0,4,4 , 3,4,3,4 , 2,9,3,3 ,
                    6,2,1,    6,2,2,    0 };

uint8_t lfA256[]= { 2,0,1,8 , 2,0,2,9 , 2,0,3,10, 2,0,4,11, 2,0,5,12,
                    2,0,6,13, 2,0,7,14, 2,1,2,15, 2,1,3,16, 2,1,4,17,
                    2,1,5,18, 2,1,6,19, 2,1,7,20, 2,2,3,21, 2,2,4,22,
                    2,2,5,23, 2,2,6,24, 2,2,7,25, 2,3,4,26, 2,3,5,27,
                    2,3,6,28, 2,3,7,29, 2,4,5,30, 2,4,6,31, 2,4,7,32,
                    2,5,6,33, 2,5,7,34, 2,6,7,35, 0};
uint8_t lfC256[] = { 0};   // not written yet
uint8_t lfZ256[] = { 2,20,24,24,2,24,27,27 , 2,25,28,28 , 2,28,30,30 , 2,4,2,2,
                    2,29,31,31 , 2,2,0,0 , 2,32,33,33 , 2,3,1,1 , 2,10,15,15 ,
                    2,34,27,27 , 9,5,7,    2,5,8,8 , 2,11,16,16 , 2,0,7,7 ,
                    2,6,0,0 , 2,1,0,0 , 2,12,17,17 , 2,17,21,21 , 2,30,1,1 ,
                    2,31,2,2 , 2,34,31,31 , 2,33,3,3 , 2,9,2,2 , 2,9,21,21 ,
                    2,13,18,18 , 2,15,3,3 , 2,15,18,18 , 2,16,4,4 , 2,18,22,22 ,
                    2,30,4,4 , 2,21,30,30 , 2,33,30,30 , 2,0,3,3 , 2,7,8,8 ,
                    2,7,2,2 , 2,14,16,16 , 2,16,19,19 , 2,19,23,23 , 2,23,26,26,
                    2,26,33,33 , 2,27,0,0 , 2,27,2,2 , 2,6,27,27 , 2,22,6,6 ,
                    2,31,4,4 , 2,31,6,6 , 2,31,33,33 , 2,33,34,34 , 2,0,30,30 ,
                    2,35,0,0 , 2,35,1,1 , 2,1,34,34 , 2,8,1,1 , 2,8,3,3 ,
                    2,8,6,6 , 2,2,30,30 , 2,35,2,2 , 2,35,3,3 , 2,35,6,6 ,
                    2,3,6,6 , 2,27,3,3 , 2,27,4,4 , 2,27,34,34 , 2,1,3,3 ,
                    2,4,34,34 , 2,7,4,4 , 2,7,30,30 , 2,34,7,7 , 2,5,6,6 ,
                    2,30,5,5 , 0 };

uint8_t lfZ343[]= { 3,3,2,3 , 7,4,3,0 , 7,3,4,0 , 7,5,4,3 , 3,2,4,2 ,
                    3,3,0,3 , 7,5,1,0 , 2,0,2,2 , 7,2,1,2 , 7,4,3,1 ,
                    6,6,0,    6,3,2,    0 };
uint8_t lfA512[]= { 2,0,1,9,  2,0,2,10, 2,0,3,11, 2,0,4,12, 2,0,5,13,
                    2,0,6,14, 2,0,7,15, 2,0,8,16, 2,1,2,17, 2,1,3,18,
                    2,1,4,19, 2,1,5,20, 2,1,6,21, 2,1,7,22, 2,1,8,23,
                    2,2,3,24, 2,2,4,25, 2,2,5,26, 2,2,6,27, 2,2,7,28,
                    2,2,8,29, 2,3,4,30, 2,3,5,31, 2,3,6,32, 2,3,7,33,
                    2,3,8,34, 2,4,5,35, 2,4,6,36, 2,4,7,37, 2,4,8,38,
                    2,5,6,39, 2,5,7,40, 2,5,8,41, 2,6,7,42, 2,6,8,43,
                    2,7,8,44, 0};
uint8_t lfC512[] = { 0};   // not written yet
uint8_t lfZ512[] = { 2,23,28,28 , 2,28,32,32 , 2,32,35,35 , 2,4,0,0 , 2,5,1,1 ,
              2,29,33,33 , 2,33,36,36 , 2,34,37,37 , 2,37,39,39 , 2,38,40,40 ,
              2,11,17,17 , 2,41,42,42 , 2,43,35,35 , 2,44,36,36 , 2,12,18,18 ,
                    9,1,6,    2,6,3,3 , 2,13,19,19 , 2,19,24,24 , 2,24,43,43 ,
                    2,36,9,9 , 2,36,43,43 , 2,39,10,10 , 2,40,17,17 , 2,42,18,18 ,
                    9,2,7,    2,7,3,3 , 2,0,1,1 , 2,0,5,5 , 2,35,0,0 ,
                    2,18,35,35 , 9,3,8,    2,14,7,7 , 2,20,7,7 , 2,25,7,7 ,
                    2,39,7,7 , 2,44,7,7 , 9,7,15,    2,6,43,43 , 2,15,6,6 ,
                    2,21,7,7 , 2,26,7,7 , 2,30,7,7 , 2,40,7,7 , 2,1,2,2 ,
                    2,9,1,1 , 2,9,43,43 , 2,2,3,3 , 2,10,2,2 , 2,10,6,6 ,
                    2,3,4,4 , 2,17,3,3 , 2,17,7,7 , 2,4,5,5 , 2,35,4,4 ,
                    2,8,0,0 , 2,8,3,3 , 2,16,22,22 , 2,22,27,27 , 2,27,31,31 ,
                    2,31,35,35 , 2,35,42,42 , 2,1,43,43 , 2,8,1,1 , 2,8,2,2 ,
                    2,8,6,6 , 2,42,8,8 , 2,2,6,6 , 2,3,7,7 , 2,4,8,8 ,
                    2,5,6,6 , 2,5,7,7 , 2,43,5,5 , 2,43,8,8 , 2,5,8,8 ,
                    0 };

uint8_t lfA625[]= { 2,0,2,5 , 2,2,3,4 , 1,0,6   , 1,0,7   , 7,4,1,5 ,
                    7,2,1,6 , 7,3,1,7 , 7,4,2,6 , 7,4,2,7 , 7,4,3,5 ,
                    7,3,3,6 , 7,2,3,7 , 2,4,1,1 , 2,0,1,1 , 0};
uint8_t lfC625[]= { 0};   // not written yet
uint8_t lfZ625[]= { 3,2,4,2 , 3,6,3,6 , 2,1,5,5 , 3,0,6,0 , 2,2,1,1 ,
                    7,3,3,6 , 7,2,5,6 , 2,6,2,2 , 7,3,6,3 , 3,1,7,1 ,
                    2,5,7,7 , 7,2,1,3 , 7,3,0,1 , 3,0,7,0 , 2,0,7,7 ,
                    3,3,2,3 , 2,7,2,2 , 6,3,0,    6,3,1,    6,4,2,   
                    6,4,3,    0 };

void runlf(DSPACE * dp, uint8_t * pg, Dfmt * pmx, uint64_t nor,Dfmt * tt)
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
          case 5:          //   [ 5 R1 R2 ] R2 +=  cp1*R1
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,(dp->f)->cp1,1,dp1,dp2);
            break;
          case 6:           //  [ 6 sc R1 ] R1 = sc * R1
            x=*(pgc++);
            x=x%(dp->f)->charc;
            dp1=pmx+(*(pgc++))*mxs;
            DSMul(dp,x,1,dp1);
            break;
          case 7:           // [ 7 sc R1 R2 ] R2 += sc * R1
            x=*(pgc++);
            x=x%(dp->f)->charc;
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,x,1,dp1,dp2);
            break;
          case 8:           //  [ 8 R1 ] R1 = 0
            dp1=pmx+(*(pgc++))*mxs;
            memset(dp1,0,dp->nob);
            break;
          case 9:           //  [ 9 R1 R2 ]  swap R1,R2
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            memcpy(tt,dp1,dp->nob);
            memcpy(dp1,dp2,dp->nob);
            memcpy(dp2,tt,dp->nob);
            break;
         case 10:           // [10 sc R1 ] R1 = R1 / sc
            x=*(pgc++);
            x=x%(dp->f)->charc;
            x=FieldInv(dp->f,x);
            dp1=pmx+(*(pgc++))*mxs;
            DSMul(dp,x,1,dp1);
            break;
          case 11:           // [11 sc R1 R2 ] R2 -= sc * R1
            x=*(pgc++);
            x=x%(dp->f)->charc;
            x=FieldNeg(dp->f,x);
            dp1=pmx+(*(pgc++))*mxs;
            dp2=pmx+(*(pgc++))*mxs;
            DSMad(dp,x,1,dp1,dp2);
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
    Dfmt * tt;
    f=dq->f;
    if(f->pow==2) prog=lfAKa;
    if(f->pow==3) prog=lfACu;
    if(f->fdef==8) prog=lfA8;
    if(f->fdef==16) prog=lfA16;
    if(f->fdef==27) prog=lfA27;
    if(f->fdef==32) prog=lfA32;
    if(f->fdef==64) prog=lfA64;
    if(f->fdef==81) prog=lfA81;
    if(f->fdef==128) prog=lfA128;
    if(f->fdef==243) prog=lfA243;
    if(f->fdef==256) prog=lfA256;
    if(f->fdef==512) prog=lfA512;
    if(f->fdef==625) prog=lfA625;
    PSSet(dq->f,dq->noc,&dp);
    tt=malloc(dp.nob);
    for(i=0;i<nor;i++)
    {
        PExtract(dq,mq+i*dq->nob,mp+i*dp.nob,1,nor*dp.nob);
        runlf(&dp,prog,mp+i*dp.nob,nor,tt);
    }
    free(tt);
}

void CExtract(DSPACE * dq, const Dfmt *mq, uint64_t nor,
                 Dfmt *mp)
{
    DSPACE dp;
    uint64_t i;
    uint8_t * prog;
    const FIELD * f;
    Dfmt * tt;

    f=dq->f;
    if(f->pow==2) prog=lfCKa;
    if(f->pow==3) prog=lfCCu;
    if(f->fdef==8) prog=lfC8;
    if(f->fdef==16) prog=lfC16;
    if(f->fdef==27) prog=lfC27;
    if(f->fdef==32) prog=lfC32;
    if(f->fdef==64) prog=lfC64;
    if(f->fdef==81) prog=lfC81;
    if(f->fdef==128) prog=lfC128;
    if(f->fdef==243) prog=lfC243;
    if(f->fdef==256) prog=lfC256;
    if(f->fdef==512) prog=lfC512;
    if(f->fdef==625) prog=lfC625;
    PSSet(dq->f,dq->noc,&dp);
    tt=malloc(dp.nob);
    for(i=0;i<nor;i++)
    {
        PExtract(dq,mq+i*dq->nob,mp+i*dp.nob,1,nor*dp.nob);
        runlf(&dp,prog,mp+i*dp.nob,nor,tt);
    }
    free(tt);
}

void assemble(DSPACE * dq, Dfmt *mq, uint64_t nor,  Dfmt *mp)
{
    DSPACE dp;
    const FIELD * f;
    uint64_t i;
    uint8_t * prog;
    Dfmt * tt;

    f=dq->f;
    if(f->pow==2) prog=lfZKa;
    if(f->pow==3) prog=lfZCu;
    if(f->fdef==8) prog=lfZ8;
    if(f->fdef==16) prog=lfZ16;
    if(f->fdef==27) prog=lfZ27;
    if(f->fdef==32) prog=lfZ32;
    if(f->fdef==64) prog=lfZ64;
    if(f->fdef==81) prog=lfZ81;
    if(f->fdef==128) prog=lfZ128;
    if(f->fdef==243) prog=lfZ243;
    if(f->fdef==256) prog=lfZ256;
    if(f->fdef==343) prog=lfZ343;
    if(f->fdef==512) prog=lfZ512;
    if(f->fdef==625) prog=lfZ625;
    PSSet(dq->f,dq->noc,&dp);
    tt=malloc(dp.nob);
    for(i=0;i<nor;i++)
    {
        runlf(&dp,prog,mp+i*dp.nob,nor,tt);
        PAssemble(dq,mp+i*dp.nob,mq+i*dq->nob,1,nor*dp.nob);
    }
    free(tt);
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
    if(f->pow==3) nom=5;
    if(f->fdef==8) nom=6;
    if(f->fdef==16) nom=9;
    if(f->fdef==27) nom=6;
    if(f->fdef==32) nom=13;
    if(f->fdef==64) nom=21;
    if(f->fdef==81) nom=9;
    if(f->fdef==128) nom=28;
    if(f->fdef==243) nom=12;
    if(f->fdef==256) nom=36;
    if(f->fdef==512) nom=45;
    if(f->fdef==625) nom=8;
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
// call PLMul to muliply pairwise
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
    if(f->pow==3)  f->linfscheme=1;
    if(f->fdef==8) f->linfscheme=1;
    if(f->fdef==16) f->linfscheme=1;
    if(f->fdef==27) f->linfscheme=1;
    if(f->fdef==32) f->linfscheme=1;
    if(f->fdef==64) f->linfscheme=1;
    if(f->fdef==81) f->linfscheme=1;
    if(f->fdef==128) f->linfscheme=1;
    if(f->fdef==243) f->linfscheme=1;
    if(f->fdef==256) f->linfscheme=1;
    if(f->fdef==512) f->linfscheme=1;
    if(f->fdef==625) f->linfscheme=1;
}

/* end of linf.c  */
