/*
              funs2.c     Some simple composite functions
              =======     R. A. Parker   27.9.2017
*/

// Contents
// fMultiplyAdd
// fNullSpace

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tuning.h"
#include "field.h"
#include "pcrit.h"
#include "funs.h"
#include "io.h"
#include "bitstring.h"

// fMultiplyAdd

void fMultiplyAdd(const char * tmp, const char *m1, int s1,
           const char *m2, int s2, const char *m3, int s3,
           const char * m4, int s4)
{
    char sta[500];
    char stb[500];
    strcpy(sta,tmp);
    strcat(sta,"a");
    strcpy(stb,tmp);
    strcat(stb,"b");
    fMultiply(sta,m1,s1,m2,s2,stb,1);
    fAdd(stb,1,m3,s3,m4,s4);
    remove(stb);
}

// fNullSpace

uint64_t fNullSpace(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2)
{
    uint64_t nullity;
    char sta[500];
    char stb[500];
    char stc[500];
    char std[500];
    char ste[500];
    strcpy(sta,tmp);
    strcat(sta,"a");
    strcpy(stb,tmp);
    strcat(stb,"b");
    strcpy(stc,tmp);
    strcat(stc,"c");
    strcpy(std,tmp);
    strcat(std,"d");
    strcpy(ste,tmp);
    strcat(ste,"e");
    fTranspose(sta,m1,s1,stb,1);
    fProduceNREF(sta,stb,1,stc,1,std,1);
    fTranspose(sta,std,1,ste,1);
    nullity=fColumnRiffleIdentity(stc,1,ste,1,m2,s2);
    remove(stb);
    remove(stc);
    remove(std);
    remove(ste);
    return nullity;
}

/* end of funs2.c  */
