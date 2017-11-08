/*
      zch.c     meataxe-64 matrix chop
      =====     R. A. Parker    17.07.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "util.h"
 
int main(int argc,  char **argv)
{
    int chops;
    uint64 r1,r2,c1,i,j;
    uint64 rch[100],cch[100],cac[100]; 
    EFIL * e2[100];
    uint64 fdef,nor,noc;
    uint64 hdr[5];
    EFIL *e1;
    FIELD * f;
    DSPACE ds1,ds2;
    Dfmt *v1,*v2;
    char fn[200];
    char *pt;
    int lfn;

    LogCmd(argc,argv);
    if (argc != 3)
    {
        LogString(80,"usage zch m c");
        exit(21);
    }
    chops=atoi(argv[2]);
    e1=ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);
    DSSet(f,noc,&ds1);
    j=nor/chops;
    for(i=0;i<chops;i++) rch[i]=j;
    j=nor-j*chops;
    for(i=0;i<j;i++) rch[i]++;
    j=noc/chops;
    for(i=0;i<chops;i++) cch[i]=j;
    j=noc-j*chops;
    for(i=0;i<j;i++) cch[i]++;
    cac[0]=0;
    for(i=1;i<chops;i++) cac[i]=cac[i-1]+cch[i-1];
    lfn=0;
    pt=argv[1];
    while((*pt) != 0)
        fn[lfn++]=*(pt++);
    fn[lfn]='c';
    fn[lfn+1]='h';
    fn[lfn+6]=0;
    
    v1=malloc(ds1.nob);
    DSSet(f,cch[0],&ds2);
    v2=malloc(ds2.nob);
    for(r1=0;r1<chops;r1++)
    {
        fn[lfn+2]='0'+r1/10;
        fn[lfn+3]='0'+r1%10;
        hdr[2]=rch[r1];
        for(c1=0;c1<chops;c1++)
        {
            fn[lfn+4]='0'+c1/10;
            fn[lfn+5]='0'+c1%10;
            hdr[3]=cch[c1];
            e2[c1]=EWHdr(fn,hdr);
        }
        for(r2=0;r2<rch[r1];r2++)
        {
            ERData(e1,ds1.nob,v1);
            for(c1=0;c1<chops;c1++)
            {
                DSSet(f,cch[c1],&ds2);
                DCut(&ds1,1,cac[c1],v1,&ds2,v2);
                EWData(e2[c1],ds2.nob,v2);

            }
        }
        for(c1=0;c1<chops;c1++)
            EWClose(e2[c1]);
    }
    ERClose(e1);
    free(f);
    free(v1);
    free(v2);
    return 0;

}      /******  end of zch.c    ******/
