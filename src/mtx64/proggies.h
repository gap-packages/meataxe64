/*
      proggies.h     All the proggies headers
      ==========     R. A. Parker    6.9.2015
*/

typedef struct
{
    char * fn;               // A
    int silent;              // A
    uint64_t fdef;           // B
    uint64_t nor;            // B
    uint64_t noc;            // B
    MOJ fmoj;                // C
    uint64_t r;              // D
    uint64_t c;              // D
    uint64_t * rnor;         // D
    uint64_t * cnoc;         // D
    MOJ ** m;                // E
    MOJ fl;                  // F

} M3S;
typedef M3S * M3;

 M3  M3Cons(const char * fn, int sil); // construct and set filename
void M3Peek(M3 a);            // get fdef nor noc by peeking
MOJ  M3FieldMOJ (uint64_t fdef);  // make field moj
void M3EvenChop (M3 a, uint64_t divr, uint64_t divc);  // even sized chunks
void M3MOJs  (M3 a);          // allocate MOJs
void M3MOJArray (M3 a);       // allocate MOJ array but not MOJs
void M3Read (M3 a);           // kick off read thread
void M3Write(M3 a);           // kick off write thread
void M3Dest(M3 a);            // destroy M3 struct

#define MULPROG 1
#define MADPROG 2
// 3 spare - used to be transpose
#define CEXPROG 4
#define REXPROG 5
#define RRFPROG 6
#define PVCPROG 7
#define ADDPROG 8
#define ECHPROG 9
#define MCPPROG 10
#define FMVPROG 11
#define CRZPROG 12
#define ADIPROG 13
#define MKRPROG 14
#define PC0PROG 15

#define MUL(pri, fmoj, a, b, c) \
  TFSubmit(pri, MULPROG, fmoj, a, b, -2l, &c, -1l)
#define MAD(pri, fmoj, a, b, c, s)               \
  TFSubmit(pri, MADPROG, fmoj, a, b, c, -2l, &s, -1l)
#define CEX(pri, fmoj, a, b, c, d) \
  TFSubmit(pri, CEXPROG, fmoj, a, b, -2l, &c, -2l, &d, -1l)
#define REX(pri, fmoj, a, b, c, d) \
  TFSubmit(pri, REXPROG, fmoj, a, b, -2l, &c, -2l, &d, -1l)
#define RRF(pri, fmoj, a, b, c, d) \
  TFSubmit(pri, RRFPROG, fmoj, a, b, c, -2l, &d, -1l)
#define PVC(pri, a, b, c, d) \
  TFSubmit(pri, PVCPROG, a, b, -2l, &c, -2l, &d, -1l)
#define ADD(pri, fmoj, a, b, c)               \
  TFSubmit(pri, ADDPROG, fmoj, a, b, -2l, &c, -1l)
#define ECH(pri, fmoj, a, b, c, d, e, f) \
  TFSubmit(pri, ECHPROG, fmoj, a, -2l, &b, -2l, &c, -2l, &d, -2l, &e, -2l, &f, -1l)
#define MCP(pri, fmoj, a, b) \
  TFSubmit(pri, MCPPROG, fmoj, a, -2l, &b, -1l)
#define FMV(pri, a, flow, b) \
  TFSubmit(pri, FMVPROG, a, flow, -2l, &b, -1l)
#define CRZ(pri, fmoj, a, b, c) \
  TFSubmit(pri, CRZPROG, fmoj, a, b, -2l, &c, -1l)
#define ADI(pri, fmoj, a, b, c) \
  TFSubmit(pri, ADIPROG, fmoj, a, b, -2l, &c, -1l)
#define MKR(pri, a, b, c) \
  TFSubmit(pri, MKRPROG, a, b, -2l, &c, -1l)
#define PC0(pri, a, b, c) \
  TFSubmit(pri, PC0PROG, a, -2l, &b, -2l, &c, -1l)


/******  end of proggies.h    ******/
