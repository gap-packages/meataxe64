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

/* end of tfarm2.c  */
