#include "bitstring.h"
#include "src/bits_intern.h" // workaround
static Obj
    TYPE_MTX64_BitString; // global variable, type of MTX64 Bitstring objects
static Obj IsMTX64BitString;

Obj MTX64_MakeBitString(UInt len)
{
  Obj bs = NewBag(T_DATOBJ, Size_Bag_BitString(len));
  SET_TYPE_DATOBJ(bs, TYPE_MTX64_BitString);
  return bs;
}

UInt IS_MTX64_BitString(Obj bs) {
  return (IS_DATOBJ(bs) && DoFilter(IsMTX64BitString, bs));
}

// GAP Callable Bitstring operations

static Obj FuncMTX64_LengthOfBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  return INTOBJ_INT(DataOfBitStringObject(bs)[0]);
}

static Obj FuncMTX64_WeightOfBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  return INTOBJ_INT(DataOfBitStringObject(bs)[1]);
}

static Obj FuncMTX64_GetEntryOfBitString(Obj self, Obj bs, Obj pos) {
  CHECK_MTX64_BitString(bs);
  if (!IS_INTOBJ(pos) || INT_INTOBJ(pos) < 0 ||
      INT_INTOBJ(pos) >= DataOfBitStringObject(bs)[0])
    ErrorMayQuit(
        "MTX64_GetEntryOfBitString: position not an integer or out of range", 0,
        0);
  return INTOBJ_INT(BSBitRead(DataOfBitStringObject(bs), INT_INTOBJ(pos)));
}

static Obj FuncMTX64_ComplementBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  UInt len = DataOfBitStringObject(bs)[0];
  UInt wt = DataOfBitStringObject(bs)[1];
  Obj c = MTX64_MakeBitString(len);
  DataOfBitStringObject(c)[0] = len;
  if (wt < len) {
    uint64_t *bsp = DataOfBitStringObject(bs) + 2;
    uint64_t *cp = DataOfBitStringObject(c) + 2;
    for (UInt i = 0; i < len / 64; i++)
      *cp++ = ~*bsp++;
    if (len % 64) {
        UInt mask = (((UInt)1) << (len % 64)) - 1;
      *cp = (~*bsp) & mask;
    }
    DataOfBitStringObject(c)[1] = len - wt;
  }
  return c;
}

static Obj FuncMTX64_PositionsBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  UInt len = DataOfBitStringObject(bs)[0];
  UInt wt = DataOfBitStringObject(bs)[1];
  if (wt == 0) {
    Obj res = NEW_PLIST(T_PLIST_EMPTY, 0);
    SET_LEN_PLIST(res, 0);
    return res;
  }
  Obj res = NEW_PLIST(T_PLIST_CYC_SSORT, wt);
  UInt i = 1;
  uint64_t *bsp = DataOfBitStringObject(bs);
  for (UInt j = 0; j < len; j++) {
    if (BSBitRead(bsp, j)) {
      SET_ELM_PLIST(res, i++, INTOBJ_INT(j + 1));
    }
  }
  SET_LEN_PLIST(res, wt);
  return res;
}

// Unlike the meataxe64 library function, we update the weight of our bitstring

static Obj FuncMTX64_EmptyBitString(Obj self, Obj len) {
  CHECK_NONNEG_SMALLINT(len);
  Obj bs = MTX64_MakeBitString(INT_INTOBJ(len));
  DataOfBitStringObject(bs)[0] = INT_INTOBJ(len);
  DataOfBitStringObject(bs)[1] = 0;
  return bs;
}

static Obj FuncMTX64_SetEntryOfBitString(Obj self, Obj bs, Obj pos) {
  CHECK_MTX64_BitString(bs);
  CHECK_MUT(bs);
  CHECK_NONNEG_SMALLINT(pos);
  if (INT_INTOBJ(pos) >= DataOfBitStringObject(bs)[0])
    ErrorMayQuit(
        "MTX64_SetEntryOfBitString: position not an integer or out of range", 0,
        0);
  uint64_t *d = DataOfBitStringObject(bs);
  UInt ipos = INT_INTOBJ(pos);
  UInt x = BSBitRead(d, ipos);
  BSBitSet(d, ipos);
  if (!x)
    d[1]++; // maintain weight
  return 0;
}

static Obj FuncMTX64_BSCombine(Obj self, Obj bs1, Obj bs2) {
  CHECK_MTX64_BitString(bs1);
  CHECK_MTX64_BitString(bs2);
  UInt len1 = DataOfBitStringObject(bs1)[0];
  UInt len2 = DataOfBitStringObject(bs2)[0];
  UInt wt1 = DataOfBitStringObject(bs1)[1];
  UInt wt2 = DataOfBitStringObject(bs2)[1];
  if (len2 != len1 - wt1)
    ErrorMayQuit("MTX64_BSCombine: bitstrings incompatible", 0, 0);
  Obj comb = MTX64_MakeBitString(len1);
  Obj rif = MTX64_MakeBitString(wt1 + wt2);
  BSCombine(DataOfBitStringObject(bs1), DataOfBitStringObject(bs2),
            DataOfBitStringObject(comb), DataOfBitStringObject(rif));
  Obj ret = NEW_PLIST(T_PLIST_HOM + IMMUTABLE, 2);
  SET_ELM_PLIST(ret, 1, comb);
  SET_ELM_PLIST(ret, 2, rif);
  SET_LEN_PLIST(ret, 2);
  CHANGED_BAG(ret);
  return ret;
}


// Bitstring and matrix functions

static Obj FuncMTX64_ColSelect(Obj self, Obj bitstring, Obj m) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched row length", 0, 0);
  UInt nos = DataOfBitStringObject(bitstring)[1];
  Obj fld = FieldOfMatrix(m);
  Obj sel = NEW_MTX64_Matrix(fld, nor, nos);
  Obj nonsel = NEW_MTX64_Matrix(fld, nor, noc - nos);
  uint64_t *bs = DataOfBitStringObject(bitstring);
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

static Obj FuncMTX64_RowSelectShifted(Obj self, Obj bitstring, Obj m,
                                      Obj shift) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  CHECK_MTX64_Row(shift, m);
  UInt ishift = INT_INTOBJ(shift);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (nor != ishift + DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched matrix length: matrix %i, bitstring + shift %i",
                 nor, ishift + DataOfBitStringObject(bitstring)[0]);
  UInt nos = DataOfBitStringObject(bitstring)[1];
  Obj fld = FieldOfMatrix(m);
  Obj sel = NEW_MTX64_Matrix(fld, nos, noc);
  Obj nonsel = NEW_MTX64_Matrix(fld, nor - ishift - nos, noc);
  if (noc != 0) {
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    uint64_t *bs = DataOfBitStringObject(bitstring);
    Dfmt *d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, ishift, d);
    Dfmt *selp = DataOfMTX64_Matrix(sel);
    Dfmt *nonselp = DataOfMTX64_Matrix(nonsel);
    for (UInt i = 0; i < nor - ishift; i++) {
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

static Obj FuncMTX64_RowCombine(Obj self, Obj bitstring, Obj m1, Obj m2) {
  CHECK_MTX64_Matrices(m1, m2, 1);
  CHECK_MTX64_BitString(bitstring);
  UInt noc = HeaderOfMatrix(m1)->noc;
  UInt nor1 = HeaderOfMatrix(m1)->nor;
  UInt nor2 = HeaderOfMatrix(m2)->nor;
  UInt len = DataOfBitStringObject(bitstring)[0];
  UInt wt = DataOfBitStringObject(bitstring)[1];
  Obj f = FieldOfMatrix(m1);
  if (nor1 != wt || nor2 != len - wt)
    ErrorMayQuit("MTX64_RowCombine: matrices and bitstring don't match", 0, 0);
  Obj m = NEW_MTX64_Matrix(f, len, noc);
  DSPACE ds;
  SetDSpaceOfMTX64_Matrix(m, &ds);
  Dfmt *d1 = DataOfMTX64_Matrix(m1);
  Dfmt *d2 = DataOfMTX64_Matrix(m2);
  Dfmt *d = DataOfMTX64_Matrix(m);
  uint64_t *bs = DataOfBitStringObject(bitstring);
  for (UInt i = 0; i < len; i++) {
    if (BSBitRead(bs, i)) {
      memcpy(d, d1, ds.nob);
      d1 = DPAdv(&ds, 1, d1);
    } else {
      memcpy(d, d2, ds.nob);
      d2 = DPAdv(&ds, 1, d2);
    }
    d = DPAdv(&ds, 1, d);
  }
  return m;
}

static Obj FuncMTX64_BSColRifZ(Obj self, Obj bitstring, Obj m) {
  CHECK_MTX64_Matrix(m);
  CHECK_MTX64_BitString(bitstring);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[1])
    ErrorMayQuit("mismatched row length", 0, 0);
  UInt noc2 = DataOfBitStringObject(bitstring)[0];
  Obj fld = FieldOfMatrix(m);
  Obj out = NEW_MTX64_Matrix(fld, nor, noc2);
  uint64_t *bs = DataOfBitStringObject(bitstring);
  Dfmt *d = DataOfMTX64_Matrix(m);
  Dfmt *outp = DataOfMTX64_Matrix(out);
  BSColRifZ(DataOfFieldObject(fld), bs, nor, d, outp);
  return out;
}

static Obj FuncMTX64_BSColPutS(Obj self, Obj bitstring, Obj m, Obj x) {
  CHECK_MTX64_Matrix(m);
  CHECK_MUT(m);
  CHECK_MTX64_BitString(bitstring);
  CHECK_MTX64_MATRIX_FELT(m, x);
  UInt nor = HeaderOfMatrix(m)->nor;
  UInt noc = HeaderOfMatrix(m)->noc;
  if (noc != DataOfBitStringObject(bitstring)[0])
    ErrorMayQuit("mismatched row length", 0, 0);
  Obj fld = FieldOfMatrix(m);
  uint64_t *bs = DataOfBitStringObject(bitstring);
  Dfmt *d = DataOfMTX64_Matrix(m);
  FELT f = GetFELTFromFELTObject(x);
  BSColPutS(DataOfFieldObject(fld), bs, nor, f, d);
  return 0;
}

static void RecountBS(Obj bs) {
    uint64_t *d = DataOfBitStringObject(bs);
    d[1] = COUNT_TRUES_BLOCKS((UInt *)(d + 2), (d[0] + 63) / 64);
}

static Obj FuncMTX64_BSShiftOr(Obj self, Obj bs1, Obj shift, Obj bs2) {
  CHECK_MTX64_BitString(bs1);
  CHECK_MTX64_BitString(bs2);
  CHECK_MUT(bs2);
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

static Obj FuncMTX64_ShallowCopyBitString(Obj self, Obj bs) {
  GAP_ASSERT(IS_MTX64_BitString(bs)); // method selection should ensure
  UInt len = DataOfBitStringObject(bs)[0];
  Obj copy = MTX64_MakeBitString(len);
  memcpy(DataOfBitStringObject(copy), DataOfBitStringObject(bs),
         Size_Data_BitString(len));
  return copy;
}

// Order may not be consistent with GAP lists. Differing lengths are OK because
// the length field at the beginning is compared first
static Obj FuncMTX64_compareBitStrings(Obj self, Obj bs1, Obj bs2) {
  CHECK_MTX64_BitString(bs1);
  CHECK_MTX64_BitString(bs2);
  UInt len = DataOfBitStringObject(bs1)[0];
  Int res = memcmp(DataOfBitStringObject(bs1), DataOfBitStringObject(bs2),
                   Size_Data_BitString(len));
  return INTOBJ_INT((res < 0) ? -1 : (res > 0) ? 1 : 0);
}

static Obj FuncMTX64_BlistBitString(Obj self, Obj bs) {
  CHECK_MTX64_BitString(bs);
  UInt len = DataOfBitStringObject(bs)[0];

  Obj bl = NewBag(T_BLIST, SIZE_PLEN_BLIST(len));
  memcpy(BLOCKS_BLIST(bl), DataOfBitStringObject(bs) + 2,
         Size_Bits_BitString(len));
  SET_LEN_BLIST(bl, len);
  return bl;
}
         
static Obj FuncMTX64_BitStringBlist(Obj self, Obj bl) {
    ConvBlist(bl);
    UInt len = LEN_BLIST(bl);
    Obj bs = MTX64_MakeBitString(len);
    DataOfBitStringObject(bs)[0] = len;
    memcpy(DataOfBitStringObject(bs) + 2, CONST_BLOCKS_BLIST(bl),
         Size_Bits_BitString(len));
    RecountBS(bs);
    return bs;
}


// Table of functions to export
static StructGVarFunc GVarFuncs[] = {
    GVAR_FUNC(MTX64_BitStringBlist, 1, "bl"),
    GVAR_FUNC(MTX64_BlistBitString, 1, "bs"),
    GVAR_FUNC(MTX64_LengthOfBitString, 1, "bs"),
    GVAR_FUNC(MTX64_WeightOfBitString, 1, "bs"),
    GVAR_FUNC(MTX64_SetEntryOfBitString, 2, "bs, pos"),
    GVAR_FUNC(MTX64_GetEntryOfBitString, 2, "bs, pos"),
    GVAR_FUNC(MTX64_EmptyBitString, 1, "len"),
    GVAR_FUNC(MTX64_BSShiftOr, 3, "bs1, shift, bs2"),
    GVAR_FUNC(MTX64_compareBitStrings, 2, "bs1, bs2"),
    GVAR_FUNC(MTX64_ColSelect, 2, "bs, m"),
    GVAR_FUNC(MTX64_RowSelectShifted, 3, "bs, m, shift"),
    GVAR_FUNC(MTX64_RowCombine, 3, "bs, m1, m2"),
    GVAR_FUNC(MTX64_BSColRifZ, 2, "bs, m"),
    GVAR_FUNC(MTX64_BSColPutS, 3, "bs, m, x"),
    GVAR_FUNC(MTX64_BSCombine, 2, "bs1, bs2"),
    GVAR_FUNC(MTX64_PositionsBitString, 1, "bs"),
    GVAR_FUNC(MTX64_ComplementBitString, 1, "bs"),
    GVAR_FUNC(MTX64_ShallowCopyBitString, 1, "bs"),


    {0} /* Finish with an empty entry */
};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel(StructInitInfo *module) {
  /* init filters and functionsi */
  InitHdlrFuncsFromTable(GVarFuncs);
  ImportGVarFromLibrary("MTX64_BitStringType", &TYPE_MTX64_BitString);
  ImportFuncFromLibrary("IsMTX64BitString", &IsMTX64BitString);


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
 StructInitInfo InitBitString = {
    .type = MODULE_DYNAMIC,
    .name = "mtx64bitstring",
    .initKernel = InitKernel,
    .initLibrary = InitLibrary,
};
