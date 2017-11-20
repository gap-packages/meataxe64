/*
      zut.c     meataxe-64 Utility routines
      =====     R. A. Parker   24.05.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "bitstring.h"
#include "pcrit.h"
 
int main(int argc,  char **argv)
{
    EFIL *e;
    int mode,b1,b2,c,res,gens;
    uint64_t hdr[5];
    uint64_t siz,i;
    uint64_t * bs;
    LogCmd(argc,argv);
    if (argc < 2)
    {
        LogString(80,"usage zut mode . . .");
        printf(" 1 resultfile < text   Input bit string\n");
        printf(" 2 bitstring           Print bit string\n");
        printf(" 3 bitstring           Print ""Nullity "" then number of unset bits\n");
        printf(" 4 bitstring           return 0 unless bits is 0, in which case 1\n");
        printf(" 5 bitstring           Print ""Split result subspace nnn quotient nnn\n");
        printf(" 6 algfile #gens       Make simple algebra file\n");
        exit(21);
    }
    mode = atoi(argv[1]);
    if(mode==1)
    {
        res=scanf("%d%d",&b1,&b2);
        siz=8*((b1+63)/64 + 2);
        bs=malloc(siz);
        memset(bs,0,siz);
        bs[0]=b1;
        bs[1]=b2;
        for(i=0;i<b1;i++)
        {
            while(1)
            {
                c=getchar();
                if(c=='0')
                    break;
                if(c=='1')
                {
                    b2--;
                    BSBitSet(bs,i);
                    break;
                }
            }
        }
        hdr[0]=2;   // bitstring
        hdr[1]=1;   // always 1
        hdr[2]=b1;
        hdr[3]=bs[1];
        hdr[4]=0;
        e=EWHdr(argv[2],hdr);
        EWData(e,siz,(uint8_t *) bs);
        EWClose(e);
        if(b2!=0) printf("Error - incorrect number of set bits\n");        
        free(bs);
        if(res==99) printf("Urgle");
        return 0;
    }
    if(mode==2)
    {
        e=ERHdr(argv[2],hdr);
        siz=8*((hdr[2]+63)/64 + 2);
        bs=malloc(siz);
        ERData(e,siz,(uint8_t *) bs);
        ERClose(e);
        printf(" %d %d\n",(int)bs[0],(int)bs[1]);
        for(i=0;i<hdr[2];i++)
        {
            b1=BSBitRead(bs,i);
            printf("%d",b1);
            if( (i%80)==79 ) printf("\n");
        }
        if( (i%80)!=79 ) printf("\n");      
        free(bs);
        return 0;
    }
    if(mode==3)
    {
        EPeek(argv[2],hdr);
        printf("Nullity %ld\n",(long)(hdr[2]-hdr[3]));
        return 0;
    }
    if(mode==4)
    {
        EPeek(argv[2],hdr);
        if(hdr[3]==0) return 1;
        return 0;
    }
    if(mode==5)
    {
        EPeek(argv[2],hdr);
        if(hdr[2]!=hdr[3])
        {
            printf("Split result subspace %ld, quotient %ld\n",
                  (long)hdr[3],(long)hdr[2]-(long)hdr[3]);
            return 0;
        }
        else
        {
            printf("Whole Space\n");
            return 1;
        }
    }
    if(mode==6)
    {
        gens=atoi(argv[3]);
        hdr[0]=4;
        hdr[1]=1;
        hdr[2]=gens;
        hdr[3]=0;
        hdr[4]=0;
        e=EWHdr(argv[2],hdr);
        EWClose(e);
        return 0;     
    }
    printf("Unknown mode %d\n",mode);
    return 17;

}      /******  end of zus.c    ******/
