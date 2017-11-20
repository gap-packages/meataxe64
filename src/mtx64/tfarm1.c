/*
         tfarm1.c   -   Thread Farm - multi-thread scheduler           
         ========       R. A. Parker     9.10.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "tfarm.h"
#include "tuning.h"
#define SCALE MAXCHOP*MAXCHOP*MAXCHOP

/* Global Variables */

/* closejobs is incremented by submit and decremented when a */
/* job completes, allowing TfWaitEnd to know when all the    */
/* submitted jobs have completed. TFUpJobs and TFDownJobs    */
/* allows proggies (e.g. M3Read, M3Write) to manipulate it.  */

int closejobs;



/* stopfree is incremented by TFStopMOJFree and decremented  */
/* by TFStartMOJFree, and if non-zero, TFRelease blocks      */
/* This allows a string of submissions to proceed without    */
/* a release discarding a MOJ when an unsubmitted job will   */
/* read it                                                   */
 
int stopfree;

void TFStopMOJFree(void)
{
    TfAdd(&stopfree,1);
}

extern void   TFStartMOJFree(void)
{
    TfAdd(&stopfree,-1);
}

/* nothreads is the (constant) number of started threads     */
/* It is set by TFInit and used by TFClose                   */

int nothreads;

/* There is a single "global" mutex called "dodgy" that can  */
/* be used to prevent data races etc., and this is used      */
/* as a last resort, or where performance is not important   */

pthread_mutex_t dodgy=PTHREAD_MUTEX_INITIALIZER;

void Lock(void)
{
    pthread_mutex_lock(&dodgy);
}

void Unlock(void)
{
    pthread_mutex_unlock(&dodgy);
}

/* Similarly there is a single "global" condition variable    */
/* that can be used (called holding with "dodgy") to wait for */
/* things to happen.  Again a last resort, or where           */
/* performance is not a major issue.                          */

pthread_cond_t maincond=PTHREAD_COND_INITIALIZER;

void LcMainPause(void)
{
    pthread_cond_wait(&maincond,&dodgy);
}

void LcMainKick(void)
{
    pthread_cond_broadcast(&maincond);
}

void TFUpJobs(void)
{
    TfAdd(&closejobs,1);
}

void TFDownJobs(void)
{
    int i;
    i=TfAdd(&closejobs,-1);
    if(i==0)
    {
        Lock();
        LcMainKick();
        Unlock();
    }
}

#define TFMSIZE 10
uint64_t * TFM;
#define TFMFRE 0
#define TFMJOB 1
#define TFMRDL 2

#define FRESIZE 2000

/* ========== thread data ================= */

typedef struct
{
    int threadno;
    jobstruct * JOB;
}  parmstruct;
int * tfthread;
parmstruct * tfparms;

int firstfreethread;

int runjobs;

void TFWaitEnd(void)
{
    Lock();
    while(1)  // wait until all submitted jobs completed
    {
        if(closejobs==0) break;
        LcMainPause();
    }
    Unlock();
    while(1)  // wait until all threads got lock (at least) to suspend
    {
        if( (runjobs+nothreads)==0) break;
        TfPause(3000);
    }
}

//  This should be inlined once it is under regression

void * AlignTalloc(size_t len)
{
    unsigned long *x,y;
    x=malloc(len+128);
    if(x==NULL) return NULL;
    y=1;
    x++;
    while( (((long int) x)&0x3f)!=0) 
    {
        x++;
        y++;
    }
    *(x-1)=y;
    return (void *) x;
}

void AlignTree(void * z)
{
    unsigned long y,*x;
    x=(unsigned long *) z;
    y=(unsigned long) *(x-1);
    x -= y;
    free(x);
}


/* ========== moj data ===== */


MOJ tfmoj;

int nfmoj;
int nmojes;

/* ========== job data and methods ======= */


jobstruct * NewJob(void) 
{
    return (jobstruct *) TfLinkOut(TFM+TFMJOB);
}

void FreeJob(jobstruct * JOB)
{
    TfLinkIn(TFM+TFMJOB,(uint64_t *) JOB);
}

void JobPop(jobstruct * JOB, int proggy, int priority)
{
    JOB->proggyno = proggy;
    JOB->priority = priority;
    JOB->ref = 1;
    JOB->nparm=0;
}

/* ========== rdl data and methods ======= */

rdlstruct * LcNewRdl(void) 
{
     return (rdlstruct *) TfLinkOut(TFM+TFMRDL);
}

void LcFreeRdl(rdlstruct * RDL)
{
    TfLinkIn(TFM+TFMRDL,(uint64_t *) RDL);
}

void TfUpJobRef(jobstruct * JOB)
{
    TfAdd(&(JOB->ref),1);   // maybe lock add someday
}

void UpMojRef(MOJ mj)
{
    TfAdd(&(mj->refc),1);
}

void LCBindRdl(jobstruct * job, MOJ moj)
{
    rdlstruct * RDL;
    int x;
    UpMojRef(moj);
    RDL=LcNewRdl();
    x=TfLinkIn((uint64_t *)(&(moj->RDL)),(uint64_t *)RDL);
    if(x==2) return;
    RDL->JOB=job;
    TfUpJobRef(job);
}


void LcBindRdl(jobstruct * JOB, int moj)
{
    rdlstruct * RDL;
    MOJ mj;
    int x;
    mj=tfmoj+moj;
    UpMojRef(mj);
    RDL=LcNewRdl();
    x=TfLinkIn((uint64_t *)(&(mj->RDL)),(uint64_t *)RDL);
    if(x==2) return;
    RDL->JOB=JOB;
    TfUpJobRef(JOB);
}

/* ==========  jobs ready to run  ======= */

jobstruct ** RUNJOB;
long  jobsready;

void LcStartJob(jobstruct * JOB, int thread);

int TfUnQThread(jobstruct * JOBNO)
{
    int thread;
    if(firstfreethread!=-1)
    {
        thread=firstfreethread;
        firstfreethread=tfthread[thread];
        LcStartJob(JOBNO,thread);
        Unlock();
        return thread;
    }
    fprintf(stderr,"different sort of failure\n");
    exit(17);
}

void TfQJob(jobstruct * JOBNO)
{
    int k,kp;
    jobstruct * t;
    RUNJOB[jobsready++]=JOBNO;
// swim
    k=jobsready-1;
    while(k>0)
    {
        kp=(k-1)/2;
        if(RUNJOB[k]->priority
         >=RUNJOB[kp]->priority) break;
        t=RUNJOB[k];
        RUNJOB[k]=RUNJOB[kp];
        RUNJOB[kp]=t;
        k=kp;
    }
}

void TfPutJob(jobstruct * JOBNO)
{

    int i;
    Lock();
    i=TfAdd(&runjobs,1);
    if(i<=0)
    {
        TfUnQThread(JOBNO);
        return;
    }
    if(firstfreethread!=-1) fprintf(stderr,"Yet another sort\n");
    TfQJob(JOBNO);
    Unlock();
    return;
}

// Rename as TfUnQJob once locking sorted

jobstruct * LcNextRun(void) 
{
    int k,kc1,kc2;
    jobstruct * rj, *t;
    rj=RUNJOB[0];
    RUNJOB[0]=RUNJOB[--jobsready];
// sink
    k=0;
    while(1)
    {
        kc1=2*k+1;
        kc2=2*k+2;
        if(kc1>=jobsready) break;
        if(kc2<jobsready)
        {
            if(RUNJOB[kc1]->priority
             >=RUNJOB[kc2]->priority)  kc1=kc2;
        }
        if(RUNJOB[k  ]->priority
         <=RUNJOB[kc1]->priority)  break;
        t=RUNJOB[k];
        RUNJOB[k]=RUNJOB[kc1];
        RUNJOB[kc1]=t;
        k=kc1;
    }
    return rj;
}

void TfJobRefDown(jobstruct * JOB) 
{
    int x;
    x=TfAdd(&(JOB->ref),-1);
    if(x!=0) return;
    TfPutJob(JOB);
}

void LcFreeThread(int thread) 
{
    tfthread[thread]=firstfreethread;
    firstfreethread=thread;
}

/* ====****====**** pthread stuff *****====*****/

/* the thread structures themselves      */
pthread_t * mythread;

/* condition variable that the workers   */
/* wait on when they are idle            */
pthread_cond_t * wakeworker;

void LcWorkerPause(int thread)
{
    pthread_cond_wait(wakeworker+thread,&dodgy);
}

void LcWorkerKick(int thread)
{
    pthread_cond_signal(wakeworker+thread);
}

void LcStartJob(jobstruct * JOB, int thread)
{
    tfparms[thread].JOB=JOB;
    LcWorkerKick(thread); 
}

/* ====== Now the code for the routines  =======*/

void tfdo(int proggyno, MOJ * p);

jobstruct * LcQThread(parmstruct * pp)
{
    while(1)
    {
        pp->JOB=NULL;
        while(pp->JOB==NULL)
        {
            LcFreeThread(pp->threadno);
            LcWorkerPause(pp->threadno);
        }
        return pp->JOB;
    } 
}

jobstruct * GetJob(parmstruct * pp)  // may be 2
{
    int i;
    jobstruct * JOB;
    Lock();
    i=TfAdd(&runjobs,-1);
    if(i>=0)
    {
        JOB=LcNextRun();
        Unlock();
        return JOB;
    }
    JOB=LcQThread(pp);
    Unlock();
    return JOB;
}

/*  The worker thread  */
void * worker(void * p)
{
    int nparm;
    jobstruct * JOB;
    parmstruct * pp;
    int proggyno;
    MOJ parms[10];
    int i;
    pp = (parmstruct *) p;

    while(1)
    {
        JOB=GetJob(pp);
        if(((uint64_t)JOB)==2) break;
        proggyno=JOB->proggyno;
        nparm=JOB->nparm;
        for(i=0;i<nparm;i++) parms[i]=JOB->parm[i];
        FreeJob(JOB);
        tfdo(proggyno,parms);
        for(i=0;i<nparm;i++) TFRelease(parms[i]);
        TFDownJobs();
    }
    return NULL;
}

void * TFPointer(MOJ mj)
{
    return mj->mem;   // read only at this point
}

void * TFAllocate(MOJ mj, size_t bytes)
{
    void * ptr;
    ptr = AlignTalloc(bytes);
    if(ptr==NULL)
    {
        printf("memory failure for moj of %ld\n",bytes);
        exit(1);
    }
    mj->mem=ptr;   // private at this point
    return ptr;
}

void TFSetPtr(MOJ mj, void * ptr)
{
    mj->mem=ptr;   // private at this point
}

void TFRelease(MOJ mj)
{
    int i;

    while(1)
    {
        if(stopfree==0) break;    // atomic fetch
        TfPause(100);
    }
    i=TfAdd(&(mj->refc),-1);
    if(i>0) return;
    if(i<0)
    {
        printf("Refcount went negative!\n");
        return;
    }
    if(mj->refc!=0) return;
    if(mj->mem!=NULL) AlignTree(mj->mem);
    mj->mem=NULL;
    mj->RDL=NULL;   // it is dead now  ?? delete this line ?
//  ought to return the MOJ to the list
}

MOJ TFNewMOJ(void)
{
    MOJ newone;
    Lock();
    if((nfmoj+1)>=nmojes)
    {
        printf("Run out of MOJs\n");
        exit(1);
    }
    newone = tfmoj+nfmoj;
    nfmoj++;
    Unlock();
    newone->mem=NULL;
    newone->RDL=NULL;
    newone->refc=0;
    return newone;
}

void TFStable (MOJ mj)
{
    rdlstruct * RDL;
    Lock();
    while(1)
    {
        RDL=(rdlstruct *) TfLinkClose((uint64_t *)&(mj->RDL));
        if(RDL==NULL) break;
        Unlock();
        TfJobRefDown(RDL->JOB);
        Lock();
        LcFreeRdl(RDL);
    }
    LcMainKick();        // it may be waiting for this moj
    Unlock();
    return;
}


void TFQuickReady(MOJ moj)
{
    TfLinkClose((uint64_t *)(&(moj->RDL)));
}

void TFGetReadRef(MOJ moj)
{
    UpMojRef(moj);
}

// now the calls that only the upper level can use

void TFClose(void)
{
    uint64_t * FRE;
    uint64_t nfree,i;
    uint64_t fakejob;
    int thread;
    fakejob=2;                  // instruction to shut down thread
    for(i=0;i<nothreads;i++)
    {
        Lock();
        thread=TfUnQThread((jobstruct *)fakejob);
        pthread_join(*(mythread+thread),NULL);
    }
    FRE=(uint64_t *)(*TFM);
    nfree=*FRE;
    for(i=1;i<=nfree;i++) AlignTree((void *)FRE[i]);
    free(FRE);
    free(TFM);
}

void   TFInit(int threads)
{
    int i;
    int jobx,rdlx;
    uint64_t * FRE;
    jobstruct * tfjob;
    rdlstruct * tfrdl;
    MOJ mj;
    jobx=SCALE*8;
    nmojes=jobx*2;
    closejobs=0;
    TFM=malloc(TFMSIZE*sizeof(uint64_t));
    FRE=malloc(FRESIZE*sizeof(uint64_t));
    *(TFM+TFMFRE)=(uint64_t)FRE;
    *FRE=0;      // none yet to free
// following is temporary fix for zpe needs.
// redesign it better for V2
    rdlx=3*nmojes;
    tfjob=AlignTalloc(jobx*sizeof(jobstruct));
    TfAppend(FRE,(uint64_t)tfjob);
    *(TFM+TFMJOB)=0;     // freechain of jobs empty
    for(i=0;i<jobx;i++) TfLinkIn(TFM+TFMJOB,(uint64_t *) (tfjob+i));
    tfrdl=AlignTalloc(rdlx*sizeof(rdlstruct));
    TfAppend(FRE,(uint64_t)tfrdl);
    *(TFM+TFMRDL)=0;     // freechain of jobs empty
    for(i=0;i<rdlx;i++) TfLinkIn(TFM+TFMRDL,(uint64_t *) (tfrdl+i));
    RUNJOB=AlignTalloc(jobx*sizeof(jobstruct *));
    TfAppend(FRE,(uint64_t)RUNJOB);
    nfmoj=0;
    runjobs=0;
    stopfree=0;

/* Initialize the moj data  */
    tfmoj=AlignTalloc(nmojes*sizeof(mojstruct));
    TfAppend(FRE,(uint64_t)tfmoj);
    for(i=0;i<nmojes;i++)
    {
        mj=tfmoj+i;
        mj->mem=NULL;
        mj->refc=0;     // This is private, so OK.
        mj->RDL=NULL;
    }
/* Initialize the thread data  */
    firstfreethread=-1;  // no free threads yet
    tfthread=AlignTalloc(threads*sizeof(int));
    TfAppend(FRE,(uint64_t)tfthread);
    tfparms =AlignTalloc(threads*sizeof(parmstruct));
    TfAppend(FRE,(uint64_t)tfparms);
    for(i=0;i<threads;i++) tfparms[i].JOB=(jobstruct *)3;  // so close works
    nothreads=threads;
    mythread=AlignTalloc(threads*sizeof(pthread_t));
    TfAppend(FRE,(uint64_t)mythread);
    wakeworker=AlignTalloc(threads*sizeof(pthread_cond_t));
    TfAppend(FRE,(uint64_t)wakeworker);

/* initialize the condition variables for the workers */
    for(i=0;i<threads;i++) pthread_cond_init(wakeworker+i,NULL);
    jobsready=0;    // no jobs ready to run
/* get the lock so that nothing does   */
/* any damage while we are starting up         */
    Lock();
/* start all the worker threads                */
    for(i=0;i<nothreads;i++)
    {
        tfparms[i].threadno=i;
        pthread_create(mythread+i,NULL, worker, tfparms+i);
    }
/* all is ready - now let's go                 */
    Unlock();
    return;
}

void TFWait(MOJ moj)
{
    Lock();
    while(1)
    {
        if(((uint64_t)(moj->RDL))==2)
        {
            Unlock();
            return;
        }
        LcMainPause();    
    }
}

void TFSubmit(int priority, int proggyno, ...)
{
    va_list va;
    MOJ moj;
    jobstruct * JOB;
    MOJ * mojadd;
    va_start(va,proggyno);
    JOB=NewJob();
    JobPop(JOB, proggyno, priority);
    while(1)
    {
        moj=va_arg(va,MOJ);
        if((long)moj==-1) break;
        if((long)moj==-2)
        {
            mojadd=va_arg(va,MOJ*);
            moj=TFNewMOJ();
            *mojadd=moj;              // writing
            UpMojRef(moj);    // for the proggy
        }
        else
        {
            Lock();
            LCBindRdl(JOB,moj);
            Unlock();
        }
        JOB->parm[JOB->nparm++]=moj;    // new parameters
    }
    va_end(va);
    TFUpJobs();
    TfJobRefDown(JOB);
}

/* end of tfarm.c  */
