/*
         funs.h  -   Level III functions prototypes
         ======      R. A. Parker 8.10.2015
*/

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
 * Individual transpose a file in to produce a file out
 * slient is TRUE if a silent close is to be done, otherwise FALSE
 */
extern void ftra(const char *in, int sin, const char *out, int sout);

/*
 * Individual multiply, C = A * B
 * slient is TRUE if a silent close is to be done, otherwise FALSE
 */
extern void fmu(const char *out, const char *ina, const char *inb, int silent);

/*
 * Individual multiply and add, C += A * B
 * slient is TRUE if a silent close is to be done, otherwise FALSE
 */
extern void fmad1(const char *ina, const char *inb, const char *inc, const char *out, const char *tmp, int silent);

/*
 * Add, C = A + B
 * slient is TRUE if a silent close is to be done, otherwise FALSE
 */
extern void fadd(const char *out, const char *ina, const char *inb, int silent);

/*
 * zcx, split input using bitstring into selected and non selected
 */
extern void fcx(const char *bits, const char *in, const char *out_sel, const char *out_nonsel, int silent);

/*
 * zpc, combine pivots into unified pivot plus riffle
 */
extern void fpc(const char *bs_in1, const char *bs_in2, const char *bs_out, const char *riffle, int silent);

/*
 * zrr, row riffle
 */
extern void frr(const char *bs_in, const char *row_in1, const char *row_in2, const char *row_out, int silent);

/*
 * zpef, produce echelon form
 */
extern void fpef(const char *in, const char *outbs, const char *outrm, int silent);

/*
 * zmkn, make null-space from bitstring and transposed remnant
 */
extern uint64_t fmkn(const char *bs, int sbs, const char *rm, int srm, const char *out, int sout);

/* end of funs.h  */
