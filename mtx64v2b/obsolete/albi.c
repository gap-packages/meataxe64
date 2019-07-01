/*
      albi.c     meataxe-64 Generate interpolation t-matrices.
      ======     R. A. Parker   12.4.2019
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "io.h"

/* input is a text file */
/* <characteristic degree number-of-rows> */
/* then some rows with <degree, element>  degree==0 marks the end  */
/* output is the t-matrix used for albe, and zem/alba */

Dfmt v[10000];
FELT rw[100];
Dfmt ex[10000];
Dfmt mu[10000];

int main(int argc,  char **argv)
{
    uint64_t charc,deg,rows,rsf,fdef,x1,fdefi;
    uint64_t hdr[5],hdr1[5];
    int i,j,junk,degi;
    FELT fel,fel1;
    FIELD *f,*fd;
    EFIL *e1,*e2;
    char fname[100];
    DSPACE ds,dsi;
    junk=0;
    junk+=scanf("%d",&i);
    charc=i;
    junk+=scanf("%d",&i);
    deg=i;
    junk+=scanf("%d",&i);
    rows=i;
    printf("Characteristic %lu, degree %lu, rows %lu\n",charc,deg,rows);
    f = malloc(FIELDLEN);
    FieldASet(charc,f);
    DSSet(f,deg,&ds);
    hdr[0]=1;
    hdr[1]=charc;
    hdr[2]=rows;
    hdr[3]=deg;
    hdr[4]=0;
    e1=EWHdr(argv[1],hdr);
    rsf=0;
    while(1)
    {
        junk+=scanf("%d",&i);
        if(i==0) break;
        degi=i;
        junk+=scanf("%d",&i);
        fel=i;
printf("Doing %d with %lu\n",degi,fel);
        if( (degi==1) && (fel==charc) ) // conventional infinity
        {
            memset(v,0,ds.nob);
            DPak(&ds,deg-1,v,1);
            EWData(e1,ds.nob,v);
            rsf++;
            continue;
        }
        if( (degi==1) && (fel==(charc+1)) ) // Karatsuba at front
        {
            memset(v,0,ds.nob);
            DPak(&ds,deg-2,v,1);
            EWData(e1,ds.nob,v);
            DPak(&ds,deg-1,v,1);
            EWData(e1,ds.nob,v);
            rsf+=2;
            continue;
        }
        if( (degi==1) && (fel==(charc+2)) ) // conventional infinity
        {
            memset(v,0,ds.nob);
            DPak(&ds,1,v,1);
            EWData(e1,ds.nob,v);
            DPak(&ds,0,v,1);
            EWData(e1,ds.nob,v);
            rsf+=2;
            continue;
        }
        fdef=1;
        for(i=0;i<degi;i++) fdef=fdef*charc;
        fd = malloc(FIELDLEN);
        FieldASet(fdef,fd);
        fel1=1;
        for(i=0;i<deg;i++)
        {
            rw[i]=fel1;
            fel1=FieldMul(fd,fel1,fel);
        }
        if(degi==1)
        {
            memset(v,0,ds.nob);
            for(i=0;i<deg;i++)
                DPak(&ds,i,v,rw[i]);
            EWData(e1,ds.nob,v);
            free(fd);
            rsf++;
            continue;
        }
// make the matrix of extractions in ex
        for(j=0;j<degi;j++)
            memset(ex+j*ds.nob,0,ds.nob);
        for(i=0;i<deg;i++)
        {
            x1=rw[i];
            for(j=0;j<degi;j++)
            {
                DPak(&ds,i,ex+j*ds.nob,x1%charc);
                x1=x1/charc;
            }
        }
// open the file of the multiplier
        fdefi=1;
        for(i=0;i<degi;i++) fdefi = fdefi*charc;
        sprintf(fname,"ch%lu/ti%lu",charc,fdefi);
        e2=ERHdr(fname,hdr1);
// probably should do some sanity checks here
// read in the multiplier
        DSSet(f,degi,&dsi);
        ERData(e2,hdr1[2]*dsi.nob,mu);
// do the matrix multiplication and write the answer out
        for(i=0;i<hdr1[2];i++)
        {
            memset(v,0,ds.nob);
            for(j=0;j<degi;j++)
            {
                fel=DUnpak(&dsi,j,mu+i*dsi.nob);
                DSMad(&ds,fel,1,ex+j*ds.nob,v);
            }
            EWData(e1,ds.nob,v);
            rsf++;
        }
// close and clean up
        ERClose(e2);
    }
    if(rsf!=rows)
        printf("Error %lu rows written but %lu rows expected\n",rsf,rows);
    if(rsf<rows)
    {
        memset(v,0,ds.nob);
        for(i=rsf;i<rows;i++)
            EWData(e1,ds.nob,v);
    }
    EWClose(e1);
    (void)junk;
    return 0;
}

/* end of albi.c */
