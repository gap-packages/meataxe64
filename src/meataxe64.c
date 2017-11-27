/*
 * meataxe64: meataxe64
 *
 * This file contains interfaces to the "slab" level of meataxe64. This sits
 * below the thread farm and provides single-threaded matrix arithmetic. "Full"
 * meataxe64 computations use the thread farm to make multiple "slab" calls in
 * parallel
 * on multi-core systems.
 *
 * The slab level functions are designed for matrix dimensions in the range from
 * a
 * few hundred to a few tens of thousands, although there are no hard limits.
 *
 */

#include "src/compiled.h" /* GAP headers */
#include "src/vec8bit.h" /* GAP headers -- we need the internals of these objects */
#include "src/vecgf2.h" /* GAP headers    for efficient vector conversion */


// TODO Split this file up along the lines of the split between the different
// headers
// in meataxe64

#include "mtx64/field.h"
// field.h has to precede the others
#include "mtx64/bitstring.h"
#include "mtx64/io.h"
#include "mtx64/slab.h"
#include <assert.h>

// meataxe64 is architecture specific
#ifndef __x86_64__
#error Meataxe package requires x86_64
#endif

/* The slab level interface with meataxe64 uses mainly
   interfaces defined in mtx64/field.h and mtx64/slab.h.

   This defines four types of object of interest, a FIELD (a large
   structure), a FELT (a 64 bit value representing a field element, a
   DSpace (a small structure that gathers some information about a
   bunch of vectors) and a a DFmt object, which is a char pointer to a
   block of bytes which contain vectors. We create the DSpace on the
   fly when we need it. We store the number of columns with the Dfmt
   data and also a UInt for the number of rown. We call this a
   MTX64_Matrix in GAP.

   We wrap FELTS (slightly inefficient, but we dont expect working
   with them to be performance critical), FIELDS, and blocks
   of Dfmt data in T_DATOBJ objects. We store a FIELD in the family of each
   MTX64_Matrix and FELT.

   We make no attempt to unite these objects with FFE and List types in GAP.
   They are entirely separate objects accessed only by their own functions.

   We might want to change this later and unite these with the
   MatrixObj development, or pay for another layer of wrapping.

   TODO -- consts, asserts and actual checks on arguments in appropriate places.
   TODO -- consistentise names
   TODO -- more comments

   For now we also deal with meataxe64 bitstrings. There are used to indicate
   sets
   of rows or columns of a matrix (for instance pivot rows in echelonisation).
   We
   again store these in T_DATOBJ objects, all with the same type

   TODO probably move bistring handling to another file.
 */

/* As usual, we construct types in GAP and import them, so these C variables are
   just copies of the corresponding GAP variables */

static Obj TYPE_MTX64_Field;  // global variable. All FIELDS have same type
static Obj TYPE_MTX64_Felt;   // function, takes field returns type of FELT
static Obj TYPE_MTX64_Matrix; // function,  takes field returns type of mx
static Obj
    TYPE_MTX64_BitString; // global variable, type of MTX64 Bitstring objects

// We also want the filters we need for testing whether objects we are passed
// are
// what we want

static Obj IsMTX64FiniteField;
static Obj IsMTX64BitString;
static Obj IsMTX64FiniteFieldElement;
static Obj IsMTX64Matrix;

/* Functions that deal with the organisation of bitstring data in a bag */

static inline uint64_t *DataOfBitStringObject(Obj bs) {
  return (uint64_t *)(ADDR_OBJ(bs) + 1);
}

static inline UInt Size_Bits_BitString(UInt len) {
  return 8 * ((len + 63) / 64);
}

static inline UInt Size_Data_BitString(UInt len) {
    return 2 * sizeof(uint64_t) + Size_Bits_BitString(len);
}

static inline UInt Size_Bag_BitString(UInt len) {
  return sizeof(Obj) + Size_Data_BitString(len);
}

static inline Obj MTX64_MakeBitString(UInt len) {
  Obj bs = NewBag(T_DATOBJ, Size_Bag_BitString(len));
  SET_TYPE_DATOBJ(bs, TYPE_MTX64_BitString);
  return bs;
}

static inline UInt IS_MTX64_BitString(Obj bs) {
  return (IS_DATOBJ(bs) && DoFilter(IsMTX64BitString, bs));
}

static inline void CHECK_MTX64_BitString(Obj bs) {
  if (!IS_MTX64_BitString(bs))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 bitstring", 0, 0);
}

/* Functions that deal with the layout of a FIELD in a bag */

static inline FIELD *DataOfFieldObject(Obj f) {
  return (FIELD *)(ADDR_OBJ(f) + 1);
}

// We just trust the field order to be a prime power here
// that check can happen at GAP level. FieldSet will exit
// if it is not
static Obj MakeMtx64Field(UInt field_order) {
  Obj field = NewBag(T_DATOBJ, FIELDLEN + sizeof(Obj));
  SET_TYPE_DATOBJ(field, TYPE_MTX64_Field);
  FieldSet(field_order, DataOfFieldObject(field));
  return field;
}

static inline UInt IS_MTX64_Field(Obj f) {
  return (IS_DATOBJ(f) && DoFilter(IsMTX64FiniteField, f));
}

static inline void CHECK_MTX64_Field(Obj f) {
  if (!IS_MTX64_Field(f))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 field", 0, 0);
}

/* Functions that deal with the layout of a FELT in a bag */

static inline FELT GetFELTFromFELTObject(Obj f) {
  return *(FELT *)(ADDR_OBJ(f) + 1);
}

static inline void SetFELTOfFELTObject(Obj f, FELT x) {
  *(FELT *)(ADDR_OBJ(f) + 1) = x;
}

static Obj MakeMtx64Felt(Obj field, FELT x) {
  Obj f = NewBag(T_DATOBJ, sizeof(FELT) + sizeof(Obj));
  UInt q = DataOfFieldObject(field)->fdef;
  Obj type = CALL_1ARGS(TYPE_MTX64_Felt, INTOBJ_INT(q));
  SET_TYPE_DATOBJ(f, type);
  SetFELTOfFELTObject(f, x);
  return f;
}

static inline UInt IS_MTX64_FELT(Obj x) {
  return (IS_DATOBJ(x) && DoFilter(IsMTX64FiniteFieldElement, x));
}

static inline void CHECK_MTX64_FELT(Obj x) {
  if (!IS_MTX64_FELT(x))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 field element", 0, 0);
}

/* Functions that deal with the layout of a matrix in a bag */

typedef struct {
  UInt noc;
  UInt nor;
} Matrix_Header;

static inline Matrix_Header *HeaderOfMatrix(Obj mx) {
  return (Matrix_Header *)(ADDR_OBJ(mx) + 1);
}

static inline UInt Size_Data_Matrix(Obj f, UInt noc, UInt nor) {
  DSPACE ds;
  DSSet(DataOfFieldObject(f), noc, &ds);
  return ds.nob * nor;
}

static inline UInt Size_Bag_Matrix(Obj f, UInt noc, UInt nor) {
  return sizeof(Obj) + sizeof(Matrix_Header) + Size_Data_Matrix(f, noc, nor);
}

static inline Obj NEW_MTX64_Matrix(Obj f, UInt nor, UInt noc) {
  Obj m;
  m = NewBag(T_DATOBJ, Size_Bag_Matrix(f, noc, nor));
  SET_TYPE_DATOBJ(
      m, CALL_1ARGS(TYPE_MTX64_Matrix, INTOBJ_INT(DataOfFieldObject(f)->fdef)));
  HeaderOfMatrix(m)->noc = noc;
  HeaderOfMatrix(m)->nor = nor;
  return m;
}

static inline Dfmt *DataOfMTX64_Matrix(Obj m) {
  return (Dfmt *)(HeaderOfMatrix(m) + 1);
}

static inline UInt IS_MTX64_Matrix(Obj m) {
  return (IS_DATOBJ(m) && DoFilter(IsMTX64Matrix, m));
}

// argument checking utilities

static Obj FieldOfMTX64FELT;
static Obj FieldOfMTX64Matrix;

static inline void CHECK_MTX64_Matrix(Obj m) {
  if (!IS_MTX64_Matrix(m))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 matrix", 0, 0);
}

// level = 0 same field, level = 1 same field and width, level = 2
// same field and shape
static inline void CHECK_MTX64_Matrices(Obj m1, Obj m2, UInt level) {
  if (!IS_MTX64_Matrix(m1) || !IS_MTX64_Matrix(m2))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 matrix", 0, 0);
  if (FAMILY_OBJ(m1) != FAMILY_OBJ(m2))
    ErrorMayQuit("Meataxe64 matrices not over the same field", 0, 0);
  if (level >= 1 && HeaderOfMatrix(m1)->noc != HeaderOfMatrix(m2)->noc)
    ErrorMayQuit("Meataxe64 matrices not same width", 0, 0);
  if (level >= 2 && HeaderOfMatrix(m1)->nor != HeaderOfMatrix(m2)->nor)
    ErrorMayQuit("Meataxe64 matrices not same shape", 0, 0);
}

static inline void CHECK_MTX64_MATRIX_FELT(Obj m, Obj x) {
  if (CALL_1ARGS(FieldOfMTX64FELT, x) != CALL_1ARGS(FieldOfMTX64Matrix, m))
    ErrorMayQuit("MTX64: element is not over same field as matrix", 0, 0);
}

static inline void CHECK_NONNEG_SMALLINT(Obj a) {
  if (!IS_INTOBJ(a) || INT_INTOBJ(a) < 0)
    ErrorMayQuit("Meataxe64: argument should be a non-negative integer < 2^60",
                 0, 0);
}

static inline void CHECK_NONNEG_SMALLINTS(Obj a, Obj b) {
  CHECK_NONNEG_SMALLINT(a);
  CHECK_NONNEG_SMALLINT(b);
}

// We assume that we have already checked that m is matrix
static inline void CHECK_MTX64_Coords(Obj row, Obj col, Obj m) {
  CHECK_NONNEG_SMALLINTS(row, col);
  if (INT_INTOBJ(row) >= HeaderOfMatrix(m)->nor ||
      INT_INTOBJ(col) >= HeaderOfMatrix(m)->noc)
    ErrorMayQuit("Meataxe64: index out of range", 0, 0);
}

// We assume that we have already checked that m is matrix
static inline void CHECK_MTX64_Row(Obj row, Obj m) {
  CHECK_NONNEG_SMALLINT(row);
  if (INT_INTOBJ(row) >= HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: row out of range", 0, 0);
}

static inline void CHECK_MTX64_RowCount(Obj row, Obj m) {
  CHECK_NONNEG_SMALLINT(row);
  if (INT_INTOBJ(row) > HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: matrix has too few rows", 0, 0);
}

// We assume that we have already checked that m is matrix
static inline void CHECK_MTX64_RowRange(Obj startrow, Obj nrows, Obj m) {
  CHECK_NONNEG_SMALLINTS(startrow, nrows);
  if (INT_INTOBJ(startrow) + INT_INTOBJ(nrows) > HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: row range too large for matrix: %i %i", INT_INTOBJ(startrow) + INT_INTOBJ(nrows) , HeaderOfMatrix(m)->nor);
}

/* GAP Callable low-level creation and access functions */

static Obj MTX64_CREATE_FIELD(Obj self, Obj field_order) {
  // This is capitalised because we can't check here whether
  // the order is a prime power and the underlying functions will exit
  // if it isn't.
  // We do what checking we can.
  if (!IS_POS_INT(field_order))
    ErrorMayQuit(
        "MTX64_CreateField: argument must be a prime power < 2^64, not a %s",
        (Int)TNAM_OBJ(field_order), 0);
  UInt q = UInt_ObjInt(field_order);
  return MakeMtx64Field(q);
}

Obj MTX64_FieldOrder(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return INTOBJ_INT(_f->fdef);
}

Obj MTX64_FieldCharacteristic(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return INTOBJ_INT(_f->charc);
}

Obj MTX64_FieldDegree(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return INTOBJ_INT(_f->pow);
}

Obj MTX64_CreateFieldElement(Obj self, Obj field, Obj elt) {
  UInt ielt;
  CHECK_MTX64_Field(field);
  if (!IS_POS_INT(elt) && elt != INTOBJ_INT(0))
    ErrorMayQuit(
        "MTX64_CreateFieldElement: element should be a non-negative integer", 0,
        0);
  ielt = UInt_ObjInt(elt); // will error out for over-large integers
  if (ielt >= DataOfFieldObject(field)->fdef)
    ErrorMayQuit("MTX64_CreateFieldElement: element number too large for field",
                 0, 0);
  return MakeMtx64Felt(field, ielt);
}

Obj MTX64_ExtractFieldElement(Obj self, Obj elt) {
  CHECK_MTX64_FELT(elt);
  return ObjInt_UInt(GetFELTFromFELTObject(elt));
}

// GAP bindings for meataxe64 field element functions

Obj MTX64_FieldAdd(Obj self, Obj f, Obj a, Obj b) {
  CHECK_MTX64_Field(f);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(f);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);
  return MakeMtx64Felt(f, FieldAdd(_f, _a, _b));
}

Obj MTX64_FieldNeg(Obj self, Obj field, Obj a) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);

  return MakeMtx64Felt(field, FieldNeg(_f, _a));
}

Obj MTX64_FieldSub(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);
  return MakeMtx64Felt(field, FieldSub(_f, _a, _b));
}

Obj MTX64_FieldMul(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);

  return MakeMtx64Felt(field, FieldMul(_f, _a, _b));
}

Obj MTX64_FieldInv(Obj self, Obj field, Obj a) {
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);

  return MakeMtx64Felt(field, FieldInv(_f, _a));
}

Obj MTX64_FieldDiv(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);

  return MakeMtx64Felt(field, FieldDiv(_f, _a, _b));
}

// GAP callable matrix constructors and inspectors

Obj MTX64_NewMatrix(Obj self, Obj field, Obj nor, Obj noc) {
  CHECK_MTX64_Field(field);
  CHECK_NONNEG_SMALLINTS(nor, noc);
  return NEW_MTX64_Matrix(field, INT_INTOBJ(nor), INT_INTOBJ(noc));
}

Obj MTX64_Matrix_NumRows(Obj self, Obj m) {
  CHECK_MTX64_Matrix(m);
  return INTOBJ_INT(HeaderOfMatrix(m)->nor);
}

Obj MTX64_Matrix_NumCols(Obj self, Obj m) {
  CHECK_MTX64_Matrix(m);
  return INTOBJ_INT(HeaderOfMatrix(m)->noc);
}

// GAP bindings for matrix (Dfmt) functions

static inline void SetDSpaceOfMTX64_Matrix(Obj m, DSPACE *ds) {
  // populate DSPACE on the fly, Only safe until next garbage collection
  Obj field = CALL_1ARGS(FieldOfMTX64Matrix, m);
  DSSet(DataOfFieldObject(field), HeaderOfMatrix(m)->noc, ds);
}

// Matrix entry access
// These functions have 0-based row and column adressing.
// The shift happens at GAP level between the "raw" functions and methods for
// list operations.

static FELT GetEntryMTX64(Obj m, UInt row, UInt col) {
  DSPACE ds;
  Dfmt *d;
  SetDSpaceOfMTX64_Matrix(m, &ds);
  d = DataOfMTX64_Matrix(m);
  d = DPAdv(&ds, row, d);
  return DUnpak(&ds, col, d);
}

Obj MTX64_GetEntry(Obj self, Obj m, Obj row, Obj col) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_Coords(row, col, m);
  UInt irow = INT_INTOBJ(row);
  UInt icol = INT_INTOBJ(col);
  Obj f = CALL_1ARGS(FieldOfMTX64Matrix, m);
  return MakeMtx64Felt(f, GetEntryMTX64(m, icol, irow));
}

static Obj FieldOfMTX64FELT;

void SetEntryMTX64(Obj m, UInt row, UInt col, FELT entry) {
  DSPACE ds;
  Dfmt *d;
  SetDSpaceOfMTX64_Matrix(m, &ds);
  d = DataOfMTX64_Matrix(m);
  d = DPAdv(&ds, row, d);
  DPak(&ds, col, d, entry);
}

Obj MTX64_SetEntry(Obj self, Obj m, Obj row, Obj col, Obj entry) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_Coords(row, col, m);
  UInt irow = INT_INTOBJ(row);
  UInt icol = INT_INTOBJ(col);
  CHECK_MTX64_FELT(entry);
  CHECK_MTX64_MATRIX_FELT(m, entry);
  SetEntryMTX64(m, icol, irow, GetFELTFromFELTObject(entry));
  return 0;
}

//
// For the next group of functions, we add a lot of startrow arguments
// compared to the meataxe64 C routines, because we can't keep pointers
// into the middle of Dfmt objects at GAP level
//

Obj MTX64_DCpy(Obj self, Obj src, Obj dst, Obj startrow, Obj nrows) {
  DSPACE ds;
  Dfmt *sp, *dp;
  CHECK_MTX64_Matrices(src, dst, 1);
  CHECK_MTX64_RowRange(startrow, nrows, src);
  CHECK_MTX64_RowCount(nrows, dst);
  UInt sr = INT_INTOBJ(startrow);
  UInt nor = INT_INTOBJ(nrows);
  SetDSpaceOfMTX64_Matrix(dst, &ds);
  sp = DataOfMTX64_Matrix(src);
  sp = DPAdv(&ds, sr, sp);
  dp = DataOfMTX64_Matrix(dst);
  DCpy(&ds, sp, nor, dp);
  return 0;
}

Obj MTX64_DCut(Obj self, Obj m, Obj startrow, Obj nrows, Obj startcol,
               Obj clip) {
  DSPACE ms, cs;
  Dfmt *mp, *cp;
  CHECK_MTX64_Matrices(m, clip, 0);
  CHECK_MTX64_RowRange(startrow, nrows, m);
  CHECK_MTX64_RowCount(nrows, clip);
  UInt sr = INT_INTOBJ(startrow);
  UInt nor = INT_INTOBJ(nrows);
  // No column checks here because DCut has defined behaviour for
  // too few columns in source
  SetDSpaceOfMTX64_Matrix(m, &ms);
  SetDSpaceOfMTX64_Matrix(clip, &cs);
  mp = DataOfMTX64_Matrix(m);
  mp = DPAdv(&ms, sr, mp);
  cp = DataOfMTX64_Matrix(clip);
  DCut(&ms, nor, INT_INTOBJ(startcol), mp, &cs, cp);
  return 0;
}

Obj MTX64_DPaste(Obj self, Obj clip, Obj startrow, Obj nrows, Obj startcol,
                 Obj m) {
  DSPACE ms, cs;
  Dfmt *mp, *cp;
  CHECK_MTX64_Matrices(m, clip, 0);
  CHECK_MTX64_RowRange(startrow, nrows, m);
  CHECK_NONNEG_SMALLINT(startcol);
  CHECK_MTX64_RowCount(nrows, clip);
  UInt sr = INT_INTOBJ(startrow);
  UInt nor = INT_INTOBJ(nrows);
  // No column checks here because DPaste has defined behaviour for
  // too few columns in destination
  SetDSpaceOfMTX64_Matrix(m, &ms);
  SetDSpaceOfMTX64_Matrix(clip, &cs);
  mp = DataOfMTX64_Matrix(m);
  mp = DPAdv(&ms, sr, mp);
  cp = DataOfMTX64_Matrix(clip);
  DPaste(&cs, cp, nor, INT_INTOBJ(startcol), &ms, mp);
  return 0;
}


Obj MTX64_DAdd(Obj self, Obj nrows, Obj d1, Obj d2, Obj d) {
  DSPACE ds;
  Dfmt *d1p, *d2p, *dp;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_Matrices(d, d1, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  CHECK_MTX64_RowCount(nrows, d);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  dp = DataOfMTX64_Matrix(d);
  DAdd(&ds, INT_INTOBJ(nrows), d1p, d2p, dp);
  return 0;
}

Obj MTX64_DSub(Obj self, Obj nrows, Obj d1, Obj d2, Obj d) {
  DSPACE ds;
  Dfmt *d1p, *d2p, *dp;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_Matrices(d, d1, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  CHECK_MTX64_RowCount(nrows, d);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  dp = DataOfMTX64_Matrix(d);
  DSub(&ds, INT_INTOBJ(nrows), d1p, d2p, dp);
  return 0;
}

Obj MTX64_DSMad(Obj self, Obj nrows, Obj scalar, Obj d1, Obj d2) {
  DSPACE ds;
  Dfmt *d1p, *d2p;
  FELT x;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  CHECK_MTX64_FELT(scalar);
  CHECK_MTX64_MATRIX_FELT(d1, scalar);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  x = GetFELTFromFELTObject(scalar);
  DSMad(&ds, x, INT_INTOBJ(nrows), d1p, d2p);
  return 0;
}

Obj MTX64_DSMul(Obj self, Obj nrows, Obj scalar, Obj d1) {
  DSPACE ds;
  Dfmt *dp;
  FELT x;
  CHECK_MTX64_Matrix(d1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_FELT(scalar);
  CHECK_MTX64_MATRIX_FELT(d1, scalar);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  dp = DataOfMTX64_Matrix(d1);
  x = GetFELTFromFELTObject(scalar);
  DSMul(&ds, x, INT_INTOBJ(nrows), dp);
  return 0;
}

Obj MTX64_DNzl(Obj self, Obj m, Obj row) {
  DSPACE ds;
  Dfmt *mp;
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_Row(row,m);
  SetDSpaceOfMTX64_Matrix(m, &ds);
  mp = DataOfMTX64_Matrix(m);
  mp = DPAdv(&ds,INT_INTOBJ(row),mp);
  UInt res = DNzl(&ds, mp);
  if (res == ZEROROW)
      return Fail;
  return INTOBJ_INT(res);
}

// Slab functions -- slightly higher level matrix operations
//  use HPMI
static void SetShapeAndResize(Obj mat, UInt nor, UInt noc) {
  Matrix_Header *h = HeaderOfMatrix(mat);
  Obj f = CALL_1ARGS(FieldOfMTX64Matrix, mat);
  h->nor = nor;
  h->noc = noc;
  ResizeBag(mat, Size_Bag_Matrix(f, noc, nor));
}

Obj MTX64_SLEchelizeDestructive(Obj self, Obj a) {
  CHECK_MTX64_Matrix(a);
  Matrix_Header *h = HeaderOfMatrix(a);
  UInt nrows = h->nor;
  UInt ncols = h->noc;
  UInt rklimit = (nrows > ncols) ? ncols : nrows; // minimum
  Obj rs = MTX64_MakeBitString(nrows);
  Obj cs = MTX64_MakeBitString(ncols);
  FELT det;
  Obj field = CALL_1ARGS(FieldOfMTX64Matrix, a);
  Obj m = NEW_MTX64_Matrix(field, rklimit, rklimit);
  Obj r = NEW_MTX64_Matrix(field, nrows, ncols);   // this may be a bit too high
  Obj c = NEW_MTX64_Matrix(field, ncols, rklimit); // this is a bit too high, as
                                                   // both bounds cannot be
                                                   // achieved at once
  // Done with garbage collection here
  uint64_t rank;
  uint64_t *rsp = DataOfBitStringObject(rs), *csp = DataOfBitStringObject(cs);
  Dfmt *mat = DataOfMTX64_Matrix(a), *multiply = DataOfMTX64_Matrix(m),
       *remnant = DataOfMTX64_Matrix(r), *cleaner = DataOfMTX64_Matrix(c);
  DSPACE ds;
  DSSet(DataOfFieldObject(field), ncols, &ds);
  rank = SLEch(&ds, mat, rsp, csp, &det, multiply, cleaner, remnant, nrows);
  // Garbage collection OK again here
  // Resize all the output matrices
  SetShapeAndResize(m, rank, rank);
  SetShapeAndResize(c, nrows - rank, rank);
  SetShapeAndResize(r, rank, ncols - rank);
  Obj result = NEW_PREC(7);
  AssPRec(result, RNamName("rank"), INTOBJ_INT(rank));
  AssPRec(result, RNamName("det"), MakeMtx64Felt(field, det));
  AssPRec(result, RNamName("multiplier"), m);
  AssPRec(result, RNamName("cleaner"), c);
  AssPRec(result, RNamName("remnant"), r);
  AssPRec(result, RNamName("rowSelect"), rs);
  AssPRec(result, RNamName("colSelect"), cs);
  return result;
}

Obj MTX64_SLMultiply(Obj self, Obj a, Obj b, Obj c) {
  CHECK_MTX64_Matrices(a, b, 0);
  CHECK_MTX64_Matrices(a, c, 0);
  Matrix_Header *ha = HeaderOfMatrix(a);
  Matrix_Header *hb = HeaderOfMatrix(b);
  Matrix_Header *hc = HeaderOfMatrix(c);
  UInt nora = ha->nor;
  UInt noca = ha->noc;
  UInt nocb = hb->noc;
  if (noca != hb->nor || nora != hc->nor || nocb != hc->noc)
    ErrorMayQuit("SLMultiply: matrices are incompatible shapes", 0, 0);
  Obj field = CALL_1ARGS(FieldOfMTX64Matrix, a);
  FIELD *f = DataOfFieldObject(field);
  Dfmt *ap = DataOfMTX64_Matrix(a);
  Dfmt *bp = DataOfMTX64_Matrix(b);
  Dfmt *cp = DataOfMTX64_Matrix(c);
  SLMul(f, ap, bp, cp, nora, noca, nocb);
  return 0;
}

Obj MTX64_SLTranspose(Obj self, Obj mat, Obj tra) {
  CHECK_MTX64_Matrices(mat, tra, 0);
  Matrix_Header *h = HeaderOfMatrix(mat);
  UInt nora = h->nor;
  UInt noca = h->noc;
  Matrix_Header *ht = HeaderOfMatrix(tra);
  if (nora != ht->noc || noca != ht->nor)
    ErrorMayQuit("SLTranspose: matrices are incompatible shapes", 0, 0);
  Obj field = CALL_1ARGS(FieldOfMTX64Matrix, mat);
  FIELD *f = DataOfFieldObject(field);
  Dfmt *mp = DataOfMTX64_Matrix(mat);
  Dfmt *tp = DataOfMTX64_Matrix(tra);
  SLTra(f, mp, tp, nora, noca);
  return 0;
}

// GAP Callable Bitstring operations

Obj MTX64_LengthOfBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  return INTOBJ_INT(DataOfBitStringObject(bs)[0]);
}

Obj MTX64_WeightOfBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  return INTOBJ_INT(DataOfBitStringObject(bs)[1]);
}

Obj MTX64_GetEntryOfBitString(Obj self, Obj bs, Obj pos) {
  CHECK_MTX64_BitString(bs);
  if (!IS_INTOBJ(pos) || INT_INTOBJ(pos) < 0 ||
      IS_INTOBJ(pos) >= DataOfBitStringObject(bs)[0])
    ErrorMayQuit(
        "MTX64_GetEntryOfBitString: position not an integer or out of range", 0,
        0);
  return INTOBJ_INT(BSBitRead(DataOfBitStringObject(bs), INT_INTOBJ(pos)));
}

// Unlike the meataxe64 library function, we update the weight of our bitstring

Obj MTX64_EmptyBitString(Obj self, Obj len) {
    CHECK_NONNEG_SMALLINT(len);
    Obj bs =MTX64_MakeBitString(INT_INTOBJ(len));
    DataOfBitStringObject(bs)[0] = INT_INTOBJ(len);
    DataOfBitStringObject(bs)[1] = 0;
    return bs;
}

Obj MTX64_SetEntryOfBitString(Obj self, Obj bs, Obj pos) {
  CHECK_MTX64_BitString(bs);
  if (!IS_INTOBJ(pos) || INT_INTOBJ(pos) < 0 ||
      IS_INTOBJ(pos) >= DataOfBitStringObject(bs)[0])
    ErrorMayQuit(
        "MTX64_SetEntryOfBitString: position not an integer or out of range", 0,
        0);
  UInt *d = DataOfBitStringObject(bs);
  UInt ipos = INT_INTOBJ(pos);
  UInt x = BSBitRead(d, ipos);
  BSBitSet(d, ipos);
  if (!x)
    d[1]++; // maintain weight
  return 0;
}

Obj MTX64_BSCombine(Obj self, Obj bs1, Obj bs2) {
    CHECK_MTX64_BitString(bs1);
    CHECK_MTX64_BitString(bs2);
    UInt len1 = DataOfBitStringObject(bs1)[0];
    UInt len2 = DataOfBitStringObject(bs2)[0];
    UInt wt1 = DataOfBitStringObject(bs1)[1];
    UInt wt2 = DataOfBitStringObject(bs2)[1];
    if (len2 != len1-wt1)
        ErrorMayQuit("MTX64_BSCombine: bitstrings incompatible",0,0);
    Obj comb = MTX64_MakeBitString(len1);
    Obj rif = MTX64_MakeBitString(wt1+wt2);
    BSCombine(DataOfBitStringObject(bs1),
              DataOfBitStringObject(bs2),
              DataOfBitStringObject(comb),
              DataOfBitStringObject(rif));
    Obj ret = NEW_PLIST(T_PLIST_HOM+IMMUTABLE, 2);
    SET_ELM_PLIST(ret,1, comb);
    SET_ELM_PLIST(ret,2, rif);
    SET_LEN_PLIST(ret,2);
    CHANGED_BAG(ret);
    return ret;
}

// Now we have a selection of routines needed at the GAP level
// but not explicitly present in the C library

Obj MTX64_ShallowCopyMatrix(Obj self, Obj m) {
  GAP_ASSERT(IS_MTX64_Matrix(m)); // method selection should ensure
  Obj f = CALL_1ARGS(FieldOfMTX64Matrix, m);
  UInt noc = HeaderOfMatrix(m)->noc;
  UInt nor = HeaderOfMatrix(m)->nor;
  Obj copy = NEW_MTX64_Matrix(f, nor, noc);
  memcpy(DataOfMTX64_Matrix(copy), DataOfMTX64_Matrix(m),
         Size_Data_Matrix(f, noc, nor));
  return copy;
}

// Order may not be consistent with GAP lists
Obj MTX64_compareMatrices(Obj self, Obj m1, Obj m2) {
  CHECK_MTX64_Matrices(m1, m2, 2);
  Obj f = CALL_1ARGS(FieldOfMTX64Matrix, m1);
  UInt noc = HeaderOfMatrix(m1)->noc;
  UInt nor = HeaderOfMatrix(m1)->nor;
  Int res = memcmp(DataOfMTX64_Matrix(m1), DataOfMTX64_Matrix(m2),
                   Size_Data_Matrix(f, noc, nor));
  return INTOBJ_INT((res < 0) ? -1 : (res > 0) ? 1 : 0);
}

Obj MTX64_ShallowCopyBitString(Obj self, Obj bs) {
  GAP_ASSERT(IS_MTX64_BitString(bs)); // method selection should ensure
  UInt len = DataOfBitStringObject(bs)[0];
  Obj copy = MTX64_MakeBitString(len);
  memcpy(DataOfBitStringObject(copy), DataOfBitStringObject(bs),
         Size_Data_BitString(len));
  return copy;
}

// Order may not be consistent with GAP lists. Differing lengths are OK because
// the length field at the beginning is compared first
Obj MTX64_compareBitStrings(Obj self, Obj bs1, Obj bs2) {
  CHECK_MTX64_BitString(bs1);
  CHECK_MTX64_BitString(bs2);
  UInt len = DataOfBitStringObject(bs1)[0];
  Int res = memcmp(DataOfBitStringObject(bs1), DataOfBitStringObject(bs2),
                   Size_Data_BitString(len));
  return INTOBJ_INT((res < 0) ? -1 : (res > 0) ? 1 : 0);
}

//
// Efficient conversions between GAP and meataxe64 representations of elements
// and vectors

// Make a translation table from GAP FFEs to Meataxe64 FELTS
// The entry of the table with (0-based) index i is the FELT number of
// Z(q)^(i-1)
// (the 0th entry is the FELT number of 0
//
// Only useful for fields up to 2^16 where GAP uses Zech logs
//

Obj MTX64_MakeFELTfromFFETable(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *f = DataOfFieldObject(field);
  UInt q = f->fdef;
  UInt p = f->charc;
  Obj l = NEW_PLIST(T_PLIST_CYC + IMMUTABLE, q);
  // line above may cause GC
  f = DataOfFieldObject(field);
  SET_ELM_PLIST(l, 1, INTOBJ_INT(0));
  FELT x = 1;
  FELT z = (p == q) ? f->conp : p; // primitive element
  for (int i = 0; i < q - 1; i++) {
    SET_ELM_PLIST(l, i + 2, INTOBJ_INT(x));
    x = FieldMul(f, x, z);
  }
  SET_LEN_PLIST(l, q);
  return l;
}

//
// Having made the tables in the kernel, they are actually
// stored in the field family, so we need to call GAP functions to get them
// back. The GAP level also computes the inverse table
//

static Obj MTX64_GetFELTfromFFETable;
static Obj MTX64_GetFFEfromFELTTable;

// Copy a T_VECFFE vector of appropriate field and length into
// the specified row of a meataxe64 matrix
// rownum is zero based

Obj MTX64_InsertVecFFE(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  UInt len = LEN_PLIST(v);
  if (len != HeaderOfMatrix(d)->noc)
    ErrorMayQuit("row length mismatch", 0, 0);
  if (len == 0)
    return 0;
  UInt nor = INT_INTOBJ(rownum);
  FF field = FLD_FFE(ELM_PLIST(v, 1));
  if (SIZE_FF(field) != q)
    // this should mean that the vector is written over a smaller (or bigger)
    // field. The entries may still be OK, so this signals the calling function
    // to fall back to element-by-element conversion at GAP level
    return Fail;
  Obj tab = CALL_1ARGS(MTX64_GetFELTfromFFETable, fld);
  // from here no GC
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(d, &ds);
  Obj *vptr = ADDR_OBJ(v);
  Dfmt *dptr = DataOfMTX64_Matrix(d);
  dptr = DPAdv(&ds, nor, dptr);
  Obj *tptr = ADDR_OBJ(tab);
  for (UInt i = 0; i < len; i++)
    DPak(&ds, i, dptr, INT_INTOBJ(tptr[VAL_FFE(vptr[i + 1]) + 1]));
  // GC OK again
  return INTOBJ_INT(len);
}

Obj MTX64_ExtractVecFFE(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  UInt nor = INT_INTOBJ(rownum);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  UInt p = DataOfFieldObject(fld)->charc;
  UInt deg = DataOfFieldObject(fld)->pow;
  UInt len = HeaderOfMatrix(d)->noc;
  if (len == 0)
    return NEW_PLIST(T_PLIST_EMPTY, 0);
  Obj v = NEW_PLIST(T_PLIST_FFE, len);
  SET_LEN_PLIST(v, len);
  Obj tab = CALL_1ARGS(MTX64_GetFFEfromFELTTable, fld);
  // from here no GC
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(d, &ds);
  Obj *vptr = ADDR_OBJ(v);
  Dfmt *dptr = DataOfMTX64_Matrix(d);
  dptr = DPAdv(&ds, nor, dptr);
  Obj *tptr = ADDR_OBJ(tab);
  for (UInt i = 0; i < len; i++)
    vptr[i + 1] = tptr[1 + DUnpak(&ds, i, dptr)];
  // GC OK again
  return v;
}

// If this fails the routine below will need rewriting, as GAP
// defined these vectors blockwise and D format defines them
// bytewise
GAP_STATIC_ASSERT(
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
    "Meataxe64 GF2 vector conversion expects little-endian system");

Obj MTX64_InsertVecGF2(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  if (q != 2)
    // maybe the matrix is over a bigger field and we need to
    // do this the slow way
    return Fail;
  if (!IS_GF2VEC_REP(v))
    ErrorMayQuit("MTX64_InsertVecGF2: vector should be a compressed GF2 vector",
                 0, 0);
  UInt len = LEN_GF2VEC(v);
  if (len != HeaderOfMatrix(d)->noc)
    ErrorMayQuit("row length mismatch", 0, 0);
  UInt nor = INT_INTOBJ(rownum);
  if (len == 0)
    return 0;
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(d, &ds);
  Dfmt *dptr = DataOfMTX64_Matrix(d);
  dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
  memcpy(dptr, BLOCKS_GF2VEC(v), ds.nob);
  return INTOBJ_INT(len);
}

Obj MTX64_ExtractVecGF2(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  if (q != 2)
    ErrorMayQuit("field mismatch", 0, 0);

  UInt len = HeaderOfMatrix(d)->noc;
  Obj v;
  NEW_GF2VEC(v, TYPE_LIST_GF2VEC, len);
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(d, &ds);
  Dfmt *dptr = DataOfMTX64_Matrix(d);
  dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
  memcpy(BLOCKS_GF2VEC(v), dptr, ds.nob);
  return v;
}

// makes a table such that table[i] is the
// byte of D-format corresponding to i in GAP 8 bit vector
// format

Obj MTX64_Make8BitConversion(Obj self, Obj fld) {
  CHECK_MTX64_Field(fld);
  UInt q = DataOfFieldObject(fld)->fdef;
  if (q > 256)
    ErrorMayQuit(
        "MTX64_Make8BitConversion: only defined for field sizes up to 256", 0,
        0);
  UInt p = DataOfFieldObject(fld)->charc;
  Obj tab = CALL_1ARGS(MTX64_GetFELTfromFFETable, fld);
  UInt max = q;
  Obj info = GetFieldInfo8Bit(q);
  UInt e = ELS_BYTE_FIELDINFO_8BIT(info);
  for (UInt i = 1; i < e; i++)
    max *= q;
  Obj tabout = NEW_STRING(max);
  for (UInt byte = 0; byte < max; byte++) {
    UInt1 x = byte;
    UInt1 y = 0;
    UInt1 z = 1;
    for (UInt i = 0; i < e; i++) {
      UInt ent = x % q;
      x /= q;
      FFV ent2 = VAL_FFE(FFE_FELT_FIELDINFO_8BIT(info)[ent]);
      FELT ent3 = INT_INTOBJ(ELM_PLIST(tab, ent2 + 1));
      y += ent3 * z;
      z *= q;
    }
    CHARS_STRING(tabout)[byte] = y;
  }
  return tabout;
}

static Obj MTX64_Get8BitImportTable;
static Obj MTX64_Get8BitExportTable;

Obj MTX64_InsertVec8Bit(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  if (!IS_VEC8BIT_REP(v))
    ErrorMayQuit("MTX64_InsertVec8Bit: bad vector format", 0, 0);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  if (q != FIELD_VEC8BIT(v))
    // maybe the matrix is over a bigger field and we need to
    // do this the slow way
    return Fail;
  UInt len = LEN_VEC8BIT(v);
  if (len != HeaderOfMatrix(d)->noc)
    ErrorMayQuit("row length mismatch", 0, 0);
  if (len == 0)
    return 0;
  Obj tbl = CALL_1ARGS(MTX64_Get8BitImportTable, fld);
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(d, &ds);
  const UChar *tptr = (const UChar *)CHARS_STRING(tbl);
  Dfmt *dptr = DataOfMTX64_Matrix(d);
  const UInt1 *vptr = (const UInt1 *)BYTES_VEC8BIT(v);
  dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
  for (UInt i = 0; i < ds.nob; i++)
    dptr[i] = tptr[vptr[i]];
  return INTOBJ_INT(len);
}

Obj MTX64_ExtractVec8Bit(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, d);
  UInt q = DataOfFieldObject(fld)->fdef;
  if (q == 2 || q > 256)
    ErrorMayQuit("field mismatch", 0, 0);
  UInt len = HeaderOfMatrix(d)->noc;
  Obj v = ZeroVec8Bit(q, len, 1);
  DSPACE ds;
  Obj tbl = CALL_1ARGS(MTX64_Get8BitExportTable, fld);
  SetDSpaceOfMTX64_Matrix(d, &ds);
  const Dfmt *dptr = (const Dfmt *)DataOfMTX64_Matrix(d);
  UInt1 *vptr = BYTES_VEC8BIT(v);
  const UChar *tptr = (const UChar *)CHARS_STRING(tbl);
  dptr = (const Dfmt *)DPAdv(&ds, INT_INTOBJ(rownum), dptr);
  for (UInt i = 0; i < ds.nob; i++)
    vptr[i] = tptr[dptr[i]];
  return v;
}

// IO

Obj MTX64_WriteMatrix(Obj self, Obj mx, Obj fname) {
  CHECK_MTX64_Matrix(mx);
  if (!IsStringConv(fname))
    ErrorMayQuit("MTX64_WriteMatrix: filename must be a string", 0, 0);
  UInt header[5];
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, mx);
  UInt nor = HeaderOfMatrix(mx)->nor;
  UInt noc = HeaderOfMatrix(mx)->noc;
  header[0] = 1;
  header[1] = DataOfFieldObject(fld)->fdef;
  header[2] = nor;
  header[3] = noc;
  EFIL *f = EWHdr((const char *)CHARS_STRING(fname),
                  header); // exits if file won't open
  EWData(f, Size_Data_Matrix(fld, noc, nor), DataOfMTX64_Matrix(mx));
  EWClose(f);
  return True;
}

static Obj MTX64_FiniteField;

Obj MTX64_ReadMatrix(Obj self, Obj fname) {
  if (!IsStringConv(fname))
    ErrorMayQuit("MTX64_ReadMatrix: filename must be a string", 0, 0);
  UInt header[5];
  EFIL *f = ERHdr((const char *)CHARS_STRING(fname), header);
  // We need to pass this construction out to GAP because the caching
  // of fields and families happens there
  Obj fld = CALL_1ARGS(MTX64_FiniteField, ObjInt_UInt(header[1]));
  UInt nor = header[2];
  UInt noc = header[3];
  Obj mx = NEW_MTX64_Matrix(fld, nor, noc);
  ERData(f, Size_Data_Matrix(fld, noc, nor), DataOfMTX64_Matrix(mx));
  ERClose(f);
  return mx;
}

// Bitstring and matrix functions

Obj MTX64_ColSelect(Obj self, Obj bitstring, Obj m) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched row length", 0, 0);
  UInt nos = DataOfBitStringObject(bitstring)[1];
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, m);
  Obj sel = NEW_MTX64_Matrix(fld, nor, nos);
  Obj nonsel = NEW_MTX64_Matrix(fld, nor, noc - nos);
  UInt *bs = DataOfBitStringObject(bitstring);
  Dfmt *d = DataOfMTX64_Matrix(m);
  Dfmt *selp = DataOfMTX64_Matrix(sel);
  Dfmt *nonselp = DataOfMTX64_Matrix(nonsel);
  FIELD *f = DataOfFieldObject(fld);
  BSColSelect(f, bs, nor, d, selp, nonselp);
  Obj ret = NEW_PLIST(T_PLIST_HOM + IMMUTABLE, 2);
  SET_LEN_PLIST(ret, 2);
  SET_ELM_PLIST(ret, 1, sel);
  SET_ELM_PLIST(ret, 2, nonsel);
  CHANGED_BAG(ret);
  return ret;
}

Obj MTX64_RowSelect(Obj self, Obj bitstring, Obj m) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (nor != DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched matrix length: matrix %i, bitstring %i", nor, DataOfBitStringObject(bitstring)[0]);
  UInt nos = DataOfBitStringObject(bitstring)[1];
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, m);
  Obj sel = NEW_MTX64_Matrix(fld, nos, noc);
  Obj nonsel = NEW_MTX64_Matrix(fld, nor-nos, noc);
  if (noc != 0) {
      DSPACE ds;
      SetDSpaceOfMTX64_Matrix(m, &ds);
      UInt *bs = DataOfBitStringObject(bitstring);
      Dfmt *d = DataOfMTX64_Matrix(m);
      Dfmt *selp = DataOfMTX64_Matrix(sel);
      Dfmt *nonselp = DataOfMTX64_Matrix(nonsel);
      for (UInt i = 0; i < nor; i++) {
          if (BSBitRead(bs, i)) {
              memcpy(selp, d, ds.nob);
              selp = DPAdv(&ds, 1, selp);
          } else {
              memcpy(nonselp, d, ds.nob);
              nonselp = DPAdv(&ds, 1, nonselp);
          }
          d = DPAdv(&ds, 1, d);
      }
  }
  Obj ret = NEW_PLIST(T_PLIST_HOM + IMMUTABLE, 2);
  SET_LEN_PLIST(ret, 2);
  SET_ELM_PLIST(ret, 1, sel);
  SET_ELM_PLIST(ret, 2, nonsel);
  CHANGED_BAG(ret);
  return ret;
}

Obj MTX64_RowCombine(Obj self, Obj bitstring, Obj m1, Obj m2) {
    CHECK_MTX64_Matrices(m1,m2,1);
    CHECK_MTX64_BitString(bitstring);
    UInt noc = HeaderOfMatrix(m1)->noc;
    UInt nor1 = HeaderOfMatrix(m1)->nor;
    UInt nor2 = HeaderOfMatrix(m2)->nor;
    UInt len = DataOfBitStringObject(bitstring)[0];
    UInt wt = DataOfBitStringObject(bitstring)[1];
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m1);
    if (nor1 != wt || nor2 != len-wt)
        ErrorMayQuit("MTX64_RowCombine: matrices and bitstring don't match",0,0);
    Obj m = NEW_MTX64_Matrix(f, len, noc);
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    Dfmt *d1 = DataOfMTX64_Matrix(m1);
    Dfmt *d2 = DataOfMTX64_Matrix(m2);
    Dfmt *d = DataOfMTX64_Matrix(m);
    UInt *bs = DataOfBitStringObject(bitstring);
    for (UInt i = 0; i < len; i++) {
        if (BSBitRead(bs, i)) {
            memcpy(d,d1,ds.nob);
            d1 = DPAdv(&ds,1,d1);
        } else {
            memcpy(d,d2,ds.nob);
            d2 = DPAdv(&ds,1,d2);
        }
        d = DPAdv(&ds,1,d);
    }
    return m;
}
    

Obj MTX64_BSColRifZ(Obj self, Obj bitstring, Obj m) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[1])
    ErrorMayQuit("mismatched row length", 0, 0);
  UInt noc2 = DataOfBitStringObject(bitstring)[0];
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, m);
  Obj out = NEW_MTX64_Matrix(fld, nor, noc2);
  UInt *bs = DataOfBitStringObject(bitstring);
  Dfmt *d = DataOfMTX64_Matrix(m);
  Dfmt *outp = DataOfMTX64_Matrix(out);
  BSColRifZ(DataOfFieldObject(fld), bs, nor, d, outp);
  return out;
}


Obj MTX64_BSColPutS(Obj self, Obj bitstring, Obj m, Obj x) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  CHECK_MTX64_MATRIX_FELT(m, x);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched row length", 0, 0);
  Obj fld = CALL_1ARGS(FieldOfMTX64Matrix, m);
  UInt *bs = DataOfBitStringObject(bitstring);
  Dfmt *d = DataOfMTX64_Matrix(m);
  FELT f = GetFELTFromFELTObject(x);
  BSColPutS(DataOfFieldObject(fld), bs, nor, f, d);
  return 0;
}

static void RecountBS(Obj bs) {
    UInt *d = DataOfBitStringObject(bs);
    d[1] = COUNT_TRUES_BLOCKS(d+2,(d[0]+63)/64);
}

Obj MTX64_BSShiftOr(Obj self, Obj bs1, Obj shift, Obj bs2) {
    CHECK_MTX64_BitString(bs1);
    CHECK_MTX64_BitString(bs2);
    CHECK_NONNEG_SMALLINT(shift);
    UInt len1 = DataOfBitStringObject(bs1)[0];
    UInt len2 = DataOfBitStringObject(bs2)[0];
    UInt ishift = INT_INTOBJ(shift);
    if (len1 + ishift > len2)
        ErrorMayQuit("BSShiftOr: destination bitstring not long enough", 0, 0);
    BSShiftOr(DataOfBitStringObject(bs1), ishift, DataOfBitStringObject(bs2));
    RecountBS(bs2);
    return 0;
}


typedef Obj (*GVarFunc)(/*arguments*/);
#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params)                   \
  { #name, nparam, params, (GVarFunc) name, srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs[] = {
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CREATE_FIELD, 1, "q"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldOrder, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldCharacteristic, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDegree, 1, "f"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateFieldElement, 2, "f,x"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractFieldElement, 1, "e"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldAdd, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldNeg, 2, "f,a"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldSub, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldMul, 3, "f,a,b"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldInv, 2, "f,a"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDiv, 3, "f,a,b"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_NewMatrix, 3, "f,nor,noc"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Matrix_NumRows, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Matrix_NumCols, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_GetEntry, 3, "m,i,j"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SetEntry, 4, "m,i,j,x"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCpy, 4,
                          "src,dst,startrow,nrows"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCut, 5,
                          "m,startrow,nrows,startcol,clip"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DPaste, 5,
                          "clip,startrow,nrows,startcol,m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DAdd, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSub, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMad, 4, "nrows,scalar,d1,d2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMul, 3, "nrows,scalar,d1"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DNzl, 2, "m, row"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLMultiply, 3, "a, b, c"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLTranspose, 2, "m, t"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLEchelizeDestructive, 1, "a"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_LengthOfBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_WeightOfBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SetEntryOfBitString, 2,
                          "bs, pos"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_GetEntryOfBitString, 2,
                          "bs, pos"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_EmptyBitString, 1, "len"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_BSShiftOr, 3,
                          "bs1, shift, bs2"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ShallowCopyMatrix, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ShallowCopyBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_compareMatrices, 2, "m1, m2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_compareBitStrings, 2,
                          "bs1, bs2"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_MakeFELTfromFFETable, 1, "q"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVecFFE, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVecFFE, 2, "d, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVecGF2, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVecGF2, 2, "d, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Make8BitConversion, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVec8Bit, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVec8Bit, 2, "d, row"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_WriteMatrix, 2, "m, fn"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ReadMatrix, 1, "fn"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ColSelect, 2, "bs, m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_RowSelect, 2, "bs, m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_RowCombine, 3, "bs, m1, m2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_BSColRifZ, 2, "bs, m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_BSColPutS, 3, "bs, m, x"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_BSCombine, 2, "bs1, bs2"),

    {0} /* Finish with an empty entry */

};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel(StructInitInfo *module) {
  /* init filters and functionsi */
  InitHdlrFuncsFromTable(GVarFuncs);

  // Types etc.
  ImportGVarFromLibrary("MTX64_FieldType", &TYPE_MTX64_Field);
  ImportGVarFromLibrary("MTX64_BitStringType", &TYPE_MTX64_BitString);
  ImportFuncFromLibrary("MTX64_FieldEltType", &TYPE_MTX64_Felt);
  ImportFuncFromLibrary("MTX64_MatrixType", &TYPE_MTX64_Matrix);
  ImportFuncFromLibrary("IsMTX64BitString", &IsMTX64BitString);
  ImportFuncFromLibrary("IsMTX64FiniteField", &IsMTX64FiniteField);
  ImportFuncFromLibrary("IsMTX64Matrix", &IsMTX64Matrix);
  ImportFuncFromLibrary("IsMTX64FiniteFieldElement",
                        &IsMTX64FiniteFieldElement);

  // Access via family
  ImportFuncFromLibrary("FieldOfMTX64Matrix", &FieldOfMTX64Matrix);
  ImportFuncFromLibrary("FieldOfMTX64FELT", &FieldOfMTX64FELT);

  // Construct various lookup tables
  ImportFuncFromLibrary("MTX64_GetFFEfromFELTTable",
                        &MTX64_GetFFEfromFELTTable);
  ImportFuncFromLibrary("MTX64_GetFELTfromFFETable",
                        &MTX64_GetFELTfromFFETable);
  ImportFuncFromLibrary("MTX64_Get8BitImportTable", &MTX64_Get8BitImportTable);
  ImportFuncFromLibrary("MTX64_Get8BitExportTable", &MTX64_Get8BitExportTable);

  // Construct fields, respecting caching
  ImportFuncFromLibrary("MTX64_FiniteField", &MTX64_FiniteField);

  /* return success */
  return 0;
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary(StructInitInfo *module) {
  /* init filters and functions */
  InitGVarFuncsFromTable(GVarFuncs);

  /* return success */
  return 0;
}

/******************************************************************************
*F  InitInfopl()  . . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
    /* type        = */ MODULE_DYNAMIC,
    /* name        = */ "meataxe64",
    /* revision_c  = */ 0,
    /* revision_h  = */ 0,
    /* version     = */ 0,
    /* crc         = */ 0,
    /* initKernel  = */ InitKernel,
    /* initLibrary = */ InitLibrary,
    /* checkInit   = */ 0,
    /* preSave     = */ 0,
    /* postSave    = */ 0,
    /* postRestore = */ 0};

StructInitInfo *Init__Dynamic(void) { return &module; }
