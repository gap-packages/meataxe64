/*
      zkx.c     meataxe-64 V2.0 Extend Field
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
    uint64_t fdef1,fdef2,nor,noc;
    uint64_t hdr[5];
    uint64_t i,j;
    EFIL *e1,*e2;
    FIELD *f1,*f2;
    DSPACE ds1,ds2;
    Dfmt *v1,*v2;
    FELT x1,x2,y1,y2;
    int mode;
    uint64_t z;
    FELT * tab;


    LogCmd(argc,argv);
/******  First check the number of input arguments  */
    if (argc != 4)
    {
        LogString(80,"usage zkx m1 <forder> m2");
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
    if( (f1->charc!=f2->charc) || ((f2->pow%f1->pow)!=0) ) 
    {
        LogString(80,"Field extension not possible");
        exit(23);
    }

    hdr[1]=fdef2;
    e2 = EWHdr(argv[3],hdr);
    DSSet(f1,noc,&ds1);
    DSSet(f2,noc,&ds2);
    v1=malloc(ds1.nob);
    v2=malloc(ds2.nob);
    tab=NULL;               // avoid compiler warnings

/* decide on strategy */

/* 3 default at the moment is to use a look-up table */

    mode=3;

/* 1 if fields are identical, just copy */

    if(f1->fdef==f2->fdef) mode=1;

/* 2 if moving from ground field, use natural embedding */
    if((mode==3) && (f1->pow==1) ) mode=2;

/* There should also be a linear forms method.  May   */
/* need a new interface to the linf module.  Basic    */
/* idea is to extract in small field, do linear forms */
/* up to the large field, and to reassemble there     */

/* else allocate look-up table (malloc might fail!)  */
    if(mode==3)
    {
        tab=malloc(f1->fdef*sizeof(FELT));
        if(tab==NULL)
        {
            printf("look-up table allocation failed\n");
            exit(17);
        }
        z=(f2->fdef-1)/(f1->fdef-1);
/* compute y2 = (gen)^z */
        x2=f2->charc;
        y2=1;
/*  answer is y2^z * x2  */
        while(z!=0)
        {
            if( (z%2)==1) y2=FieldMul(f2,y2,x2);
            x2=FieldMul(f2,x2,x2);
            z=z/2;
        }
        y1=f1->charc;
        x1=1;
        x2=1;
/* so x1,y1 (in field1) equals x2,y2 (in field2) */
        for(i=0;i<f1->fdef;i++)
        {
            tab[x1]=x2;
            x1=FieldMul(f1,x1,y1);
            x2=FieldMul(f2,x2,y2);
        }
        tab[0]=0;
    }
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
            if(mode==2) x2=x1;  // from ground field
               else     x2=tab[x1];  // from lookup table
            DPak(&ds2,j,v2,x2);
        }
        EWData(e2,ds2.nob,v2);
    }
    free(v1);
    free(v2);
    free(f1);
    free(f2);
    ERClose(e1);
    EWClose(e2);
    return 0;
}

/******  end of zkx.c    ******/
