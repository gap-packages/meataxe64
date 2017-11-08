/*
      zas.c     meataxe-64 matrix assemble (un-chop)
      =====     R. A. Parker    18.07.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    int chops;
    char fn[200];    //  work area for making filenames
    char *pt;
    int lfn;
    uint64 hdr[5];
    uint64 fdef,nor,noc,nor1,noc1;
    FIELD * f;
    DSPACE ds,dsv;
    Dfmt *v1,*v2;
    uint64 r1,r2,c1;
    EFIL *e2[100];
    EFIL *e;
    uint64 cch[100],cac[100]; 

    LogCmd(argc,argv);
    if (argc != 3)
    {
        LogString(80,"usage zas m c");
        exit(21);
    }
    chops=atoi(argv[2]);
    lfn=0;
    pt=argv[1];
    while((*pt) != 0)
        fn[lfn++]=*(pt++);
    fn[lfn]='c';
    fn[lfn+1]='h';
    fn[lfn+2]='0';
    fn[lfn+3]='0';
    fn[lfn+4]='0';
    fn[lfn+5]='0';
    fn[lfn+6]=0;
    EPeek(fn,hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldASet(fdef,f);

    for(r1=1;r1<chops;r1++)
    {
        fn[lfn+2]='0'+r1/10;
        fn[lfn+3]='0'+r1%10;
        EPeek(fn,hdr);
        nor+=hdr[2];
    }
    fn[lfn+2]='0';
    fn[lfn+3]='0';
    for(c1=1;c1<chops;c1++)
    {
        fn[lfn+4]='0'+c1/10;
        fn[lfn+5]='0'+c1%10;
        EPeek(fn,hdr);
        noc+=hdr[3];
    }
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    v2=malloc(ds.nob);
    hdr[2]=nor;
    hdr[3]=noc;
    e=EWHdr(argv[1],hdr);
    nor1=0;                     // stop compiler warnings
    for(r1=0;r1<chops;r1++)
    {
        fn[lfn+2]='0'+r1/10;
        fn[lfn+3]='0'+r1%10;
        noc1=0;
        for(c1=0;c1<chops;c1++)
        {
            fn[lfn+4]='0'+c1/10;
            fn[lfn+5]='0'+c1%10;
            e2[c1]=ERHdr(fn,hdr);
            if(c1==0) nor1=hdr[2];
            else   if(hdr[2]!=nor1)
            {
                LogString(80,"matrices incompatible");
                exit(21);
            }
            cch[c1]=hdr[3];
            cac[c1]=noc1;
            noc1+=hdr[3];
        }
        if(noc1!=noc)
        {
            LogString(80,"matrices incompatible");
            exit(21);
        }
        for(r2=0;r2<nor1;r2++)
        {
            memset(v1,0,ds.nob);
            for(c1=0;c1<chops;c1++)
            {
                DSSet(f,cch[c1],&dsv);
                ERData(e2[c1],dsv.nob,v2);
                DPaste(&dsv,v2,1,cac[c1],&ds,v1);
            }
            EWData(e,ds.nob,v1);
        }
        for(c1=0;c1<chops;c1++)
            ERClose(e2[c1]);
    }
    EWClose(e);
    free(f);
    free(v1);
    free(v2);
    return 0;

}      /******  end of zas.c    ******/
