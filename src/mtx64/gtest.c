/*   gtest.c Grease Code testing Module   */
/*   R. A. Parker 1.11.2016       */

//  **** uncomment for verbose ****
// #define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gcodes.h"

int main(int argc, char ** argv)
{
    uint16_t * c;
    uint16_t d[100];
    uint8_t adc[1000];
    uint8_t at[10000];
    int type,prime,i;
    type=atoi(argv[1]);
    prime=atoi(argv[2]);
    c=findcode(type,prime)+1;
    printf("%4d ",prime);
#ifdef DEBUG
    printf("\nlen %d, code ",c[0]);
    for(i=0;i<c[0];i++) printf("%4d",c[i+1]);
    printf("\n");
#endif
    i=mkatab(type,prime,at);
#ifdef DEBUG
    if(type==2)
    {
        for(i=0;i<prime;i++)
        {
            printf(" %3d = %3d-%d  ",i,c[at[2*i]+1],c[at[2*i+1]+1]);
            if(c[at[2*i+1]+1]<10) printf(" ");
            if(c[at[2*i+1]+1]<100) printf(" ");
            if((i%5)==4) printf("\n");
        }
    }
    if(type==3)
    {
        for(i=0;i<prime;i++)
        {
            printf(" %3d = %3d+%3d-%d ",i,
                c[at[3*i]+1],c[at[3*i+2]+1],c[at[3*i+1]+1]);
            if(c[at[3*i+1]+1]<10) printf(" ");
            if(c[at[3*i+1]+1]<100) printf(" ");
            if((i%4)==3) printf("\n");
        }
    }
    if(type==4)
    {
        for(i=0;i<prime;i++)
        {
            printf(" %3d = %3d+%3d-%3d-%d ",i,
                c[at[4*i]+1],c[at[4*i+2]+1],c[at[4*i+1]+1],c[at[4*i+3]+1]);
            if(c[at[4*i+3]+1]<10) printf(" ");
            if(c[at[4*i+3]+1]<100) printf(" ");
            if((i%3)==2) printf("\n");
        }
    }
    printf("\n");
#endif
    i=addchain(type,prime,adc);
    printf("0+%2d[%d]",c[0]-1,i/3-c[0]+2);
    i=0;
    d[0]=0;
    d[1]=1;
    printf(" 0 1");
    while(1)
    {
        if(adc[i]==0) break;
#ifdef DEBUG
        printf(" %2d + %2d -> %2d\n",adc[i],adc[i+1],adc[i+2]);
#else
        d[adc[i+2]]=d[adc[i]]+d[adc[i+1]];
        if(  (d[adc[i+2]]-c[adc[i+2]+1])%prime==0  ) 
             printf(" %d",d[adc[i+2]]);
        else printf(" [%d]",d[adc[i+2]]);
#endif
        i+=3;
    }
    printf("\n");
    return 0;
}

/* end of gtest.c  */
