/*
      zrd.c     meataxe-64 V2.0 Generate a random matrix
      =====     R. A. Parker   29.12.2016 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"

uint64 pseed;
uint64 pseed1;
uint64 pseed2;
static uint64 prand(void)
{
    uint64 x;
    x=1;
    x=x<<63;
    if((x&pseed)!=0) pseed=(pseed<<1)^0x3b4f0bf89;
    else pseed=pseed<<1;
    pseed1=(pseed1*17)%1000003;
    pseed2=(pseed2*19)%1000033;
    return pseed+pseed1+pseed2;
}
 
int main(int argc,  char **argv)
{
    uint64 fdef,nor,noc;
    uint64 hdr[5];
    EFIL * e;
    FIELD * f;
    DSPACE ds;
    Dfmt * v1;
    FELT elt;
    uint64 i,j;
    LogCmd(argc,argv);
    if(  (argc<5) || (argc>6)  )
    {
        LogString(80,"usage zrd m1 fdef nor noc seed");
        exit(14);
    }
    fdef=strtoull(argv[2],NULL,0);
    nor=atoi(argv[3]);
    noc=atoi(argv[4]);
    if(argc==6)
        pseed=atoi(argv[5]);
    else
        pseed=31;
    pseed1=pseed;
    pseed2=pseed;
    for(i=0;i<213;i++) prand();

    hdr[0]=1;
    hdr[1]=fdef;
    hdr[2]=nor;
    hdr[3]=noc;
    hdr[4]=0;
    e = EWHdr(argv[1],hdr);
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(8);
    }
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);

    /******  check that a row fits in memory  */
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        LogString(81,"Can't malloc a single vector");
        exit(9);
    }
    /******  for each row of the matrix  */
    for(i=0;i<nor;i++)
    {
        memset(v1,0,ds.nob);
        for(j=0;j<noc;j++)
        {
            elt=prand()%fdef;
	    DPak(&ds,j,v1,elt);
        }
        EWData(e,ds.nob,v1);
    }
    EWClose(e);
    free(v1);
    free(f);
    return 0;

}      /******  end of zrd.c    ******/
