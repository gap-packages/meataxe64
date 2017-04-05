/*
      bartest.c     Test Barrett values
      ======     R. A. Parker    21.10.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SHIFT 41

int main(int argc,  char **argv)
{
    uint64_t fdef,a,b,x,y,bars;

    if (argc != 2)
    {
        printf("usage bartest <fieldorder>\n");
        exit(21);
    }
    fdef=atoi(argv[1]);
    x=1;
    x=x<<SHIFT;
    bars=x/fdef+1;
    b=fdef-1;
    a=0;
    while(1)
    {
        x=a*fdef+b;
        y=(x*bars)>>SHIFT;
        if(y!=a) break;
        a++;
    }
    printf("field %lu shift %d first failure at a = %lu\n",fdef,SHIFT,a);
    return 0;
}

/******  end of bartest.c    ******/
