/*
      zcv.c     meataxe-64 convert text to internal format
      =====     R. A. Parker   30.09.2015
                With command-line field-order over-ride
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
int hadcr;
FIELD * f;
FELT dig[11];
uint64_t fdef;
 
    /******  subroutine to get an integer like FORTRAN does  */
uint64_t getin(uint64_t a)
{
    int c;
    uint32_t i,j;
    j=0;
 
    if(hadcr == 1) return j;
    for(i=0;i<a;i++)
    {
        c = getchar();
        if(c == '\n')
        {
            hadcr = 1;
            return j;
        }
        if(c < '0') c = '0';
        if(c > '9') c = '0';
        j = 10*j + (c-'0');
    }
    return j;
}

    /****** get single character as a field element  */
uint64_t getaz(void)
{
    int c;
    if(hadcr == 1) return 0;
    c = getchar();
    if(c == '\n')
    {
        hadcr = 1;
        return 0;
    }
    if( (c>='0') && (c<='9') ) return c-'0';
    if( (c>='A') && (c<='Z') ) return c-'A'+10;
    if( (c>='a') && (c<='z') ) return fdef -1 + (c-'z');
    return 0;
}
 
/* subroutine to get an integer and reduce mod p */
FELT getz()
{
    FELT res,res1;
    int c, flag;
    flag=1;
    res=0;
    c=getchar();
    while ((c==' ')||(c=='\n')) c=getchar();
    if(c=='-') 
    {
        flag=2;
        c=getchar();
    }
    while ((c!=' ')&&(c!='\n'))
    {
        if((c<'0')||(c>'9'))
        {
            LogString(80,"Illegal character in input");
            exit(7);
        }
        res1 = FieldMul(f,dig[10],res);
        res = FieldAdd(f,dig[c-'0'],res1);
        c=getchar();
    }
    if (flag==2) 
        return FieldNeg(f,res);
    else
        return res;
}
static void nextline()
{
    if(hadcr == 1)
    {
        hadcr=0;
        return;
    }
    while (getchar() != '\n');
}
 
int main(int argc,  char **argv)
{
    EFIL * e;
    uint64_t hdr[5];
    uint64_t nor,noc,mode;
    int res;
    DSPACE ds;
    FELT fel;

    uint64_t i,j,k;
    Dfmt * v1;
    LogCmd(argc,argv);
    res=0;
    /******  First check the number of input arguments  */
    if ((argc != 2) && (argc != 3) )
    {
        LogString(80,"usage zcv <m1> <optional field order> < <textfile>");
        exit(14);
    }

    hadcr = 0;
    res+=scanf("%lu",&mode);
    res+=scanf("%lu",&fdef);
    res+=scanf("%lu",&nor);
    res+=scanf("%lu",&noc);
    if(argc==3) fdef=atol(argv[2]);

    if( (mode==12) || (mode==13) )     // maps
    {
        hdr[0]=3;
        hdr[1]=fdef;
        hdr[2]=nor;
        hdr[3]=noc;
        if(mode==12) hdr[3]=nor;
        hdr[4]=0;
        e = EWHdr(argv[1],hdr);
        for(i=0;i<nor;i++)
        {
            res+=scanf("%lu",&k);
            k--;
            EWData(e,8,(uint8_t *) &k);
        }
        EWClose(e);
        return 0;
    }
 
    /******  open the output file  */
    hdr[0]=1;
    hdr[1]=fdef;
    hdr[2]=nor;
    hdr[3]=noc;
    hdr[4]=0;
    e = EWHdr(argv[1],hdr);
 
    /******  initialize the arithmetic  */
    /******  tell it what field to use  */
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(8);
    }
    FieldASet(fdef,f);
    dig[0]=0;
    dig[1]=1;
    for(i=2;i<=10;i++)
        dig[i]=FieldAdd(f,dig[1],dig[i-1]);
    /****** and how long the rows are  */
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
 
    /******  start off with the row = 0  */
        memset(v1,0,ds.nob);
 
        switch(mode)
        {
          case 1:
    /******  then for each entry in the row  */
            for(j=0;j<noc;j++)
            {
                if(j%80 == 0) nextline();
                k=getaz();

    /******  put the entry in place  */
                fel=k;
                DPak(&ds,j,v1,fel);
            }
            break;

          case 2:
 
    /******  then put the 1 into the correct place  */
            res+=scanf("%lu",&k);
            DPak(&ds,k-1,v1,1);
            break;

          case 3:
    /******  then for each entry in the row  */
            for(j=0;j<noc;j++)
            {
                if(j%25 == 0) nextline();
                k=getin(3);

    /******  put the entry in place  */
                fel=k;
                DPak(&ds,j,v1,fel);
            }
            break;
 
          case 4:
    /******  then for each entry in the row  */
            for(j=0;j<noc;j++)
            {
                if(j%5 == 0) nextline();
                k=getin(8);

    /******  put the entry in place  */
                fel=k;
                DPak(&ds,j,v1,fel);
            }
         break;
 
          case 5:
    /******  then for each entry in the row  */
            for(j=0;j<noc;j++)
            {
                k=getz();

    /******  put the entry in place  */
                fel=k;
                DPak(&ds,j,v1,fel);
            }
            break;

          case 6:
    /******  then for each entry in the row  */
            for(j=0;j<noc;j++)
            {
                res+=scanf("%lu",&k);
    /******  put the entry in place  */
                fel=k;
                DPak(&ds,j,v1,fel);
            }
            break;
 
          default:
          {
            LogString(80,"mode unknown");
            exit(9);
          }
        }
 
    /******  write a row of the matrix  */
        EWData(e,ds.nob,v1);
    }
    EWClose(e);
    free(v1);
    free(f);
    (void)res;
    return 0;

}      /******  end of zcv.c    ******/
