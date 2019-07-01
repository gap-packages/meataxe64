/*
      zpr.c     meataxe-64 V2.0 convert internal format to text
      =====     R. A. Parker 29.12.2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "bitstring.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    FIELD * f;
    Dfmt * v1;
    DSPACE ds;
    int opt,chct,ch,b1;
    uint64_t fdef,nor,noc;
    uint64_t degree,nopolys;
    uint64_t *bs;
    uint64_t siz;
    FELT fel;
    uint64_t i,j,k,fk;
    uint64_t sp,de;        // sparse and dense blocks
    uint64_t header[5];
    LogCmd(argc,argv);

    if ( (argc < 2) || (argc>3) )
    {
        LogString(80,"usage zpr <m1> <opt> > text");
        exit(14);
    }
    if(argc==3) opt=atoi(argv[2]);
        else    opt=0;
    e = ERHdr(argv[1],header);
    fdef=header[1];
    nor=header[2];
    noc=header[3];
    if(header[0]==2)    // bitstring
    {
        siz=8*((header[2]+63)/64 + 2);
        bs=malloc(siz);
        ERData(e,siz,(uint8_t *) bs);
        ERClose(e);
        printf(" %d %d\n",(int)bs[0],(int)bs[1]);
        for(i=0;i<header[2];i++)
        {
            b1=BSBitRead(bs,i);
            printf("%d",b1);
            if( (i%80)==79 ) printf("\n");
        }
        if( (i%80)!=79 ) printf("\n");      
        free(bs);
        return 0;
    }
    if(header[0]==3)    // permutations and maps
    {
        if(opt==0)
        {
            if(nor==noc)
            {
                opt=12;
                noc=1;
            }
            else opt=13;
        }
        printf("%2d     1",opt);
        if(nor <99999) printf("%6lu",nor);
            else       printf(" %lu",nor);
        if(noc <99999) printf("%6lu\n",noc);
                  else printf(" %lu\n",noc);
        for(i=0;i<nor;i++)
        {
            ERData(e,8,(uint8_t *)&k);
            k++;
            if(k>99999) printf(" %lu\n",k);
              else      printf("%6lu\n",k);
        }
        ERClose(e);
        return 0;
    }
    if(header[0]==4)  // polynomial
    {
        degree=header[2];
        nopolys=header[3];
        fdef=header[1];
        f = malloc(FIELDLEN);
        FieldASet(fdef,f);
        DSSet(f,degree,&ds);
        v1=malloc(ds.nob);
        printf(" 31 %lu %lu %lu\n",fdef,degree,nopolys);
        for(i=0;i<nopolys;i++)
        {
            ERData(e,8,(uint8_t *)&j);
            DSSet(f,j+1,&ds);
            ERData(e,ds.nob,v1);
            printf(" %lu   ",j);
            for(k=0;k<=j;k++)
            {
                fel=DUnpak(&ds,k,v1);
                printf("%lu ",fel);
            }
            printf("\n");
        }
        ERClose(e);
        free(f);
        free(v1);
        return 0;
    }
    if(header[0]==5)  // Sparsified matrix - drops through
    {
        ERData(e,8,(uint8_t *)&sp);
        ERData(e,8,(uint8_t *)&de);
        printf("%ld sparse blocks\n",sp);
        for(i=0;i<sp;i++)
        {
            ERData(e,8,(uint8_t *)&j);
            printf(" start %lu ",j);
            ERData(e,8,(uint8_t *)&j);
            printf(" length %lu ",j);
            ERData(e,8,(uint8_t *)&j);
            printf(" destination %lu\n",j);
        }
        nor=0;
        printf("%ld dense blocks\n",de);
        for(i=0;i<de;i++)
        {
            ERData(e,8,(uint8_t *)&j);
            printf(" start %lu ",j);
            ERData(e,8,(uint8_t *)&j);
            printf(" length %lu\n",j);
            nor+=j;
        }        
    }
    if(opt==0)
    {
        opt=1;
        if(fdef>9) opt=3;
        if(fdef>999) opt=4;
        if(fdef>99999999) opt=6;
    }
    printf("%2d",opt);
    if(fdef<99999) printf("%6lu",fdef);
        else       printf(" %lu",fdef);
    if(nor <99999) printf("%6lu",nor);
        else       printf(" %lu",nor);
    if(noc <99999) printf("%6lu\n",noc);
        else printf(" %lu\n",noc);
    f = malloc(FIELDLEN);
    if(f==NULL)
    {
        LogString(81,"Can't malloc field structure");
        exit(15);
    }
    FieldASet(fdef,f);
    DSSet(f,noc,&ds);
    v1=malloc(ds.nob);
    if(v1==NULL)
    {
        LogString(81,"Can't malloc a single vector");
        exit(19);
    }
    chct=0;
    for(i=0;i<nor;i++)
    {
	ERData(e,ds.nob,v1);
	for(j=0;j<noc;j++)
	{
	    fel=DUnpak(&ds,j,v1);
            switch(opt)
            {
              case 1:
                if(chct==80)
                {
                    printf("\n");
                    chct=0;
                }
                ch='*';
                if(fel<10) ch='0'+fel;
                if( (ch=='*')  && (fel<36) ) ch='A'+fel-10;
                if( (ch=='*')  && (fdef-fel<27) ) ch='z'-fdef+fel+1;
                printf("%c",ch);
                chct++;
                break;
              case 3:
                if(chct==75)
                {
                    printf("\n");
                    chct=0;
                }
                if(fel<1000) printf("%3lu",fel);
                      else   printf("***");
                chct+=3;
                break;
              case 4:
                if(chct==40)
                {
                    printf("\n");
                    chct=0;
                }
                if(fel<10000000) printf("%8lu",fel);
                      else   printf("********");
                chct+=8;
                break;
              case 6:
                k=1;
                fk=fel;
                while(fk>=10)
                {
                    fk=fk/10;
                    k++;
                }
                if((chct+k)>=80)
                {
                    printf("\n");
                    chct=0;
                } 
                printf(" %lu",fel);
                chct+=k;
                break;
              default:
                printf("Unknown mode\n");
            }
	}
        printf("\n");
        chct=0;
    }
    ERClose(e);
    free(v1);
    free(f);
    return 0;
}
/*  end of zpr.c    */
