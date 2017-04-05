/*
      zkc.c     meataxe-64 V2.0 Contract Field
      =====     R. A. Parker   30.12.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    FIELD *f1,*f2;
    DSPACE ds1,ds2;
    Dfmt *v1,*v2;
    uint64_t fdef1,fdef2,nor,noc,i,j;
    uint64_t ratio,good,mode;
    uint64_t hdr[5];
    uint16_t *log16, *alog16;
    FELT x1,x2;

    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zkc m1 <forder> m2");
        exit(21);
    }
    e1=ERHdr(argv[1],hdr);
    fdef1=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    fdef2=strtoul(argv[2],NULL,0);
    f1 = malloc(FIELDLEN);
    f2 = malloc(FIELDLEN);
    FieldASet(fdef1,f1);
    FieldASet(fdef2,f2);
    if( (f1->charc!=f2->charc) || ((f1->pow%f2->pow)!=0) ) 
    {
        LogString(80,"Field contraction not possible");
        exit(23);
    }
    hdr[1]=fdef2;
    e2 = EWHdr(argv[3],hdr);
    DSSet(f1,noc,&ds1);
    DSSet(f2,noc,&ds2);
    v1=malloc(ds1.nob);
    v2=malloc(ds2.nob);
    log16=NULL;        // avoid compiler warnings
    alog16=NULL;
    ratio=(f1->fdef-1)/(f2->fdef-1);

/* decide on strategy */

/* 4.  Give up!  */

    mode=4;              // default - no way.

/* 1 if fields are identical, just copy */
    if(f1->fdef==f2->fdef) mode=1;

/* 2 if moving to the ground field, use natural embedding */
    if((mode==4) && (f2->pow==1) ) mode=2;

/* 3 if log16 and alog16 available, use them */
    if((mode==4) && (f1->fdef <= 65536) )
    {
        log16=(uint16_t *)((uint8_t *)f1 + f1->Tlog16);
        alog16=(uint16_t *)((uint8_t *)f2 + f2->Talog16);

        mode=3;
    }
/* 5 if linear forms available for both fields, extract from */
/*   large field, do linear forms to small field, check that */
/*   the rest is zero then reassemble in small field         */
/*   Not yet written                                         */

    if (mode == 4)
    {
        LogString(80,"This program version cannot handle this case");
        exit(21);
    }

    good=1;

    for(i=0;i<nor;i++)
    {
	ERData(e1,ds1.nob,v1);
        if(mode==1)    // fields identical
        {
            EWData(e2,ds2.nob,v1);
            continue;
        }
        memset(v2,0,ds2.nob);
        for(j=0;j<noc;j++)
        {
            x1=DUnpak(&ds1,j,v1);
            if(mode==2)
            {
                if(x1>=f2->fdef) good=0;
                else x2=x1;  // from ground field
            }
            if(mode==3)
            {
                if(x1==0) x2=0;
                else
                {
                    x2=alog16[log16[x1]/ratio];  // using logs
                    if( (log16[x1]%ratio)!=0 ) good=0;
                }
            }
            if(good==0) break;
            DPak(&ds2,j,v2,x2);
        }
        if(good==0) break;
        EWData(e2,ds2.nob,v2);
    }
    if (good == 0)
    {
        LogString(80,"Matrix not writeable in this field");
        exit(21);
    }
    free(v1);
    free(v2);
    free(f1);
    free(f2);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zkc.c    ******/
