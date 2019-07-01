/*
      albe.c     meataxe-64 elementary matrices to linf 'c' code
                 for extraction direction.
      ======     R. A. Parker    5.4.2019
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int lin;
FILE *f2;

void prog2(int a, int b)
{
    fprintf(f2, " %d,%d ,",a,b);
    lin++;
    if(lin==5)
    {
        fprintf(f2,"\n                  ");
        lin=0;
    }
}

void prog3(int a, int b, int c)
{
    fprintf(f2, " %d,%d,%d ,",a,b,c);
    lin++;
    if(lin==5)
    {
        fprintf(f2,"\n                  ");
        lin=0;
    }
}

void prog4(int a, int b, int c, int d)
{
    fprintf(f2, " %d,%d,%d,%d ,",a,b,c,d);
    lin++;
    if(lin==5)
    {
        fprintf(f2,"\n                  ");
        lin=0;
    }
}



void progend(void)
{
    fprintf(f2, " 0 };\n");
}

int main(int argc,  char **argv)
{
    FILE *f1;
    int cmd,R1,R2,R3,sc;
    int junk=0;

    if(argc!=3)
    {
        printf("usage albe in outpg\n");
        exit(42);
    }
    f1=fopen(argv[1],"rb");
    f2=fopen(argv[2],"wb");
    fprintf(f2,"uint8_t lfAZZ[]  = { ");
    lin=0;
    while(1)
    {
        junk+=fscanf(f1,"%d",&cmd);
        if(cmd==0) break;
        switch(cmd)
        {
          case 1:    // R2 += sc.R1
            junk+=fscanf(f1,"%d",&sc);
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Add %d.R%d into R%d (1)\n",sc,R1,R2);
            prog4(7,sc,R1,R2);
            break;
          case 2:    // R2 += R1
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Add R%d into R%d (2)\n",R1,R2);
            prog4(2,R1,R2,R2);
            break;
          case 3:    // R2 -= R1
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Subtract R%d from R%d (3)\n",R1,R2);
            prog4(3,R2,R1,R2);
            break;
          case 4:    // R1 = sc.R1
            junk+=fscanf(f1,"%d",&sc);
            junk+=fscanf(f1,"%d",&R1);
printf("R%d = %d.R%d (4)\n",R1,sc,R1);
            prog3(6,sc,R1);
            break;
          case 5:    // R3 = R1 + R2
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
            junk+=fscanf(f1,"%d",&R3);
printf("Add R%d into R%d giving R%d (5)\n",R1,R2,R3);
            prog4(2,R1,R2,R3);
            break;
          case 6:    // R3 = R1 - R2
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
            junk+=fscanf(f1,"%d",&R3);
printf("Subtract R%d from R%d giving R%d (5)\n",R2,R1,R3);
            prog4(3,R1,R2,R3);
            break;
          case 7:    // R2 = sc.R1
            junk+=fscanf(f1,"%d",&sc);
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Set R%d = %d.R%d (7)\n",R2,sc,R1);
            prog3(1,R1,R2);
            prog3(6,sc,R1);
            break;
          case 8:    // R2 = R1
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("R%d = R%d (8)\n",R2,R1);
            prog3(1,R1,R2);
            break;
          case 9:    // R2 = -R1
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("R%d = -R%d (9)\n",R2,R1);
            prog2(8,R2);
            prog4(3,R1,R2,R2);
            break;
          case 10:    // Swap R1 and R2
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Swap R%d and R%d (10)\n",R1,R2);
            prog3(9,R1,R2);
            break;
          case 11:    // Move R1 to R2
            junk+=fscanf(f1,"%d",&R1);
            junk+=fscanf(f1,"%d",&R2);
printf("Move R%d to R%d (11)\n",R1,R2);
            prog3(1,R1,R2);
            break;
          default:
            printf("Unknown command %d in input\n",cmd);
            break;
        }
    }
    progend();
    (void)junk;
    fclose(f2);
    fclose(f1);
    return 0;
}

/* end of albe.c */
