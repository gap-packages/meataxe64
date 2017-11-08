/*
         tfarm.h  -   Thread Farm Header
         =======      R. A. Parker  4.5.2016
*/

/* tfarm0 assembler thread-safe routines */

int TfAdd(int * x, int y);
long TfGetUni(long * uni);
void TfPutUni(long * uni, long val);
uint64_t * TfLinkOut(uint64_t * chain);
uint64_t * TfLinkClose(uint64_t * chain);
int TfLinkIn(uint64_t * chain, uint64_t * ours);
void TfAppend(uint64_t * list, uint64_t new);
void TfPause(long wait);

#define MAXPARAMS 10

typedef struct rdlinc
{
    struct rdlinc *NEXRDL;
    struct jobinc *JOB;  // Job to be notified moj is stable
} rdlstruct;

typedef struct
{
    void * mem;      // Memory pointer (NULL if none allocated) 
    rdlstruct * RDL; // closable chain of RDLs for this moj
    int  refc;       // Read reference counter
    int  junk;       // to make sizeof(mojstruct) divisible by 8
} mojstruct;

typedef mojstruct * MOJ;

typedef struct jobinc
{
    mojstruct * parm[MAXPARAMS];    // new parameters
    int  proggyno;   // proggy to be executed
    int  priority;   // priority (lower runs earlier)
    int  ref;        // how many things it is waiting for 
    int nparm;       // number of parameters
} jobstruct;

/* temporary fix!  */
extern MOJ tfmoj;

// Routines for use in the proggies

extern void * TFPointer(MOJ moj);
extern void * TFAllocate(MOJ moj, size_t bytes);
extern void   TFSetPtr(MOJ moj, void * ptr);
extern void   TFRelease(MOJ moj);
extern void   TFStable(MOJ moj);
extern void   TFWait(MOJ moj);
extern void   TFGetReadRef(MOJ moj);
extern void   TFSubmit(int priority, int proggyno, ...);
extern MOJ    TFNewMOJ(void);
extern void   TFQuickReady(MOJ moj);
extern void   TFInit(int threads);
extern void   TFWaitEnd(void);
extern void   TFClose(void);
extern void   TFUpJobs(void);
extern void   TFDownJobs(void);
extern void   TFStopMOJFree(void);
extern void   TFStartMOJFree(void);

// service routines from tfarm2

extern  MOJ TFFlowCons(uint64_t res, uint64_t upres, uint64_t downres);
extern void TFFlowUp(MOJ moj);
extern void TFFlowWait(MOJ moj);
extern void TFFlowDest(MOJ moj);

/* end of tfarm.h  */
