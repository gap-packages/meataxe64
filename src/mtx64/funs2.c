// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// funs2.c     Some simple composite functions

// Contents
// fMultiplyAdd
// fNullSpace
// fInvert

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

extern void fInvert(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2)
{
    FIELD *f;
    uint64_t hdr[5];
    uint64_t rank,nor,i,k;
    uint64_t * inv;
    EFIL *e1,*e2;
    char sta[500];
    char stb[500];
    strcpy(sta,tmp);
    strcat(sta,"a");
    strcpy(stb,tmp);
    strcat(stb,"b");
    EPeek(m1,hdr);
    if(hdr[0]==1)  // flat matrix
    {
        if(hdr[2]!=hdr[3])
        {
            printf("Invert - matrix not square\n");
            exit(15);
        }

        rank=fFullEchelize(sta,m1,s1,"NULL",1,"NULL",1,stb,1,
             "NULL",1,"NULL",1);
        if(rank!=hdr[2])
        {
            printf("Invert - Matrix is singular\n");
            remove(stb);
            exit(13);
        }

        f = malloc(FIELDLEN);
        FieldASet(hdr[1],f);
        fScalarMul(stb,1,m2,s2,f->charc-1);
        remove(stb);
        free(f);
        return;
    }
    if(hdr[0]==3)   // invert for permutations
    {
        e1 = ERHdr(m1,hdr);
        nor=hdr[2];
// missing checks that nor=noc and invertible.
        inv=malloc(8*nor);
        for(i=0;i<nor;i++)
        {
            ERData(e1,8,(uint8_t *) &k);
            inv[k]=i;
        }
        ERClose(e1);

        e2 = EWHdr(m2,hdr);
        EWData(e2,8*nor,(uint8_t *)inv);
        EWClose(e2);
        free(inv);
        return;
    }
    printf("Cannot invert objects of type %ld\n",hdr[0]);
}

/* end of funs2.c  */
