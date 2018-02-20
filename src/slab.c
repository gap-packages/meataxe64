
// Slab functions -- slightly higher level matrix operations
//  use HPMI
#include "slab.h"

// A utlity for cleaning up return matrices.
static void SetShapeAndResize(Obj mat, UInt nor, UInt noc) {
  Matrix_Header *h = HeaderOfMatrix(mat);
  Obj f = FieldOfMatrix(mat);
  h->nor = nor;
  h->noc = noc;
  ResizeBag(mat, Size_Bag_Matrix(f, noc, nor));
}

//
// Slab echelize. Currently destroys its argument.
// Richard plans to develop this further.
//
static Obj FuncMTX64_SLEchelizeDestructive(Obj self, Obj a) {
  CHECK_MTX64_Matrix(a);
  CHECK_MUT(a);
  Matrix_Header *h = HeaderOfMatrix(a);
  UInt nrows = h->nor;
  UInt ncols = h->noc;
  UInt rklimit = (nrows > ncols) ? ncols : nrows; // minimum
  Obj rs = MTX64_MakeBitString(nrows);
  Obj cs = MTX64_MakeBitString(ncols);
  FELT det;
  Obj field = FieldOfMatrix(a);
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
  // make return record
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

static Obj FuncMTX64_SLMultiply(Obj self, Obj a, Obj b) {
  CHECK_MTX64_Matrices(a, b, 0);
  Matrix_Header *ha = HeaderOfMatrix(a);
  Matrix_Header *hb = HeaderOfMatrix(b);
  UInt nora = ha->nor;
  UInt noca = ha->noc;
  UInt nocb = hb->noc;
  if (noca != hb->nor)
    ErrorMayQuit("SLMultiply: matrices are incompatible shapes", 0, 0);
  Obj field = FieldOfMatrix(a);
  Obj c = NEW_MTX64_Matrix(field, nora, nocb);
  FIELD *f = DataOfFieldObject(field);
  Dfmt *ap = DataOfMTX64_Matrix(a);
  Dfmt *bp = DataOfMTX64_Matrix(b);
  Dfmt *cp = DataOfMTX64_Matrix(c);
  SLMul(f, ap, bp, cp, nora, noca, nocb);
  return c;
}

static void allocateAndChop(Obj m, Obj parts[2][2], UInt n2, UInt n2a,
                            UInt m2, Obj field) {
  parts[0][0] = NEW_MTX64_Matrix(field, n2, m2);
  parts[0][1] = NEW_MTX64_Matrix(field, n2, m2);
  parts[1][0] = NEW_MTX64_Matrix(field, n2, m2);
  parts[1][1] = NEW_MTX64_Matrix(field, n2, m2);
  DSPACE ds;
  DSPACE ds2;
  SetDSpaceOfMTX64_Matrix(m, &ds);
  SetDSpaceOfMTX64_Matrix(parts[0][0], &ds2);
  Dfmt *mp = DataOfMTX64_Matrix(m);
  Dfmt *m11p = DataOfMTX64_Matrix(parts[0][0]);
  Dfmt *m12p = DataOfMTX64_Matrix(parts[0][1]);
  Dfmt *m21p = DataOfMTX64_Matrix(parts[1][0]);
  Dfmt *m22p = DataOfMTX64_Matrix(parts[1][1]);
  DCut(&ds, n2, 0, mp, &ds2, m11p);
  DCut(&ds, n2, m2, mp, &ds2, m12p);
  mp = DPAdv(&ds, n2, mp);
  DCut(&ds, n2a, 0, mp, &ds2, m21p);
  DCut(&ds, n2a, m2, mp, &ds2, m22p);
  return;
}

static void addForStrassen(Obj x, Obj y, Obj s, UInt n) {
  Dfmt *xp = DataOfMTX64_Matrix(x);
  Dfmt *yp = DataOfMTX64_Matrix(y);
  Dfmt *sp = DataOfMTX64_Matrix(s);
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(x, &ds);
  DAdd(&ds, n, xp, yp, sp);
  return;
}

static void subForStrassen(Obj x, Obj y, Obj s, UInt n) {
  Dfmt *xp = DataOfMTX64_Matrix(x);
  Dfmt *yp = DataOfMTX64_Matrix(y);
  Dfmt *sp = DataOfMTX64_Matrix(s);
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(x, &ds);
  DSub(&ds, n, xp, yp, sp);
  return;
}

static UInt splitPoint(Obj field, UInt n) {
  /*     UInt cauldron = DataOfFieldObject(field)->cauldron;
  UInt alcove = DataOfFieldObject(field)->alcove;
  UInt grid = cauldron;
  while (grid % alcove)
      grid += cauldron;
      return  (((n/2) + grid -1)/grid)*grid; */
  return (n + 1) / 2;
}

static Obj SLMultiplyStrassenSquare(Obj a, Obj b, Obj c, Obj field, UInt n,
                                    UInt reclevel) {
  if (!reclevel) {
    // base case
    if (!c)
      c = NEW_MTX64_Matrix(field, n, n);
    Dfmt *ap = DataOfMTX64_Matrix(a);
    Dfmt *bp = DataOfMTX64_Matrix(b);
    Dfmt *cp = DataOfMTX64_Matrix(c);
    SLMul(DataOfFieldObject(field), ap, bp, cp, n, n, n);
    return c;
  }
  UInt n2 = splitPoint(field, n);
  UInt n2a = n - n2;
  if (n2 == 0 || n2a == 0) {
    return SLMultiplyStrassenSquare(a, b, c, field, n, 0);
  }

  // chop up a
  Obj aparts[2][2];
  allocateAndChop(a, aparts, n2, n2a, n2, field);
  a = 0; // allow GC to collect a

  // chop up b
  Obj bparts[2][2];
  allocateAndChop(b, bparts, n2, n2a, n2, field);
  b = 0; // allow GC to collect a

  // allocate chopped sections of c
  Obj cparts[2][2];
  cparts[0][0] = NEW_MTX64_Matrix(field, n2, n2);
  cparts[0][1] = NEW_MTX64_Matrix(field, n2, n2);
  cparts[1][0] = NEW_MTX64_Matrix(field, n2, n2);
  cparts[1][1] = NEW_MTX64_Matrix(field, n2, n2);

  // do the work
  subForStrassen(aparts[0][0], aparts[1][0], cparts[0][0], n2);
  addForStrassen(aparts[1][0], aparts[1][1], aparts[1][0], n2);
  subForStrassen(bparts[0][1], bparts[0][0], cparts[1][1], n2);
  subForStrassen(bparts[1][1], bparts[0][1], bparts[0][1], n2);
  SLMultiplyStrassenSquare(cparts[0][0], bparts[0][1], cparts[1][0], field, n2,
                           reclevel - 1);
  subForStrassen(aparts[1][0], aparts[0][0], cparts[0][1], n2);
  SLMultiplyStrassenSquare(aparts[0][0], bparts[0][0], cparts[0][0], field, n2,
                           reclevel - 1);
  subForStrassen(bparts[1][1], cparts[1][1], bparts[0][0], n2);
  SLMultiplyStrassenSquare(aparts[1][0], cparts[1][1], aparts[0][0], field, n2,
                           reclevel - 1);
  subForStrassen(bparts[0][0], bparts[1][0], cparts[1][1], n2);
  SLMultiplyStrassenSquare(aparts[1][1], cparts[1][1], aparts[1][0], field, n2,
                           reclevel - 1);
  subForStrassen(aparts[0][1], cparts[0][1], aparts[1][1], n2);
  SLMultiplyStrassenSquare(cparts[0][1], bparts[0][0], cparts[1][1], field, n2,
                           reclevel - 1);
  addForStrassen(cparts[0][0], cparts[1][1], cparts[1][1], n2);
  SLMultiplyStrassenSquare(aparts[0][1], bparts[1][0], cparts[0][1], field, n2,
                           reclevel - 1);
  addForStrassen(cparts[0][0], cparts[0][1], cparts[0][0], n2);
  addForStrassen(cparts[1][1], aparts[0][0], cparts[0][1], n2);
  addForStrassen(cparts[1][1], cparts[1][0], cparts[1][1], n2a);
  subForStrassen(cparts[1][1], aparts[1][0], cparts[1][0], n2a);
  addForStrassen(cparts[1][1], aparts[0][0], cparts[1][1], n2a);
  SLMultiplyStrassenSquare(aparts[1][1], bparts[1][1], aparts[0][1], field, n2,
                           reclevel - 1);
  addForStrassen(cparts[0][1], aparts[0][1], cparts[0][1], n2);

  // release these objects before allocating c
  aparts[0][0] = 0;
  aparts[0][1] = 0;
  aparts[1][0] = 0;
  aparts[1][1] = 0;
  bparts[0][0] = 0;
  bparts[0][1] = 0;
  bparts[1][0] = 0;
  bparts[1][1] = 0;
  if (!c)
    c = NEW_MTX64_Matrix(field, n, n);

  // reassemble c
  DSPACE ds, ds2;
  SetDSpaceOfMTX64_Matrix(c, &ds);
  SetDSpaceOfMTX64_Matrix(cparts[0][0], &ds2);
  Dfmt *cp = DataOfMTX64_Matrix(c);
  Dfmt *c11p = DataOfMTX64_Matrix(cparts[0][0]);
  Dfmt *c12p = DataOfMTX64_Matrix(cparts[0][1]);
  Dfmt *c21p = DataOfMTX64_Matrix(cparts[1][0]);
  Dfmt *c22p = DataOfMTX64_Matrix(cparts[1][1]);
  DPaste(&ds2, c11p, n2, 0, &ds, cp);
  DPaste(&ds2, c12p, n2, n2, &ds, cp);
  cp = DPAdv(&ds, n2, cp);
  DPaste(&ds2, c21p, n2a, 0, &ds, cp);
  DPaste(&ds2, c22p, n2a, n2, &ds, cp);
  return c;
}

static Obj FuncMTX64_SLMultiplyStrassenSquare(Obj self, Obj a, Obj b,
                                              Obj level) {
  CHECK_MTX64_Matrices(a, b, 0);
  CHECK_NONNEG_SMALLINT(level);
  Matrix_Header *ha = HeaderOfMatrix(a);
  Matrix_Header *hb = HeaderOfMatrix(b);
  UInt n = ha->nor;
  if (n != ha->noc || n != hb->nor || n != hb->noc) {
    ErrorMayQuit("Matrices must be square", 0, 0);
  }
  Obj field = FieldOfMatrix(a);
  return SLMultiplyStrassenSquare(a, b, NULL, field, n, INT_INTOBJ(level));
}

// static Obj DisplayOp = 0;

static inline void Dis( Obj m, Char *str) {
    //    printf("\n%s\n",str);
    //    CALL_1ARGS(DisplayOp, m);
}

static Obj SLMultiplyStrassenNonSquare(Obj a, Obj b, Obj c, Obj field, UInt n,
                                       UInt m, UInt k, UInt reclevel) {
  if (!reclevel) {
    // base case
    if (!c)
      c = NEW_MTX64_Matrix(field, n, k);
    Dfmt *ap = DataOfMTX64_Matrix(a);
    Dfmt *bp = DataOfMTX64_Matrix(b);
    Dfmt *cp = DataOfMTX64_Matrix(c);
    SLMul(DataOfFieldObject(field), ap, bp, cp, n, m, k);
    return c;
  }
  UInt n2 = splitPoint(field, n);
  UInt n2a = n - n2;
  UInt m2 = splitPoint(field, m);
  UInt m2a = m - m2;
  UInt k2 = splitPoint(field, k);
  UInt k2a = k - k2;
  if (n2 == 0 || n2a == 0 || m2 == 0 || m2a == 0 || k2 == 0 || k2a == 0) {
      // printf("Too small to cut %lli %lli %lli\n", n, m, k);
    return SLMultiplyStrassenNonSquare(a, b, c, field, n, m, k, 0);
  }

  // printf("%lli %lli %lli %lli %lli %lli %lli %lli %lli\n",n,n2,n2a, m,m2,m2a, k,k2,k2a);
  // chop up a
  Obj aparts[2][2];
  allocateAndChop(a, aparts, n2, n2a, m2, field);
  a = 0; // allow GC to collect a

  Dis(aparts[0][0], "a00");
  Dis(aparts[0][1], "a01");
  Dis(aparts[1][0], "a10");
  Dis(aparts[1][1], "a11");

  // chop up b
  Obj bparts[2][2];
  allocateAndChop(b, bparts, m2, m2a, k2, field);
  b = 0; // allow GC to collect b

  Dis(bparts[0][0], "b00");
  Dis(bparts[0][1], "b01");
  Dis(bparts[1][0], "b10");
  Dis(bparts[1][1], "b11");

  // allocate chopped sections of c
  Obj cparts[2][2];
  cparts[0][0] = NEW_MTX64_Matrix(field, n2, k2);
  cparts[0][1] = NEW_MTX64_Matrix(field, n2, k2);
  cparts[1][0] = NEW_MTX64_Matrix(field, n2, k2);
  cparts[1][1] = NEW_MTX64_Matrix(field, n2, k2);

  // Allocate two temporaries
  // Following Pernet et al, but his m is our n, his k is our m and his n is our
  // k
  Obj x = NEW_MTX64_Matrix(field, n2, k2 > m2 ? k2 : m2);
  HeaderOfMatrix(x)->noc =
      m2; // width m2 for now, but with room to change to k2 later
  Obj y = NEW_MTX64_Matrix(field, m2, k2);

  // do the work
  subForStrassen(aparts[0][0], aparts[1][0], x, n2);
  Dis(x, "step 1");
  subForStrassen(bparts[1][1], bparts[0][1], y, m2);
  Dis(y, "step 2");
  SLMultiplyStrassenNonSquare(x, y, cparts[1][0], field, n2, m2, k2,
                              reclevel - 1);
  Dis(cparts[1][0], "step 3");
  addForStrassen(aparts[1][0], aparts[1][1], x, n2);
  Dis(x, "step 4");
  subForStrassen(bparts[0][1], bparts[0][0], y, m2);
  Dis(y, "step 5");
  SLMultiplyStrassenNonSquare(x, y, cparts[1][1], field, n2, m2, k2,
                              reclevel - 1);
  Dis(cparts[1][1], "step 6");
  subForStrassen(x, aparts[0][0], x, n2);
  Dis(x, "step 7");
  subForStrassen(bparts[1][1], y, y, m2);
  Dis(y, "step 8");
  SLMultiplyStrassenNonSquare(x, y, cparts[0][1], field, n2, m2, k2,
                              reclevel - 1);
  Dis(cparts[0][1], "step 9");
  subForStrassen(aparts[0][1], x, x, n2);
  Dis(x, "step 10");

  SLMultiplyStrassenNonSquare(x, bparts[1][1], cparts[0][0], field, n2, m2, k2,
                              reclevel - 1);
  Dis(cparts[0][0], "step 11");
  // change width of x
  HeaderOfMatrix(x)->noc = k2;
  SLMultiplyStrassenNonSquare(aparts[0][0], bparts[0][0], x, field, n2, m2, k2,
                              reclevel - 1);
  addForStrassen(cparts[0][1], x, cparts[0][1], n2);
  addForStrassen(cparts[0][1], cparts[1][0], cparts[1][0], n2);
  addForStrassen(cparts[0][1], cparts[1][1], cparts[0][1], n2);
  addForStrassen(cparts[1][0], cparts[1][1], cparts[1][1], n2a);
  addForStrassen(cparts[0][1], cparts[0][0], cparts[0][1], n2);
  subForStrassen(y, bparts[1][0], y, m2);
  SLMultiplyStrassenNonSquare(aparts[1][1], y, cparts[0][0], field, n2, m2, k2,
                              reclevel - 1);
  y = 0; // allow GC to reclaim y if it needs to
  subForStrassen(cparts[1][0], cparts[0][0], cparts[1][0], n2);
  SLMultiplyStrassenNonSquare(aparts[0][1], bparts[1][0], cparts[0][0], field,
                              n2, m2, k2, reclevel - 1);
  addForStrassen(x, cparts[0][0], cparts[0][0], n2);

  // release these objects before allocating c
  aparts[0][0] = 0;
  aparts[0][1] = 0;
  aparts[1][0] = 0;
  aparts[1][1] = 0;
  bparts[0][0] = 0;
  bparts[0][1] = 0;
  bparts[1][0] = 0;
  bparts[1][1] = 0;
  x = 0;
  if (!c)
    c = NEW_MTX64_Matrix(field, n, k);

  // reassemble c
  DSPACE ds, ds2;
  SetDSpaceOfMTX64_Matrix(c, &ds);
  SetDSpaceOfMTX64_Matrix(cparts[0][0], &ds2);
  Dfmt *cp = DataOfMTX64_Matrix(c);
  Dfmt *c11p = DataOfMTX64_Matrix(cparts[0][0]);
  Dfmt *c12p = DataOfMTX64_Matrix(cparts[0][1]);
  Dfmt *c21p = DataOfMTX64_Matrix(cparts[1][0]);
  Dfmt *c22p = DataOfMTX64_Matrix(cparts[1][1]);
  DPaste(&ds2, c11p, n2, 0, &ds, cp);
  DPaste(&ds2, c12p, n2, k2, &ds, cp);
  cp = DPAdv(&ds, n2, cp);
  DPaste(&ds2, c21p, n2a, 0, &ds, cp);
  DPaste(&ds2, c22p, n2a, k2, &ds, cp);
  return c;
}

static Obj FuncMTX64_SLMultiplyStrassenNonSquare(Obj self, Obj a, Obj b,
                                                 Obj level) {
  CHECK_MTX64_Matrices(a, b, 0);
  CHECK_NONNEG_SMALLINT(level);
  Matrix_Header *ha = HeaderOfMatrix(a);
  Matrix_Header *hb = HeaderOfMatrix(b);
  UInt n = ha->nor;
  UInt m = ha->noc;
  UInt k = hb->noc;
  if (m != hb->nor)
    ErrorMayQuit("Matrices Incompatible", 0, 0);
  Obj field = FieldOfMatrix(a);
  return SLMultiplyStrassenNonSquare(a, b, NULL, field, n, m, k,
                                     INT_INTOBJ(level));
}

static Obj FuncMTX64_SLTranspose(Obj self, Obj mat) {
  CHECK_MTX64_Matrix(mat);
  Matrix_Header *h = HeaderOfMatrix(mat);
  UInt nora = h->nor;
  UInt noca = h->noc;
  Obj field = FieldOfMatrix(mat);
  Obj tra = NEW_MTX64_Matrix(field, noca, nora);
  FIELD *f = DataOfFieldObject(field);
  Dfmt *mp = DataOfMTX64_Matrix(mat);
  Dfmt *tp = DataOfMTX64_Matrix(tra);
  SLTra(f, mp, tp, nora, noca);
  return tra;
}


// Table of functions to export
static StructGVarFunc GVarFuncs[] = {
    GVAR_FUNC(MTX64_SLMultiply, 2, "a, b"),
    GVAR_FUNC(MTX64_SLMultiplyStrassenSquare, 3, "a, b, level"),
    GVAR_FUNC(MTX64_SLMultiplyStrassenNonSquare, 3, "a, b, level"),
    GVAR_FUNC(MTX64_SLTranspose, 1, "m"),
    GVAR_FUNC(MTX64_SLEchelizeDestructive, 1, "a"),

    {0} /* Finish with an empty entry */
};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel(StructInitInfo *module) {
  /* init filters and functionsi */
  InitHdlrFuncsFromTable(GVarFuncs);


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
*F  InitFunctions  . . . . . . . . . . . . . . . . . table of init functions
*/
 StructInitInfo InitSlab = {
    .type = MODULE_DYNAMIC,
    .name = "mtx64slab",
    .initKernel = InitKernel,
    .initLibrary = InitLibrary,
};

