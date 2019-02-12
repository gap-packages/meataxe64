/*
      bluf.c     Search for Froenius I(n) blueprints
      ======     R. A. Parker    9.12.2018
*/

#define MAXFLD 200
#define MAXFAC 30
#define MAXGP   1000000

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"

FELT lk[MAXFAC];
uint64_t d;
uint64_t fdef;
FIELD * f;

// gelt = ax*fdef+bx;

uint64_t afm(uint64_t x, uint64_t y)
{
    int ax,ay,az;
    FELT bx,by,bz;
    ax=x/fdef;
    bx=x-ax*fdef;
    ay=y/fdef;
    by=y-ay*fdef;
    FELT q;
    az=(ax+ay)%d;
    q=FieldMul(f,by,lk[ax]);
    bz=FieldAdd(f,q,bx);
    return az*fdef+bz;
}

char got[MAXGP];

uint64_t A[MAXGP],B[MAXGP];

void mkAB()
{
    uint64_t g1,g2,g3,g4,i;
    g1=fdef;      // (ax+0)
    g2=0;         // (0x+0)
    g3=fdef+1;    // (ax+1);
    g4=0;
    for(i=0;i<d;i++)
    {
        A[i]=g2;
        g2=afm(g1,g2);
        B[i]=g4;
        g4=afm(g3,g4);
    }
}

int test3()
{
    uint64_t i,j,k,p2;
    for(i=0;i<fdef*d;i++) got[i]=0;
    for(i=0;i<d;i++)
      for(j=0;j<d;j++)
      {
        p2=afm(A[i],B[j]);
        for(k=0;k<d;k++)
          got[afm(p2,A[k])]=1;
      }
    for(i=0;i<fdef*d;i++) if(got[i]==0) return 0;
    for(i=0;i<fdef*d;i++) got[i]=0;
    for(i=0;i<d;i++)
      for(j=0;j<d;j++)
      {
        p2=afm(B[i],A[j]);
        for(k=0;k<d;k++)
          got[afm(p2,B[k])]=1;
      }
    for(i=0;i<fdef*d;i++) if(got[i]==0) return 0;
    return 1;
}

uint64_t perf3()
{
    return d*d-d+1;
}
 
int main(int argc,  char **argv)
{
    uint64_t qmin1,idx,j;
    FELT e,a,x;
    int i;
    
    f = malloc(FIELDLEN);
    printf(" N order div    primrt\n");
    for(fdef=2;fdef<MAXFLD;fdef++)
    {
        i=FieldASet1(fdef,f,13);  // error, check prime, prim root
        if(i==-1) continue;
        qmin1=fdef-1;
        for(d=2;d<MAXFAC;d++)
        {
            if(d>qmin1) continue;
            if((d*fdef)>MAXGP) continue;
            if((qmin1%d)!=0) continue;
            idx=qmin1/d;
            if(f->pow==1) e=f->conp;
                else      e=f->charc;
            a=1;
            for(j=0;j<idx;j++) a=FieldMul(f,a,e);
            x=1;
            for(i=0;i<d;i++)
            {
                lk[i]=x;
                x=FieldMul(f,x,a);
            }
            mkAB();
            if(test3())
            {
                if(perf3()==fdef)
                    printf("%8lu %3lu I(3) \n",fdef,d);
                continue;
            }
//            printf("%8lu %3lu No I \n",fdef,d);
        }
    }
    return 0;
}

/******  end of bluf.c    ******/
