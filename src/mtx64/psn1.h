/*
         psn1.h  -   Parker's first syntax notation Header
         ======      R. A. Parker  7.6.2015
*/

typedef struct
{
    char * p;
    long len;
    long alc;
} PSN1;

// routines for use in the proggies

extern void PSN1Init(PSN1 * ps);
extern char * PSN1Point(PSN1 * ps);
extern void PSN1Grow(PSN1 * ps, long ln);
extern void PSN1APut(PSN1 * ps, char * ch, long len);
extern void PSN1AZero(PSN1 * ps, long len);
extern void PSN1ASkip(PSN1 * ps, long len);
extern void PSN1AGet(PSN1 * ps, long len, char * ch); 
extern void PSN1APutlong(PSN1 * ps, long x);
extern long PSN1AGetlong(PSN1 * ps);
extern void PSN1APutchar(PSN1 * ps, char x);
extern char PSN1AGetchar(PSN1 * ps);
extern void PSN1BPut(PSN1 * ps, char * ch);
extern long PSN1BLen(PSN1 * ps);
extern void PSN1BSkip(PSN1 * ps);
extern void PSN1BGet(PSN1 * ps, char * ch);

/* end of psn1.h  */
