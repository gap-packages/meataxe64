#ifndef MTX64_MEATAXE64_H
#define MTX64_MEATAXE64_H
#include "src/compiled.h" /* GAP headers */


// TODO Split this file up along the lines of the split between the different
// headers in meataxe64

#include "mtx64/field.h"

#include "functions.h" /* headers from other files in this package */
#include "slab.h" /* headers from other files in this package */

#include <assert.h>

// meataxe64 is architecture specific
#ifndef __x86_64__
#error Meataxe package requires x86_64
#endif


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


extern Obj MTX64_MakeBitString(UInt len);

extern UInt IS_MTX64_BitString(Obj bs);

static inline void CHECK_MTX64_BitString(Obj bs) {
  if (!IS_MTX64_BitString(bs))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 bitstring", 0, 0);
}

/* Functions that deal with the layout of a FIELD in a bag */

static inline FIELD *DataOfFieldObject(Obj f) {
  return (FIELD *)(ADDR_OBJ(f) + 1);
}

extern UInt IS_MTX64_Field(Obj f);

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

extern Obj MakeMtx64Felt(Obj field, FELT x);
extern Obj MakeMtx64Field(UInt field_order);

extern Obj FieldOfFELT(Obj f);

extern UInt IS_MTX64_FELT(Obj x);

static inline void CHECK_MTX64_FELT(Obj x) {
  if (!IS_MTX64_FELT(x))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 field element", 0, 0);
}

// Import GAP level function that gets this information (from the family)
static Obj FieldOfMTX64Matrix;

typedef struct {
  UInt noc;
  UInt nor;
} Matrix_Header;

static inline Matrix_Header *HeaderOfMatrix(Obj mx) {
  return (Matrix_Header *)(ADDR_OBJ(mx) + 1);
}

extern Obj FieldOfMatrix(Obj m);

static inline Dfmt *DataOfMTX64_Matrix(Obj m) {
  return (Dfmt *)(HeaderOfMatrix(m) + 1);
}

extern UInt IS_MTX64_Matrix(Obj m);


static inline UInt Size_Data_Matrix(Obj f, UInt noc, UInt nor) {
  DSPACE ds;
  DSSet(DataOfFieldObject(f), noc, &ds);
  return ds.nob * nor;
}

static inline UInt Size_Bag_Matrix(Obj f, UInt noc, UInt nor) {
  return sizeof(Obj) + sizeof(Matrix_Header) + Size_Data_Matrix(f, noc, nor);
}

extern Obj NEW_MTX64_Matrix(Obj f, UInt nor, UInt noc);

// argument checking utilities

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

// Assumes they have been checked individually
static inline void CHECK_MTX64_MATRIX_FELT(Obj m, Obj x) {
  if (FieldOfFELT(x) != FieldOfMatrix(m))
    ErrorMayQuit("MTX64: element is not over same field as matrix", 0, 0);
}

// Can't help feeling this should exist somewhere more general
static inline void CHECK_MUT(Obj o) {
  if (!IS_MUTABLE_OBJ(o))
    ErrorMayQuit("MTX64: object must be mutable", 0, 0);
}

// We need this a lot
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
// Check that row and col are valid row and column numbers for this mx
// 0-based
static inline void CHECK_MTX64_Coords(Obj row, Obj col, Obj m) {
  CHECK_NONNEG_SMALLINTS(row, col);
  if (INT_INTOBJ(row) >= HeaderOfMatrix(m)->nor ||
      INT_INTOBJ(col) >= HeaderOfMatrix(m)->noc)
    ErrorMayQuit("Meataxe64: index out of range", 0, 0);
}

// We assume that we have already checked that m is matrix
// Just check a 0-based row number
static inline void CHECK_MTX64_Row(Obj row, Obj m) {
  CHECK_NONNEG_SMALLINT(row);
  if (INT_INTOBJ(row) >= HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: row out of range", 0, 0);
}

// Check a row count -- slightly different in that n is allowed
static inline void CHECK_MTX64_RowCount(Obj row, Obj m) {
  CHECK_NONNEG_SMALLINT(row);
  if (INT_INTOBJ(row) > HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: matrix has too few rows", 0, 0);
}

// We assume that we have already checked that m is matrix
// Check a starting row/number of rows pair represent a valid row range
// 0-based
static inline void CHECK_MTX64_RowRange(Obj startrow, Obj nrows, Obj m) {
  CHECK_NONNEG_SMALLINTS(startrow, nrows);
  if (INT_INTOBJ(startrow) + INT_INTOBJ(nrows) > HeaderOfMatrix(m)->nor)
    ErrorMayQuit("Meataxe64: row range too large for matrix: %i %i",
                 INT_INTOBJ(startrow) + INT_INTOBJ(nrows),
                 HeaderOfMatrix(m)->nor);
}


// This is used to populate a DSpace structure on the fly. Since it
// contains a C pointer to the field, we can't keep it alive through a GC

static inline void SetDSpaceOfMTX64_Matrix(Obj m, DSPACE *ds) {
  Obj field = FieldOfMatrix(m);
  DSSet(DataOfFieldObject(field), HeaderOfMatrix(m)->noc, ds);
}

#endif
