// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// functions header file

// First the large functions each in a separate own source file


extern void fTranspose(const char * tmp, const char *in, int sin,
                 const char *out, int sout);  //ftra.c
extern void fMultiply(const char * tmp, const char *m1, int s1, 
                 const char *m2, int s2, const char *m3, int s3);  //fmul.c
extern uint64_t fProduceNREF(const char * tmp, const char *m1, int s1,
                     const char *b2, int s2,const char *m3, int s3);  //fpef.c
extern uint64_t fFullEchelize(const char *temp, 
                const char *m1, int s1, const char *b2, int s2,
                const char *b3, int s3, const char *m4, int s4,
                const char *m5, int s5, const char *m6, int s6);  // fech.c
// funs1 (Gaussian-related) functions
extern void fColumnExtract(const char *bs, int sbs, const char *in, int sin, 
          const char *sel, int ssel, const char * nsel, int snsel);
extern void fRowRiffle(const char *bs, int sbs, const char * ins, int sins,
                 const char * inn, int sinn, const char * out, int sout);
extern void fPivotCombine(const char *b1, int sb1, const char *b2, int sb2,
                          const char *bc, int sbc, const char *br, int sbr);
extern uint64_t fColumnRiffleIdentity(const char *bs, int sbs,
              const char *rm, int srm, const char *out, int sout);

// funs2 (small, composite) functions
extern void fMultiplyAdd(const char * tmp, const char *m1, int s1,
           const char *m2, int s2, const char *m3, int s3,
           const char * m4, int s4);
extern uint64_t fNullSpace(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2);
extern void fInvert(const char *tmp, const char *m1, int s1,
                           const char *m2, int s2);

// funs3 field-changing routines
extern void fFrobenius(const char *m1, int s1, const char *m2, int s2);

extern int  fFieldContract(const char *m1, int s1, uint64_t newfield,
                           const char *m2, int s2);
extern void fFieldExtend(const char *m1, int s1, uint64_t newfield,
                         const char *m2, int s2);

// funs4 pieces of tensor powers
extern void fTensor(const char *m1, int s1, 
                 const char *m2, int s2, const char *m3, int s3);
extern void fExteriorSquare(const char *m1, int s1,
                           const char *m2, int s2);
extern void fExteriorCube(const char *m1, int s1, 
                          const char *m2, int s2);
extern void fSymmetricSquare(const char *m1, int s1,
                           const char *m2, int s2);

// funs5 miscellanous small functions
extern void fAdd(const char *fn1, int s1, const char *fn2, int s2,
                 const char *fn3, int s3);
extern void fScalarMul(const char *m1, int s1,
                       const char *m2, int s2, FELT sc);
extern FELT fTrace(const char *m1, int s1);

void fProjectiveVector(const char *m1, int s1, uint64_t pvec,
                       const char *m2, int s2);
void fMulMatrixMap(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3);
void fMulMaps(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3);
void fRandomMatrix(const char *m1, int s1, uint64_t fdef, 
                       uint64_t nor, uint64_t noc);

/* end of funs.h  */
