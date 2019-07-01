/*
      alba.c     meataxe-64 elementary matrices to linf 'c' code
                 for assembly direction.
      ======     R. A. Parker    6.4.2019
*/

#define MAXPROG 10000

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int lin;
FILE *f2;

int pg[MAXPROG];

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

void progend(void)
{
    fprintf(f2, " 0 };\n");
}

int main(int argc,  char **argv)
{
    FILE *f1;
    int junk=0;
    int inp,pctr,cmd,sc,R1,R2,R3;
    if(argc!=3)
    {
        printf("usage alba in outpg\n");
        exit(42);
    }

    f1=fopen(argv[1],"rb");
    f2=fopen(argv[2],"wb");
    fprintf(f2,"uint8_t lfZXX[]  = { ");
    lin=0;

    pctr=0;
// read in "program"
    while(1)
    {
        junk+=fscanf(f1,"%d",&inp);
        if(inp==0) break;
        pg[pctr++]=inp;
        if((pctr+10)>MAXPROG)
        {
            printf("Program too large - increase MAXPROG\n");
            exit(18);
        }
        junk+=fscanf(f1,"%d",pg+(pctr++));
        junk+=fscanf(f1,"%d",pg+(pctr++));
        if( (inp==2) || (inp==3) || (inp== 4) ||
            (inp==8) || (inp==9) || (inp==10) || (inp==11) ) pctr++;
        else junk+=fscanf(f1,"%d",pg+(pctr++));  
    }

    while(1)
    {
        if(pctr==0)
        {
            progend();
            break;
        }
        pctr-=4;
        cmd=pg[pctr];
        switch(cmd)
        {
          case 1:    // add sc.R2 into R1
            sc=pg[pctr+1];
            R1=pg[pctr+2];
            R2=pg[pctr+3];
printf("Add %d.R%d into R%d (1)\n",sc,R2,R1);
            prog4(7,sc,R2,R1);
            break;
          case 2:    // add R2 into R1
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Add R%d into R%d (2)\n",R2,R1);
            prog4(2,R2,R1,R1);
            break;
          case 3:    // subtract R2 from R1
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Subtract R%d from R%d (3)\n",R2,R1);
            prog4(3,R1,R2,R1);
            break;
          case 4:    // sc multiply R1
            sc=pg[pctr+1];
            R1=pg[pctr+2];
printf("Scalar %d multiply R%d (4)\n",sc,R1);
            prog3(6,sc,R1);
            break;
          case 5:    // add R3 into R1 and R2 then trash
            R1=pg[pctr+1];
            R2=pg[pctr+2];
            R3=pg[pctr+3];
printf("Add R%d into R%d and R%d and trash(5)\n",R3,R1,R2);
            prog4(2,R3,R1,R1);
            prog4(2,R3,R2,R2);
            break;
          case 6:    // add R3 into R1, subtract R3 from R2 then trash
            R1=pg[pctr+1];
            R2=pg[pctr+2];
            R3=pg[pctr+3];
printf("Add R%d into R%d, subtract R%d from R%d and trash (6)\n",R3,R1,R3,R2);
            prog4(2,R3,R1,R1);
            prog4(3,R2,R3,R2);
            break;
          case 7:    // add sc.R2 into R1 and trash R2
            sc=pg[pctr+1];
            R1=pg[pctr+2];
            R2=pg[pctr+3];
printf("Add %d.R%d into R%d and trash (7)\n",sc,R2,R1);
            prog4(7,sc,R2,R1);
            break;
          case 8:    // add R2 into R1 then trash
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Add R%d into R%d and trash (8)\n",R2,R1);
            prog4(2,R2,R1,R1);
            break;
          case 9:    // subtract R2 from R1 then trash
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Subtract R%d from R%d and trash (9)\n",R2,R1);
            prog4(3,R1,R2,R1);
            break;
          case 10:    // swap R1 and R2
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Swap R%d and R%d (10)\n",R1,R2);
            prog3(9,R1,R2);
            break;
          case 11:    // move R2 to R1
            R1=pg[pctr+1];
            R2=pg[pctr+2];
printf("Move R%d to R%d (11)\n",R2,R1);
            prog3(1,R2,R1);
            break;
          default:
            printf("Unknown command %d in input\n",cmd);
            break;
        }
    }

    return 0;
}

/* end of alba.c */
