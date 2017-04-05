/*
         psn1.c  -   Parker's first syntax notation
         ======      R. A. Parker  5.6.2015
*/

#define INITIALALLOC 32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "psn1.h"

// routines for use in the proggies

void PSN1Init(PSN1 * ps)
{
    ps->p=malloc(INITIALALLOC);
    ps->len=0;
    ps->alc=INITIALALLOC;
}

char * PSN1Point(PSN1 * ps)
{
    return ps->p;
}

void PSN1Grow(PSN1 * ps, long len)
{
    long newalc;
    if((ps->len+len)>ps->alc)
    {
        newalc=ps->len+len;
        if(newalc<(ps->alc+ps->alc/4)) newalc=ps->alc+ps->alc/4;
        ps->p=realloc(ps->p,newalc);
        ps->alc=newalc;
    }
}

void PSN1APut(PSN1 * ps, char * ch, long len)
{
    PSN1Grow(ps,len);
    memcpy(ps->p+ps->len,ch,len);
    ps->len+=len;
}

void PSN1AZero(PSN1 * ps, long len)
{
    PSN1Grow(ps,len);
    memset(ps->p+ps->len,0,len);
    ps->len+=len;
}

void PSN1ASkip(PSN1 * ps, long len)
{
    ps->len+=len;
}

void PSN1AGet(PSN1 * ps, long len, char * ch)
{
    memcpy(ch,ps->p+ps->len,len);
    ps->len+=len;
}

void PSN1APutlong(PSN1 * ps, long x)
{
    PSN1Grow(ps,8);
    memcpy(ps->p+ps->len,&x,8);
    ps->len+=8;
}

long PSN1AGetlong(PSN1 * ps)
{
    long x;
    memcpy(&x,ps->p+ps->len,8);
    ps->len+=8;
    return x;
}

void PSN1APutchar(PSN1 * ps, char x)
{
    PSN1Grow(ps,1);
    memcpy(ps->p+ps->len,&x,1);
    ps->len+=1;
}

char PSN1AGetchar(PSN1 * ps)
{
    char x;
    memcpy(&x,ps->p+ps->len,1);
    ps->len+=1;
    return x;
}

void PSN1BPut(PSN1 * ps, char * ch)
{
    PSN1APut(ps,ch,strlen(ch)+1);
}

long PSN1BLen(PSN1 * ps)
{
    return strlen(ps->p+ps->len);
}

void PSN1BSkip(PSN1 * ps)
{
    ps->len+=strlen(ps->p+ps->len)+1;
}

void PSN1BGet(PSN1 * ps, char * ch)
{
    strcpy(ch,ps->p+ps->len);
}

/* end of psn1.c  */
