/*
      zpr.c     meataxe-64 V2.0 convert internal format to text
      =====     R. A. Parker 29.12.2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    FIELD * f;
    Dfmt * v1;
    DSPACE ds;
    int opt,chct,ch;
    uint64 fdef,nor,noc;
    FELT fel;
    uint64 i,j,k,fk;
    uint64 header[5];
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
            ERData(e,8,(uint8*)&k);
            k++;
            if(k>99999) printf(" %lu\n",k);
              else      printf("%6lu\n",k);
        }
        ERClose(e);
        return 0;
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
