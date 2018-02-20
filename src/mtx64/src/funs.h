// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// functions header file

// First the large functions each in a separate own source file


extern void fTranspose(const char * tmp, const char *in, int sin,
                 const char *out, int sout);
extern void fMultiply(const char * tmp, const char *m1, int s1, 
                 const char *m2, int s2, const char *m3, int s3);
extern uint64_t fProduceNREF(const char * tmp, const char *m1, int s1,
                     const char *b2, int s2,const char *m3, int s3);

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
extern FELT fTrace(const char *m1, int s1);

void fProjectiveVector(const char *m1, int s1, uint64_t pvec,
                       const char *m2, int s2);
void fMulMatrixMap(const char *m1, int s1, const char *x2, int s2,
                       const char *m3, int s3);

// The remainder is to be considered, and either removed
// or put into the first part

typedef struct
{
    FIELD * f;
} TLS;

extern uint64_t fech(const char *m1, int s1, const char *b2, int s2,
                const char *b3, int s3, const char *m4, int s4,
                const char *m5, int s5, const char *m6, int s6);
// Scalar multiply
extern void fsmu(TLS * tls, const char *fn1, int s1,
                 const char *fn2, int s2, FELT sc);

/*
 * Chop on disc for operations such as ftr, fmu
 * fname is the input file
 * tmp is a root for forming temporary names
 * rchops and cchops show respectively how may rows and columns of files we want
 * returns a 2 dimensional array of names
 * of the temporary files produced by the chopping operation
 * To get the (i,j) filename you need (*tmpnames)[i][j]
 * It is an assumption of this routine that neither rchops nor cchops is zero
 * and that the input file has at least rchops rows and cchops columns
 * The program will exit non-zero if these conditions aren't met
 * This function will always do silent close operations
 */
extern char ***fchp(const char *fname, const char *tmp, unsigned int rchops, unsigned int cchops);

/*
 * Reassemble from disc after operations such as ftr, fmu
 * fname is the output file
 * tmpnames is the 2 dimensional array of temporary names
 * rchops and cchops show respectively how may rows and columns of files we want
 * To get the (i,j) filename you need (*tmpnames)[i][j]
 * It is an assumption of this routine that neither rchops nor cchops is zero
 * The program will exit non-zero if these conditions aren't met
 * This function will always do logging close operations
 */
extern void fasp(const char *fname, char ***tmpnames, unsigned int rchops, unsigned int cchops);

/*
 * zmkn, make null-space from bitstring and transposed remnant
 */


/* end of funs.h  */
