/*
      adc.c     meataxe-64 Addition chain program
      =====     R. A. Parker    18.10.2017
*/

// Input is a file with the format . . .
//   <prime>  <nvals> <exptlength>  <val> <val> ...
//  and the output is a byte-code program

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc,  char **argv)
{
    FILE *f1;
    int i,j,k,prime,nvals,exptlength;
    int v[80];
    char g[i];
    f1=fopen(argv[1],"rb");
    while(1)
    {
        fscanf(f1,"%d",&prime);
        if(prime==-1) break;
        fscanf(f1,"%d",&nvals);
        fscanf(f1,"%d",&exptlength);
printf("%d %d %d\n",prime,nvals,exptlength);
        for(i=1;i<=nvals;i++) fscanf(f1,"%d",&v[i]);
        v[0]=0;
        nvals++;
        for(i=0;i<prime;i++) g[i]=0;
        for(i=0;i<nvals;i++)
          for(j=0;j<nvals;j++)
            for(k=0;k<nvals;k++)
                g[(prime+v[i]+v[j]-v[k])%prime]=1;
         for(i=0;i<prime;i++)
         {
             if(g[i]==1) continue;
             printf("can't make %d\n",i);
             break;
         }
    }
    printf("End of processing\n");
}

/* end of adc.c */
