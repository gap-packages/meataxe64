/*
         tfarm2.c   -   Thread Farm - service routines           
         ========       R. A. Parker     9.8.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "tfarm.h"

typedef struct
{
    uint64_t upres;
    uint64_t downres;
    uint64_t res;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}  FlowMOJ;

MOJ TFFlowCons(uint64_t res, uint64_t upres, uint64_t downres)
{
    MOJ moj;
    FlowMOJ * pt;
    moj=TFNewMOJ();
    TFGetReadRef(moj);
    pt=(FlowMOJ *) TFAllocate(moj,sizeof(FlowMOJ));
    pthread_mutex_init(&(pt->lock), NULL);
    pthread_cond_init(&(pt->cond),NULL);
    pt->res=res;
    pt->upres=upres;
    pt->downres=downres;
    TFQuickReady(moj);
    return moj;
}

void TFFlowUp(MOJ moj)
{
    FlowMOJ * pt;
    pt=(FlowMOJ *) TFPointer(moj);
    pthread_mutex_lock(&(pt->lock));
    pt->res+=pt->upres;
    pthread_cond_broadcast(&(pt->cond));
    pthread_mutex_unlock(&(pt->lock));
    return;
}

void TFFlowWait(MOJ moj)
{
    FlowMOJ * pt;
    pt=(FlowMOJ *) TFPointer(moj);
    pthread_mutex_lock(&(pt->lock));
    while(pt->res<pt->downres)
        pthread_cond_wait(&(pt->cond),&(pt->lock));
    pt->res-=pt->downres;
    pthread_mutex_unlock(&(pt->lock));
    return;
}

void TFFlowDest(MOJ moj)
{
//    FlowMOJ * pt;    need to destroy the pthread stuff
//    pt=(FlowMOJ *) TFPointer(moj);
    TFRelease(moj);
    return;
}

void TFSetElt(MOJ moj,uint64_t i, uint64_t j, MOJ entry)
{
    uint64_t * pt;
    uint64_t noc;
    pt=TFPointer(moj);
    noc=pt[1];
    pt[2+i*noc+j]=(uint64_t) entry;
}

MOJ TFMkMOJAry(uint64_t nor, uint64_t noc,int flag)
{
    MOJ moj,nmoj;
    MOJ * pt;
    uint64_t i,j;
    moj=TFNewMOJ();
    TFGetReadRef(moj);
    pt=TFAllocate(moj,(8*(2+nor*noc)));
    pt[0]=(MOJ)nor;
    pt[1]=(MOJ)noc;
    if(flag==0) return moj;
    for(i=0;i<nor;i++)
        for(j=0;j<noc;j++)
        {
            nmoj=TFNewMOJ();
            TFSetElt(moj,i,j,nmoj);
            TFGetReadRef(nmoj);
        }
    return moj;
}

MOJ TFAryElt(MOJ moj,uint64_t i, uint64_t j)
{
    uint64_t * pt;
    uint64_t noc;
    pt=TFPointer(moj);
    noc=pt[1];
    return (MOJ) pt[2+i*noc+j];
}

/* end of tfarm2.c  */
