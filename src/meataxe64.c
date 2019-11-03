/*
 * meataxe64: meataxe64
 *
 * This file contains interfaces to the "slab" level of meataxe64. This sits
 * below the thread farm and provides single-threaded matrix arithmetic. "Full"
 * meataxe64 computations use the thread farm to make multiple "slab" calls in
 * parallel on multi-core systems.
 *
 * The slab level functions are designed for matrix dimensions in the range from
 * a few hundred to a few tens of thousands, although there are no hard limits.
 */
#include "meataxe64.h"
#include "src/vec8bit.h" /* GAP headers -- we need the internals of these objects */
#include "src/vecgf2.h" /* GAP headers    for efficient vector conversion */

#include "mtx64/bitstring.h"
#include "mtx64/io.h"

/* This defines four types of object of interest, a FIELD (a large
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
   sets of rows or columns of a matrix (for instance pivot rows in
   echelonisation).
   We again store these in T_DATOBJ objects, all with the same type  */

/* As usual, we construct types in GAP and import them, so these C variables are
   just copies of the corresponding GAP variables */

static Obj TYPE_MTX64_Field;  // global variable. All FIELDS have same type
static Obj TYPE_MTX64_Felt;   // function, takes field returns type of FELT
static Obj TYPE_MTX64_Matrix; // function,  takes field returns type of mx

// We also import the filters we need for testing whether objects we are passed
// are what we want

static Obj IsMTX64FiniteField;
static Obj IsMTX64FiniteFieldElement;
static Obj IsMTX64Matrix;


UInt IS_MTX64_Field(Obj f) {
  return (IS_DATOBJ(f) && DoFilter(IsMTX64FiniteField, f));
}

UInt IS_MTX64_FELT(Obj x) {
  return (IS_DATOBJ(x) && DoFilter(IsMTX64FiniteFieldElement, x));
}

static Obj MTX64_FieldOfMatrix;

Obj FieldOfMatrix(Obj m) {
  return CALL_1ARGS(MTX64_FieldOfMatrix, m);
}

UInt IS_MTX64_Matrix(Obj m) {
  return (IS_DATOBJ(m) && DoFilter(IsMTX64Matrix, m));
}

Obj NEW_MTX64_Matrix(Obj f, UInt nor, UInt noc) {
  Obj t = CALL_1ARGS(TYPE_MTX64_Matrix, ObjInt_UInt(DataOfFieldObject(f)->fdef));
  Obj m = NewBag(T_DATOBJ, Size_Bag_Matrix(f, noc, nor));
  SET_TYPE_OBJ(m, t);
  HeaderOfMatrix(m)->noc = noc;
  HeaderOfMatrix(m)->nor = nor;
  return m;
}

// We just trust the field order to be a prime power here
// that check can happen at GAP level. FieldSet will exit
// if it is not
Obj MakeMtx64Field(UInt field_order) {
  Obj field = NewBag(T_DATOBJ, FIELDLEN + sizeof(Obj));
  SET_TYPE_OBJ(field, TYPE_MTX64_Field);
  FieldSet(field_order, DataOfFieldObject(field));
  return field;
}


Obj MakeMtx64Felt(Obj field, FELT x) {
  UInt q = DataOfFieldObject(field)->fdef;
  Obj type = CALL_1ARGS(TYPE_MTX64_Felt, ObjInt_UInt(q));
  Obj f = NewBag(T_DATOBJ, sizeof(FELT) + sizeof(Obj));
  SET_TYPE_OBJ(f, type);
  SetFELTOfFELTObject(f, x);
  return f;
}

static Obj MTX64_FieldOfElement;

Obj FieldOfFELT(Obj f) { return CALL_1ARGS(MTX64_FieldOfElement, f); }

/* Functions that deal with the layout of a matrix in a bag */


/* GAP Callable low-level creation and access functions */

// fields

static Obj FuncMTX64_CREATE_FIELD(Obj self, Obj field_order) {
  // This is capitalised because we can't check here whether
  // the order is a prime power and the underlying functions will exit
  // if it isn't. We do what checking we can.
  if (!IS_POS_INT(field_order))
    ErrorMayQuit(
        "MTX64_CreateField: argument must be a prime power < 2^64, not a %s",
        (Int)TNAM_OBJ(field_order), 0);
  UInt q = UInt_ObjInt(field_order);
  return MakeMtx64Field(q);
}

static Obj FuncMTX64_FieldOrder(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return ObjInt_UInt(_f->fdef);
}

static Obj FuncMTX64_FieldCharacteristic(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return ObjInt_UInt(_f->charc);
}

static Obj FuncMTX64_FieldDegree(Obj self, Obj field) {
  CHECK_MTX64_Field(field);
  FIELD *_f = DataOfFieldObject(field);
  return INTOBJ_INT(_f->pow);
}

// field elements

static Obj FuncMTX64_CreateFieldElement(Obj self, Obj field, Obj elt) {
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

static Obj FuncMTX64_ExtractFieldElement(Obj self, Obj elt) {
  CHECK_MTX64_FELT(elt);
  return ObjInt_UInt(GetFELTFromFELTObject(elt));
}

// GAP bindings for meataxe64 field element functions

static Obj FuncMTX64_FieldAdd(Obj self, Obj f, Obj a, Obj b) {
  CHECK_MTX64_Field(f);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(f);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);
  return MakeMtx64Felt(f, FieldAdd(_f, _a, _b));
}

static Obj FuncMTX64_FieldNeg(Obj self, Obj field, Obj a) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);

  return MakeMtx64Felt(field, FieldNeg(_f, _a));
}

static Obj FuncMTX64_FieldSub(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);
  return MakeMtx64Felt(field, FieldSub(_f, _a, _b));
}

static Obj FuncMTX64_FieldMul(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);

  return MakeMtx64Felt(field, FieldMul(_f, _a, _b));
}

static Obj FuncMTX64_FieldInv(Obj self, Obj field, Obj a) {
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);

  return MakeMtx64Felt(field, FieldInv(_f, _a));
}

static Obj FuncMTX64_FieldDiv(Obj self, Obj field, Obj a, Obj b) {
  CHECK_MTX64_Field(field);
  CHECK_MTX64_FELT(a);
  CHECK_MTX64_FELT(b);
  FIELD *_f = DataOfFieldObject(field);
  FELT _a = GetFELTFromFELTObject(a);
  FELT _b = GetFELTFromFELTObject(b);

  return MakeMtx64Felt(field, FieldDiv(_f, _a, _b));
}

// GAP callable matrix constructors and inspectors

static Obj FuncMTX64_NewMatrix(Obj self, Obj field, Obj nor, Obj noc) {
  CHECK_MTX64_Field(field);
  CHECK_NONNEG_SMALLINTS(nor, noc);
  return NEW_MTX64_Matrix(field, INT_INTOBJ(nor), INT_INTOBJ(noc));
}

static Obj FuncMTX64_NumRows(Obj self, Obj m) {
  CHECK_MTX64_Matrix(m);
  return INTOBJ_INT(HeaderOfMatrix(m)->nor);
}

static Obj FuncMTX64_NumCols(Obj self, Obj m) {
  CHECK_MTX64_Matrix(m);
  return INTOBJ_INT(HeaderOfMatrix(m)->noc);
}

// GAP bindings for matrix (Dfmt) functions


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

static Obj FuncMTX64_GetEntry(Obj self, Obj m, Obj row, Obj col) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_Coords(row, col, m);
  UInt irow = INT_INTOBJ(row);
  UInt icol = INT_INTOBJ(col);
  Obj f = FieldOfMatrix(m);
  return MakeMtx64Felt(f, GetEntryMTX64(m, irow, icol));
}

void SetEntryMTX64(Obj m, UInt row, UInt col, FELT entry) {
  DSPACE ds;
  Dfmt *d;
  SetDSpaceOfMTX64_Matrix(m, &ds);
  d = DataOfMTX64_Matrix(m);
  d = DPAdv(&ds, row, d);
  DPak(&ds, col, d, entry);
}

static Obj FuncMTX64_SetEntry(Obj self, Obj m, Obj row, Obj col, Obj entry) {
  CHECK_MTX64_Matrix(m);
  CHECK_MUT(m);
  CHECK_MTX64_Coords(row, col, m);
  CHECK_MTX64_FELT(entry);
  CHECK_MTX64_MATRIX_FELT(m, entry);
  UInt irow = INT_INTOBJ(row);
  UInt icol = INT_INTOBJ(col);
  SetEntryMTX64(m, irow, icol, GetFELTFromFELTObject(entry));
  return 0;
}

//
// For the next group of functions, we add a lot of startrow arguments
// compared to the meataxe64 C routines, because we can't keep pointers
// into the middle of Dfmt objects at GAP level
//
// Other than that we are basically just wrapping the D format functions
// from mtx64/field.h
//

static Obj FuncMTX64_DCpy(Obj self, Obj src, Obj dst, Obj startrow, Obj nrows) {
  DSPACE ds;
  Dfmt *sp, *dp;
  CHECK_MTX64_Matrices(src, dst, 1);
  CHECK_MUT(dst);
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

static Obj FuncMTX64_DCut(Obj self, Obj m, Obj startrow, Obj nrows,
                          Obj startcol, Obj clip) {
  DSPACE ms, cs;
  Dfmt *mp, *cp;
  CHECK_MTX64_Matrices(m, clip, 0);
  CHECK_MUT(clip);
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

static Obj FuncMTX64_DPaste(Obj self, Obj clip, Obj startrow, Obj nrows,
                            Obj startcol, Obj m) {
  DSPACE ms, cs;
  Dfmt *mp, *cp;
  CHECK_MTX64_Matrices(m, clip, 0);
  CHECK_MUT(m);
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

static Obj FuncMTX64_DAdd(Obj self, Obj nrows, Obj d1, Obj d2) {
  DSPACE ds;
  Dfmt *d1p, *d2p, *dp;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  Obj d = NEW_MTX64_Matrix(FieldOfMatrix(d1), INT_INTOBJ(nrows),
                           HeaderOfMatrix(d1)->noc);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  dp = DataOfMTX64_Matrix(d);
  DAdd(&ds, INT_INTOBJ(nrows), d1p, d2p, dp);
  return d;
}

static Obj FuncMTX64_DSub(Obj self, Obj nrows, Obj d1, Obj d2) {
  DSPACE ds;
  Dfmt *d1p, *d2p, *dp;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  Obj d = NEW_MTX64_Matrix(FieldOfMatrix(d1), INT_INTOBJ(nrows),
                           HeaderOfMatrix(d1)->noc);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  dp = DataOfMTX64_Matrix(d);
  DSub(&ds, INT_INTOBJ(nrows), d1p, d2p, dp);
  return d;
}

static Obj FuncMTX64_DSMad(Obj self, Obj nrows, Obj scalar, Obj d1, Obj d2) {
  DSPACE ds;
  Dfmt *d1p, *d2p;
  FELT x;
  CHECK_MTX64_Matrices(d1, d2, 1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MTX64_RowCount(nrows, d2);
  CHECK_MTX64_FELT(scalar);
  CHECK_MTX64_MATRIX_FELT(d1, scalar);
  CHECK_MUT(d2);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  d1p = DataOfMTX64_Matrix(d1);
  d2p = DataOfMTX64_Matrix(d2);
  x = GetFELTFromFELTObject(scalar);
  DSMad(&ds, x, INT_INTOBJ(nrows), d1p, d2p);
  return 0;
}

static Obj FuncMTX64_DSMul(Obj self, Obj nrows, Obj scalar, Obj d1) {
  DSPACE ds;
  Dfmt *dp;
  FELT x;
  CHECK_MTX64_Matrix(d1);
  CHECK_MTX64_RowCount(nrows, d1);
  CHECK_MUT(d1);
  CHECK_MTX64_FELT(scalar);
  CHECK_MTX64_MATRIX_FELT(d1, scalar);
  SetDSpaceOfMTX64_Matrix(d1, &ds);
  dp = DataOfMTX64_Matrix(d1);
  x = GetFELTFromFELTObject(scalar);
  DSMul(&ds, x, INT_INTOBJ(nrows), dp);
  return 0;
}

// we return Fail for zero row.
static Obj FuncMTX64_DNzl(Obj self, Obj m, Obj row) {
  DSPACE ds;
  Dfmt *mp;
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_Row(row, m);
  SetDSpaceOfMTX64_Matrix(m, &ds);
  mp = DataOfMTX64_Matrix(m);
  mp = DPAdv(&ds, INT_INTOBJ(row), mp);
  UInt res = DNzl(&ds, mp);
  if (res == ZEROROW)
    return Fail;
  return INTOBJ_INT(res);
}

// Now we have a selection of routines needed at the GAP level
// but not explicitly present in the C library

static Obj FuncMTX64_ShallowCopyMatrix(Obj self, Obj m) {
  GAP_ASSERT(IS_MTX64_Matrix(m)); // method selection should ensure
  Obj f = FieldOfMatrix(m);
  UInt noc = HeaderOfMatrix(m)->noc;
  UInt nor = HeaderOfMatrix(m)->nor;
  Obj copy = NEW_MTX64_Matrix(f, nor, noc);
  memcpy(DataOfMTX64_Matrix(copy), DataOfMTX64_Matrix(m),
         Size_Data_Matrix(f, noc, nor));
  return copy;
}

// Order may not be consistent with GAP lists
static Obj FuncMTX64_CompareMatrices(Obj self, Obj m1, Obj m2) {
  CHECK_MTX64_Matrices(m1, m2, 2);
  Obj f = FieldOfMatrix(m1);
  UInt noc = HeaderOfMatrix(m1)->noc;
  UInt nor = HeaderOfMatrix(m1)->nor;
  Int res = memcmp(DataOfMTX64_Matrix(m1), DataOfMTX64_Matrix(m2),
                   Size_Data_Matrix(f, noc, nor));
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

//
// FieldMul is very slow for some fields of orders
// over 2^16 (like 2^24). Do what we needs more quickly
//
static FELT mulFeltByZ( FIELD *f, FELT x, FELT z)  {
    if (f->multyp <= 5)
        return FieldMul(f,x,z);
    if (f->charc == 2) {
        FELT x0 = x;
        x <<= 1;
        if (x & f->fdef) 
            x ^= (f->fdef | f->conp);
        return x;
    }
    x *= z;
    GAP_ASSERT(f->pow > 1);
    UInt y = (x/f->fdef);
    if (!y) return x;
    x %= f->fdef;
    // Now we need to add y*f->conp to x but treating them
    // as polynomials mod z
    UInt a = 1;
    UInt r = 0;
    UInt c = f->conp;
    for (UInt i = 0; i < f->pow; i++) {
        r += a*((x +  y * c) %z);
        a *= z;
        x /= z;
        c /= z;
    }
    return r;
}

static Obj FuncMTX64_MakeFELTfromFFETable(Obj self, Obj field) {
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
    x = mulFeltByZ(f, x, z);
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

static Obj FuncMTX64_InsertVecFFE(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  CHECK_MUT(d);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_ExtractVecFFE(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  UInt nor = INT_INTOBJ(rownum);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_InsertVecGF2(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MUT(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_ExtractVecGF2(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_Make8BitConversion(Obj self, Obj fld) {
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
      FFV ent2 = VAL_FFE(FFE_FELT_FIELDINFO_8BIT(info,ent));
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

static Obj FuncMTX64_InsertVec8Bit(Obj self, Obj d, Obj v, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MUT(d);
  CHECK_MTX64_Row(rownum, d);
  if (!IS_VEC8BIT_REP(v))
    ErrorMayQuit("MTX64_InsertVec8Bit: bad vector format", 0, 0);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_ExtractVec8Bit(Obj self, Obj d, Obj rownum) {
  CHECK_MTX64_Matrix(d);
  CHECK_MTX64_Row(rownum, d);
  Obj fld = FieldOfMatrix(d);
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

static Obj FuncMTX64_WriteMatrix(Obj self, Obj mx, Obj fname) {
  CHECK_MTX64_Matrix(mx);
  if (!IsStringConv(fname))
    ErrorMayQuit("MTX64_WriteMatrix: filename must be a string", 0, 0);
  UInt header[5];
  Obj fld = FieldOfMatrix(mx);
  UInt nor = HeaderOfMatrix(mx)->nor;
  UInt noc = HeaderOfMatrix(mx)->noc;
  header[0] = 1;
  header[1] = DataOfFieldObject(fld)->fdef;
  header[2] = nor;
  header[3] = noc;
  EFIL *f = EWHdr((const char *)CHARS_STRING(fname),
                  (uint64_t *)header); // exits if file won't open
  EWData(f, Size_Data_Matrix(fld, noc, nor), DataOfMTX64_Matrix(mx));
  EWClose(f);
  return True;
}

static Obj MTX64_FiniteField;

static Obj FuncMTX64_ReadMatrix(Obj self, Obj fname) {
  if (!IsStringConv(fname))
    ErrorMayQuit("MTX64_ReadMatrix: filename must be a string", 0, 0);
  UInt header[5];
  EFIL *f = ERHdr((const char *)CHARS_STRING(fname), (uint64_t *)header);
  // We need to pass this construction out to GAP because the caching
  // of fields and families happens there
  if (header[0] != 1)
      ErrorMayQuit("File does not contain a matrix", 0, 0);
  Obj fld = CALL_1ARGS(MTX64_FiniteField, ObjInt_UInt(header[1]));
  UInt nor = header[2];
  UInt noc = header[3];
  Obj mx = NEW_MTX64_Matrix(fld, nor, noc);
  ERData(f, Size_Data_Matrix(fld, noc, nor), DataOfMTX64_Matrix(mx));
  ERClose(f);
  return mx;
}



static inline UInt random64(UInt4 *source) {
    return ((UInt)nextrandMT_int32(source)) << 32  | nextrandMT_int32(source);
}

static inline UInt random_uniform(UInt q, UInt4 *source) {
    UInt x;
    if (q <= (1L<<32)) {        
        do {
            x = nextrandMT_int32(source);
        } while (x >= ((1L<<32)/q)*q);
    } else {
        // Somewhat ugly method of finding the largest multiple of q < 2^64
        UInt y = ((UInt)1<<63)/(q >> 1);
        UInt r = ((UInt)1<<63) % (q >> 1);
        if (y > 2*r)
            y--;
        // Let q = 2k+1. Then 2^63 = ky + r and so qy - 2^64 = y-2r.
        // If this risks being negative, we need to reduce y by 1.
        UInt ct = 0;
        do {
            x = random64(source);
        } while (x >= y*q);
    }
    return x % q;
}

static inline void random_buf(void * buf, size_t len, UInt4 *source) {
    for (UInt i = 0; i < len/8; i++)        
        ((UInt *)buf)[i] = random64(source);
    for (UInt i = 0; i < len % 8; i++)
        ((UInt1 *)buf)[(len/8)*8 + i] = nextrandMT_int32(source) % 256;   
}


static Obj FuncMTX64_RANDOM_MAT(Obj self, Obj field, Obj nrows, Obj ncols, Obj mtsource) {
    CHECK_MTX64_Field(field);
    CHECK_NONNEG_SMALLINT(nrows);
    RequireStringRep("MTX64_RANDOM_MAT", mtsource);
    if (GET_LEN_STRING(mtsource) < 2500) {
        ErrorMayQuit(
                     "MTX64_RANDOM_MAT: <mtstr> must be a string "
                     "with at least 2500 characters",
         0, 0);
    }
    UInt nor = INT_INTOBJ(nrows);
    CHECK_NONNEG_SMALLINT(ncols);
    UInt noc = INT_INTOBJ(ncols);
    Obj m = NEW_MTX64_Matrix(field, nor, noc);
    FIELD *f = DataOfFieldObject(field);
    Dfmt *mp = DataOfMTX64_Matrix(m);
    UInt q = f->fdef;
    DSPACE ds;    
    SetDSpaceOfMTX64_Matrix(m, &ds);
    UInt4 *source = (UInt4 *)CHARS_STRING(mtsource);
    for (UInt i = 0; i < nor; i++) {
        switch(f->paktyp) {
        case 0:
            for (UInt j = 0; j < noc; j++) {
                ((UInt *)mp)[j] = random_uniform(q,source);
            }
            break;
        case 1:
            for (UInt j = 0; j < noc; j++) 
                ((uint32_t *)mp)[j] = random_uniform(q,source);
            break;
        case 2:
            if (q == (1<<16)) {
                random_buf(mp, ds.nob,source);
            } else {
                for (UInt j = 0; j < noc; j++) 
                    ((uint16_t *)mp)[j] = (uint16_t)random_uniform(q,source);
            }
            break;
        case 3:
            if (q == 256) {
                random_buf(mp, ds.nob,source);
            } else {
                for (UInt j = 0; j < noc; j++) 
                    ((uint8_t *)mp)[j] = (uint8_t)random_uniform(q,source);
            }
            break;
        case 4:
            if (q == 16) {
                random_buf(mp, noc/2,source);
            } else {
                for (UInt j = 0; j < noc/2; j++) 
                    ((uint8_t *)mp)[j] = (uint8_t)random_uniform(q*q,source);
            }
            if (noc % 2) {
                mp[ds.nob-1] = (uint8_t)random_uniform(q,source);
            }
            break;
        case 5:
            for (UInt j = 0; j < noc/3; j++) 
                ((uint8_t *)mp)[j] = (uint8_t)random_uniform(125,source);
            switch (noc %3) {
            case 0:
                break;
            case 1:
                mp[ds.nob-1] = (uint8_t)random_uniform(5,source);
                break;
            case 2:
                mp[ds.nob-1] = (uint8_t)random_uniform(25,source);
                break;
            }
            break;
        case 6:
            random_buf(mp, noc/4,source);
            if (noc % 4) {
                mp[ds.nob-1] = (uint8_t)random_uniform(1<< 2*(noc % 4),source);
            }
            break;
        case 7:
            for (UInt j = 0; j < noc/5; j++) 
                ((uint8_t *)mp)[j] = (uint8_t)random_uniform(243,source);
            switch (noc %5) {
            case 0:
                break;
            case 1:
                mp[ds.nob-1] = (uint8_t)random_uniform(3,source);
                break;
            case 2:
                mp[ds.nob-1] = (uint8_t)random_uniform(9,source);
                break;
            case 3:
                mp[ds.nob-1] = (uint8_t)random_uniform(27,source);
                break;
            case 4:
                mp[ds.nob-1] = (uint8_t)random_uniform(81,source);
                break;
            }
            break;
        case 8:
            random_buf(mp, noc/8,source);
            if (noc % 8) {
                mp[ds.nob-1] = (uint8_t)random_uniform(1<< (noc % 8),source);
            }
            break;
        }
        mp = DPAdv(&ds,1,mp);
    }
    return m;
}

Obj FuncMTX64_HashMatrix(Obj self, Obj m) {
    GAP_ASSERT(IS_MTX64_Matrix(m)); // method selection should ensure
    Obj f = FieldOfMatrix(m);
    UInt noc = HeaderOfMatrix(m)->noc;
    UInt nor = HeaderOfMatrix(m)->nor;
    return HashValueToObjInt(HASHKEY_MEM_NC((const UChar *)DataOfMTX64_Matrix(m),
                                            1, Size_Data_Matrix(f, nor, noc)));
}


/* Obj FuncMTX64_CharPoly(Obj self, Obj m) { */
/*     CHECK_MTX64_Matrix(m); */
/*     UInt n = HeaderOfMatrix(m)->noc; */
/*     if (n != HeaderOfMatrix(m)->nor) */
/*         ErrorMayQuit("MTX64_CharPoly: non-square matrix"); */
/*     Obj fld = MTX64_FieldOfMatrix(m); */
/*     DSPACE dsp; */
/*     DSSet(DataOfFieldObject(fld), 2, &dsp); */


/*     Dfmt **polystart = malloc(sizeof(Dfmt *)*n); // pointers to matrix starts */
/*     DSPACE ds; */
/*     SetDSpaceOfMTX64_Matrix(m, &ds); */
/*     UInt npol = Dcp(&ds, DataOfMTX64_Matrix(m), polys, polystart); */
/*     //  */
    
    

// Table of functions to export
static StructGVarFunc GVarFuncs[] = {
    GVAR_FUNC(MTX64_CREATE_FIELD, 1, "q"),

    GVAR_FUNC(MTX64_FieldOrder, 1, "f"),
    GVAR_FUNC(MTX64_FieldCharacteristic, 1, "f"),
    GVAR_FUNC(MTX64_FieldDegree, 1, "f"),

    GVAR_FUNC(MTX64_CreateFieldElement, 2, "f,x"),
    GVAR_FUNC(MTX64_ExtractFieldElement, 1, "e"),

    GVAR_FUNC(MTX64_FieldAdd, 3, "f,a,b"),
    GVAR_FUNC(MTX64_FieldNeg, 2, "f,a"),
    GVAR_FUNC(MTX64_FieldSub, 3, "f,a,b"),
    GVAR_FUNC(MTX64_FieldMul, 3, "f,a,b"),
    GVAR_FUNC(MTX64_FieldInv, 2, "f,a"),
    GVAR_FUNC(MTX64_FieldDiv, 3, "f,a,b"),

    GVAR_FUNC(MTX64_NewMatrix, 3, "f,nor,noc"),
    GVAR_FUNC(MTX64_NumRows, 1, "m"),
    GVAR_FUNC(MTX64_NumCols, 1, "m"),
    GVAR_FUNC(MTX64_GetEntry, 3, "m,i,j"),
    GVAR_FUNC(MTX64_SetEntry, 4, "m,i,j,x"),

    GVAR_FUNC(MTX64_DCpy, 4, "src,dst,startrow,nrows"),
    GVAR_FUNC(MTX64_DCut, 5, "m,startrow,nrows,startcol,clip"),
    GVAR_FUNC(MTX64_DPaste, 5, "clip,startrow,nrows,startcol,m"),
    GVAR_FUNC(MTX64_DAdd, 3, "nrows,d1,d2"),
    GVAR_FUNC(MTX64_DSub, 3, "nrows,d1,d2"),
    GVAR_FUNC(MTX64_DSMad, 4, "nrows,scalar,d1,d2"),
    GVAR_FUNC(MTX64_DSMul, 3, "nrows,scalar,d1"),
    GVAR_FUNC(MTX64_DNzl, 2, "m, row"),

    GVAR_FUNC(MTX64_ShallowCopyMatrix, 1, "m"),
    GVAR_FUNC(MTX64_CompareMatrices, 2, "m1, m2"),
    GVAR_FUNC(MTX64_RANDOM_MAT, 4, "f, nor, noc, state"),

    GVAR_FUNC(MTX64_MakeFELTfromFFETable, 1, "q"),
    GVAR_FUNC(MTX64_InsertVecFFE, 3, "d, v, row"),
    GVAR_FUNC(MTX64_ExtractVecFFE, 2, "d, row"),
    GVAR_FUNC(MTX64_InsertVecGF2, 3, "d, v, row"),
    GVAR_FUNC(MTX64_ExtractVecGF2, 2, "d, row"),
    GVAR_FUNC(MTX64_Make8BitConversion, 1, "f"),
    GVAR_FUNC(MTX64_InsertVec8Bit, 3, "d, v, row"),
    GVAR_FUNC(MTX64_ExtractVec8Bit, 2, "d, row"),

    GVAR_FUNC(MTX64_WriteMatrix, 2, "m, fn"),
    GVAR_FUNC(MTX64_ReadMatrix, 1, "fn"),
    GVAR_FUNC(MTX64_HashMatrix, 1, "m" ),

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
  ImportFuncFromLibrary("MTX64_FieldEltType", &TYPE_MTX64_Felt);
  ImportFuncFromLibrary("MTX64_MatrixType", &TYPE_MTX64_Matrix);
  ImportFuncFromLibrary("IsMTX64FiniteField", &IsMTX64FiniteField);
  ImportFuncFromLibrary("IsMTX64Matrix", &IsMTX64Matrix);
  ImportFuncFromLibrary("IsMTX64FiniteFieldElement",
                        &IsMTX64FiniteFieldElement);

  // Access via family
  ImportFuncFromLibrary("MTX64_FieldOfMatrix", &MTX64_FieldOfMatrix);
  ImportFuncFromLibrary("MTX64_FieldOfElement", &MTX64_FieldOfElement);

  // Construct various lookup tables
  ImportFuncFromLibrary("MTX64_GetFFEfromFELTTable",
                        &MTX64_GetFFEfromFELTTable);
  ImportFuncFromLibrary("MTX64_GetFELTfromFFETable",
                        &MTX64_GetFELTfromFFETable);
  ImportFuncFromLibrary("MTX64_Get8BitImportTable", &MTX64_Get8BitImportTable);
  ImportFuncFromLibrary("MTX64_Get8BitExportTable", &MTX64_Get8BitExportTable);

  // Construct fields, respecting caching
  ImportFuncFromLibrary("MTX64_FiniteField", &MTX64_FiniteField);
  return InitBitString.initKernel(&InitBitString) ||
      InitSlab.initKernel(&InitSlab) || 
      InitFunctions.initKernel(&InitFunctions);
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary(StructInitInfo *module) {
  /* init filters and functions */
  InitGVarFuncsFromTable(GVarFuncs);
  return InitBitString.initLibrary(&InitBitString) || 
      InitSlab.initLibrary(&InitSlab) || 
      InitFunctions.initLibrary(&InitFunctions);
}

/******************************************************************************
*F  InitInfopl()  . . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
    .type = MODULE_DYNAMIC,
    .name = "meataxe64",
    .initKernel = InitKernel,
    .initLibrary = InitLibrary,
};

StructInitInfo *Init__Dynamic(void) { return &module; }
