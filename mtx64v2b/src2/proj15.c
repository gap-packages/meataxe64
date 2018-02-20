/*   proj15.c Projective grease level 1.5   */
/*            R. A. Parker 16.1.2018        */

// proj15 <field order> <seed>

#define MAXVEC 200
#define MAXFIELD 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"

FIELD *f;
DSPACE ds;
Dfmt * v[MAXVEC];
int vecs;
uint64_t q,lines;
char got[MAXFIELD*MAXFIELD+MAXFIELD+2];

int compdef(void)
{
    int i,j,def;
    uint64_t pvec,col;
    Dfmt w[24];
    FELT fel;
    FELT f0,f1,f2;
    for(i=0;i<lines;i++) got[i]=0;
    def=lines;
    for(i=0;i<vecs;i++)
    {
        for(j=i;j<vecs;j++)
        {
            for(fel=1;fel<q;fel++)
            {
                memcpy(w,v[i],ds.nob);
                DSMad(&ds,fel,1,v[j],w);
                col=DNzl(&ds,w);
                if(col==ZEROROW) continue;
                f0=DUnpak(&ds,col,w);
                f0=FieldInv(f,f0);
                DSMul(&ds,f0,1,w);
                f0=DUnpak(&ds,0,w);
                f1=DUnpak(&ds,1,w);
                f2=DUnpak(&ds,2,w);
                pvec=0;
                if(f0==1) pvec=f1*q+f2;
                if((f0==0) && (f1==1)) pvec=q*q+f2;
                if((f0==0) && (f1==0) && (f2==1) ) pvec=q*(q+1);
                if(got[pvec]==0)
                {
                    got[pvec]=1;
                    def--;
                    if(def==0) return 0;
                }
            }
        }
    }
    return def;
}

int main(int argc, char ** argv)
{
    uint64_t fdef;
    int seed,i,bsf,def;
    int ks1,ks2;
    FELT ks3,kv0,kv1,kv2;
    int s1[MAXVEC];
    int s2[MAXVEC];
    FELT s3[MAXVEC];

    fdef=atoi(argv[1]);
    if(fdef>MAXFIELD)
    {
        printf("Field too big - is %ld, should be at most %d\n",fdef,MAXFIELD);
        exit(2);
    }
    f = malloc(FIELDLEN);
    FieldASet1(fdef,f,5);  // full checking
    DSSet(f,3,&ds);
    for(i=0;i<MAXVEC;i++)
    {
        v[i]=malloc(ds.nob);
        memset(v[i],0,ds.nob);
    }
    for(i=0;i<3;i++)
    {
        DPak(&ds,i,v[i],1);   // set basis
        s1[i]=0;
        s2[i]=0;
        s3[i]=1;   // say
    }
    seed=atoi(argv[2]);
    srand(seed);
    bsf=MAXVEC-1;
    q=fdef;
    lines=q*q+q+1;
    while(1)   // keep finding better ones
    {
        while(1)    // keep looking at random ones
        {
            vecs=3;
            while(1)   // add a vector
            {
                ks1=rand()%vecs;
                ks2=rand()%vecs;
                if(ks1==ks2) continue;
                ks3=rand()%fdef;
                if(ks3==0) continue;
                memcpy(v[vecs],v[ks1],ds.nob);
                DSMad(&ds,ks3,1,v[ks2],v[vecs]);
                s1[vecs]=ks1;
                s2[vecs]=ks2;
                s3[vecs]=ks3;
                def=compdef();
                if(def!=0) 
                {
                    vecs++;
                    if(vecs>=bsf) break;
                    continue;
                }
                bsf=vecs;
                printf("\n\n Field %ld, Work %d\n",fdef,vecs-3);
                for(i=0;i<vecs;i++)
                {
                    kv0=DUnpak(&ds,0,v[i]);
                    kv1=DUnpak(&ds,1,v[i]);
                    kv2=DUnpak(&ds,2,v[i]);
                    printf("%3d %3d %4ld %4ld %4ld %4ld\n",
                          s1[i],s2[i],s3[i],kv0,kv1,kv2);
                }
                break;
            }
        }
    }
}

/* end of proj15.c  */
