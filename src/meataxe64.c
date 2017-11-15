/*
 * meataxe64: meataxe64
 */

#include "src/compiled.h"          /* GAP headers */
#include "src/vecgf2.h"          /* GAP headers */
#include "src/vec8bit.h"          /* GAP headers */

#include <assert.h>
#include "mtx64/field.h"
#include "mtx64/slab.h"
#include "mtx64/bitstring.h"

/* The slab level interface with meataxe 64 works uses mainly
   interfaces defined in mtx64/field.h and mtx64/slab.h.  
   This defines four types of
   object of interest, a FIELD (a large structure), a FELT (a 64 bit
   value representing a field element, a DSpace (a small structure
   that gathers some information about a bunch of vectors) and a a
   DFmt object, which is a char pointer to a block of bytes which
   contain vectors. We create the DSpace on the fly when we need if
   we store the number of columns with the Dfmt data and also 
   an int for the number of rowns. We call this a MTX64_Matrix in GAP.

   We wrap FELTS (slightly inefficient, but we dont expect working
   with them to be performance critical), FIELDS, and blocks
   of Dfmt data in T_DATOBJ objects. We store a FIELD in the family of each MTX64_Matrix
   and FELT. 

   For the time being, we make no attempt to unite these objects with
   FFE and List types in GAP. They are entirely separate objects
   accessed only by their own functions.

   We might want to change this later and unite these with the
   MatrixObj development, or pay for another layer of wrapping.

   TODO -- consts, asserts and actual checks on arguments in appropriate places.
   TODO -- consistentise names
   TODO -- more comments
   
 */

static Obj TYPE_MTX64_Field;     // global variable. All FIELDS have same type 
static Obj TYPE_MTX64_Felt;      // function, takes field
static Obj TYPE_MTX64_Matrix;    // function,  takes field
static Obj TYPE_MTX64_BitString; // global variable, type of MTX64 Bitstring objects

static inline uint64_t *DataOfBitStringObject(Obj bs) {
    return (uint64_t *)(ADDR_OBJ(bs)+1);
}

static inline UInt Size_Data_BitString(UInt len) {
    return 2*sizeof(uint64_t) + 8*((len + 63)/64);
}

static inline UInt Size_Bag_BitString(UInt len) {
    return sizeof(Obj) + Size_Data_BitString(len);
}

static inline Obj MTX64_MakeBitString(UInt len) {
    Obj bs = NewBag(T_DATOBJ,Size_Bag_BitString(len) );
    SET_TYPE_DATOBJ(bs, TYPE_MTX64_BitString);
    return bs;
}

static inline FIELD * DataOfFieldObject( Obj f) {
    return (FIELD *)(ADDR_OBJ(f)+1);
}

static inline FELT GetFELTFromFELTObject (Obj f) {
    return *(FELT *) (ADDR_OBJ(f)+1);
}

static inline void SetFELTOfFELTObject (Obj f, FELT x) {
    *(FELT *)(ADDR_OBJ(f)+1) = x;
}

typedef struct {
    UInt noc;
    UInt nor;
} MTX64_Matrix_Header;

static inline MTX64_Matrix_Header *HeaderOfMTX64_Matrix (Obj mx) {
    return (MTX64_Matrix_Header *)(ADDR_OBJ(mx)+1);
}

static inline UInt Size_Data_Matrix(Obj f, UInt noc, UInt nor) {
    DSPACE ds;
    DSSet(DataOfFieldObject(f), noc, &ds);
    return ds.nob*nor;
}

static inline UInt Size_Bag_Matrix(Obj f, UInt noc, UInt nor) {
    return sizeof(Obj) + sizeof(MTX64_Matrix_Header) + Size_Data_Matrix(f,noc,nor) ;    
}

static inline Obj NEW_MTX64_Matrix(Obj f, UInt nor, UInt noc) {
    Obj m;
    m = NewBag(T_DATOBJ,Size_Bag_Matrix(f, noc, nor) );
    SET_TYPE_DATOBJ(m, CALL_1ARGS(TYPE_MTX64_Matrix,INTOBJ_INT(DataOfFieldObject(f)->fdef)));
    HeaderOfMTX64_Matrix(m)->noc = noc;
    HeaderOfMTX64_Matrix(m)->nor = nor;
    return m;
}

static inline Dfmt * DataOfMTX64_Matrix ( Obj m) {
    return (Dfmt *)(HeaderOfMTX64_Matrix(m) + 1);
}

static Obj MakeMtx64Field(UInt field_order) {
    Obj field = NewBag(T_DATOBJ, FIELDLEN + sizeof(Obj));
    SET_TYPE_DATOBJ(field, TYPE_MTX64_Field);
    FieldSet(field_order, DataOfFieldObject(field));
    return field;
}

/* probably all these should be static */

static Obj MTX64_CreateField(Obj self, Obj field_order) {
    return MakeMtx64Field(INT_INTOBJ(field_order));
}

static Obj MakeMtx64Felt(Obj field, FELT x) {
    Obj f = NewBag(T_DATOBJ, sizeof(FELT)+sizeof(Obj));
    UInt q = DataOfFieldObject(field)->fdef;
    Obj type = CALL_1ARGS(TYPE_MTX64_Felt,
                          INTOBJ_INT(q));
    SET_TYPE_DATOBJ(f,type );
    SetFELTOfFELTObject(f, x);
    return f;
}

static Obj FieldOfMTX64Matrix;

static inline void SetDSpaceOfMTX64_Matrix( Obj m, DSPACE *ds) {
    /* Only safe until next garbage collection */
    Obj field = CALL_1ARGS(FieldOfMTX64Matrix, m);
    DSSet(DataOfFieldObject(field), HeaderOfMTX64_Matrix(m)->noc, ds);
}


Obj MTX64_FieldOrder(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->fdef);
}

Obj MTX64_FieldCharacteristic(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->charc);
}

Obj MTX64_FieldDegree(Obj self, Obj field)
{
    FIELD * _f = DataOfFieldObject(field);
    return INTOBJ_INT(_f->pow);
}

Obj MTX64_CreateFieldElement(Obj self, Obj field, Obj elt)
{
    UInt ielt;
    if (TNUM_OBJ(elt) == T_INTPOS) {
        Obj x = MOD(elt, INTOBJ_INT(1L << 32));
        Obj y = QUO(elt, INTOBJ_INT(1L << 32));
        GAP_ASSERT(IS_INTOBJ(x));
        if (!IS_INTOBJ(y) || INT_INTOBJ(y) >= (1L << 32))
            ErrorMayQuit("MTX64_CreateFieldElement: Element too large",0L,0L);
        ielt = (INT_INTOBJ(y) << 32) + INT_INTOBJ(x);
    } else
        ielt = INT_INTOBJ(elt);
    // TODO Should check against field size as well 
    return MakeMtx64Felt(field, ielt);
}

Obj MTX64_ExtractFieldElement(Obj self, Obj elt)
{
    return ObjInt_UInt(GetFELTFromFELTObject(elt));
}

Obj MTX64_FieldAdd(Obj self, Obj f, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(f);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(f, FieldAdd(_f, _a, _b));
}

Obj MTX64_FieldNeg(Obj self, Obj field, Obj a)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);

    return MakeMtx64Felt(field, FieldNeg(_f, _a));
}

Obj MTX64_FieldSub(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldSub(_f, _a, _b));
}

Obj MTX64_FieldMul(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldMul(_f, _a, _b));
}

Obj MTX64_FieldInv(Obj self, Obj field, Obj a)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);

    return MakeMtx64Felt(field, FieldInv(_f, _a));
}

Obj MTX64_FieldDiv(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = DataOfFieldObject(field);
    FELT _a = GetFELTFromFELTObject(a);
    FELT _b = GetFELTFromFELTObject(b);

    return MakeMtx64Felt(field, FieldDiv(_f, _a, _b));
}

// Add GAP callable matrix constructors and inspectors

Obj MTX64_NewMatrix(Obj self, Obj field, Obj nor, Obj noc) {
    // checks, or at least asserts
    return NEW_MTX64_Matrix(field, INT_INTOBJ(nor), INT_INTOBJ(noc));
}

Obj MTX64_Matrix_NumRows(Obj self, Obj m) {
    // checks, or at least asserts
    return INTOBJ_INT(HeaderOfMTX64_Matrix(m)->nor);
}

Obj MTX64_Matrix_NumCols(Obj self, Obj m) {
    // checks, or at least asserts
    return INTOBJ_INT(HeaderOfMTX64_Matrix(m)->noc);
}

// 0 based adressing
static FELT GetEntryMTX64(Obj m, UInt row, UInt col)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    return DUnpak(&ds, col, d);
}

Obj MTX64_GetEntry(Obj self, Obj m, Obj row, Obj col)
{
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m);
    return MakeMtx64Felt(f,GetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row)));
}

void SetEntryMTX64(Obj m, UInt row, UInt col, FELT entry)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    DPak(&ds, col, d, entry);
}

Obj MTX64_SetEntry(Obj self, Obj m, Obj row, Obj col, Obj entry)
{
    SetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row),GetFELTFromFELTObject(entry));
    return 0;
}

Obj MTX64_DCpy(Obj self, Obj src, Obj dst, Obj startrow, Obj nrows)
{
    DSPACE ds;
    Dfmt * sp, *dp;
    SetDSpaceOfMTX64_Matrix(src, &ds);
    sp = DataOfMTX64_Matrix(src);
    dp = DataOfMTX64_Matrix(dst);
    dp = DPAdv(&ds, INT_INTOBJ(startrow), dp);
    DCpy(&ds, sp, INT_INTOBJ(nrows), dp);
    return 0;
}

Obj MTX64_DCut(Obj self, Obj m, Obj startrow, Obj nrows, Obj startcol, Obj clip ) 
{
    DSPACE ms, cs;
    Dfmt * mp, *cp;
    SetDSpaceOfMTX64_Matrix(m, &ms);
    SetDSpaceOfMTX64_Matrix(clip, &cs);
    mp = DataOfMTX64_Matrix(m);
    mp = DPAdv(&ms, INT_INTOBJ(startrow), mp);
    cp = DataOfMTX64_Matrix(clip);
    DCut(&ms, INT_INTOBJ(nrows), INT_INTOBJ(startcol),
         mp, &cs, cp);
    return 0;
}

Obj MTX64_DPaste(Obj self, Obj clip, Obj startrow, Obj nrows, Obj startcol, Obj m)
{
    DSPACE ms, cs;
    Dfmt * mp, *cp;
    SetDSpaceOfMTX64_Matrix(m, &ms);
    SetDSpaceOfMTX64_Matrix(clip, &cs);
    mp = DataOfMTX64_Matrix(m);
    mp = DPAdv(&ms, INT_INTOBJ(startrow), mp);
    cp = DataOfMTX64_Matrix(clip);
    DPaste(&cs, cp, INT_INTOBJ(nrows), INT_INTOBJ(startcol), &ms, mp );
    return 0;
    
}

Obj MTX64_DAdd(Obj self, Obj nrows, Obj d1, Obj d2, Obj d)
{
    DSPACE ds;
    Dfmt * d1p, *d2p, *dp;
    SetDSpaceOfMTX64_Matrix(d1, &ds);    
    d1p = DataOfMTX64_Matrix(d1);
    d2p = DataOfMTX64_Matrix(d2);
    dp = DataOfMTX64_Matrix(d);
    DAdd( &ds, INT_INTOBJ(nrows), d1p, d2p, dp);
    return 0;
}

Obj MTX64_DSub(Obj self, Obj nrows, Obj d1, Obj d2, Obj d)
{
    DSPACE ds;
    Dfmt * d1p, *d2p, *dp;
    SetDSpaceOfMTX64_Matrix(d1, &ds);    
    d1p = DataOfMTX64_Matrix(d1);
    d2p = DataOfMTX64_Matrix(d2);
    dp = DataOfMTX64_Matrix(d);
    DSub( &ds, INT_INTOBJ(nrows), d1p, d2p, dp);
    return 0;
}


Obj MTX64_DSMad(Obj self, Obj nrows, Obj scalar, Obj d1, Obj d2)
{
    DSPACE ds;
    Dfmt * d1p, *d2p;
    FELT x;
    SetDSpaceOfMTX64_Matrix(d1, &ds);    
    d1p = DataOfMTX64_Matrix(d1);
    d2p = DataOfMTX64_Matrix(d2);
    x = GetFELTFromFELTObject(scalar);
    DSMad( &ds, x, INT_INTOBJ(nrows), d1p, d2p);
    return 0;
}

// In place?
Obj MTX64_DSMul(Obj self, Obj nrows, Obj scalar, Obj d1)
{
    DSPACE ds;
    Dfmt *dp;
    FELT x;
    SetDSpaceOfMTX64_Matrix(d1, &ds);    
    dp = DataOfMTX64_Matrix(d1);
    x = GetFELTFromFELTObject(scalar);
    DSMul( &ds, x, INT_INTOBJ(nrows), dp);
    return 0;    
}

Obj MTX64_DNzl(Obj self, Obj m)
{
    DSPACE ds;
    Dfmt *mp;
    SetDSpaceOfMTX64_Matrix(m, &ds);    
    mp = DataOfMTX64_Matrix(m);
    return INTOBJ_INT(DNzl(&ds,mp));
}

void SetShapeAndResize(Obj mat, UInt nor, UInt noc) {
    MTX64_Matrix_Header *h = HeaderOfMTX64_Matrix(mat);
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,mat);
    h->nor = nor;
    h->noc = noc;
    ResizeBag(mat, Size_Bag_Matrix(f, noc, nor));    
}


Obj MTX64_SLEchelizeDestructive(Obj self, Obj a)
{
    MTX64_Matrix_Header * h = HeaderOfMTX64_Matrix(a);
    UInt nrows = h->nor;
    UInt ncols = h->noc;
    UInt rklimit = (nrows > ncols) ? ncols: nrows; // minimum
    Obj rs = MTX64_MakeBitString(nrows);
    Obj cs = MTX64_MakeBitString(ncols);
    FELT det;
    Obj field = CALL_1ARGS(FieldOfMTX64Matrix,a);
    Obj m = NEW_MTX64_Matrix(field, rklimit, rklimit);
    Obj r = NEW_MTX64_Matrix(field, nrows, ncols); // this may be a bit too high
    Obj c = NEW_MTX64_Matrix(field, ncols, rklimit); // this is a bit too high, as both bounds cannot be achieved at once
    // Done with garbage collection here 
    uint64_t rank;
    uint64_t *rsp = DataOfBitStringObject(rs),
        *csp = DataOfBitStringObject(cs);
    Dfmt *mat = DataOfMTX64_Matrix(a),
        *multiply = DataOfMTX64_Matrix(m),
        *remnant = DataOfMTX64_Matrix(r),
        *cleaner = DataOfMTX64_Matrix(c);
    DSPACE ds;
    DSSet(DataOfFieldObject(field),ncols, &ds);
    rank = SLEch(&ds, mat, rsp, csp, &det, multiply, cleaner, remnant, nrows);
    // Garbage collection OK again here
    // Resize all the output matrices
    SetShapeAndResize(m, rank, rank);
    SetShapeAndResize(c, ncols - rank, rank);
    SetShapeAndResize(r, rank, ncols - rank);
    Obj result =  NEW_PREC(7);
    AssPRec(result, RNamName("rank"), INTOBJ_INT(rank));
    AssPRec(result, RNamName("det"), MakeMtx64Felt(field,det));
    AssPRec(result, RNamName("multiplier"), m);
    AssPRec(result, RNamName("cleaner"), c);
    AssPRec(result, RNamName("remnant"), r);
    AssPRec(result, RNamName("rowSelect"), rs);
    AssPRec(result, RNamName("colSelect"), cs);
    return result;
}

Obj MTX64_SLMultiply(Obj self, Obj a, Obj b, Obj c)
{
    MTX64_Matrix_Header * ha = HeaderOfMTX64_Matrix(a);
    MTX64_Matrix_Header * hb = HeaderOfMTX64_Matrix(b);
    UInt nora = ha->nor;
    UInt noca = ha->noc;
    UInt nocb = hb->noc;
    Obj field = CALL_1ARGS(FieldOfMTX64Matrix,a);
    FIELD *f = DataOfFieldObject(field);
    Dfmt *ap = DataOfMTX64_Matrix(a);
    Dfmt *bp = DataOfMTX64_Matrix(b);
    Dfmt *cp = DataOfMTX64_Matrix(c);
    SLMul(f,ap,bp,cp,nora,noca,nocb);
    return 0;
}

Obj MTX64_SLTranspose(Obj self, Obj mat, Obj tra)
{
    MTX64_Matrix_Header * h = HeaderOfMTX64_Matrix(mat);
    UInt nora = h->nor;
    UInt noca = h->noc;
    Obj field = CALL_1ARGS(FieldOfMTX64Matrix,mat);
    FIELD *f = DataOfFieldObject(field);
    Dfmt *mp = DataOfMTX64_Matrix(mat);
    Dfmt *tp = DataOfMTX64_Matrix(tra);
    SLTra(f, mp, tp, nora, noca);
    return 0;
}

Obj MTX64_LengthOfBitString(Obj self, Obj bs)
{
    return INTOBJ_INT(DataOfBitStringObject(bs)[0]);
}

Obj MTX64_WeightOfBitString(Obj self, Obj bs)
{
    return INTOBJ_INT(DataOfBitStringObject(bs)[1]);
}

Obj MTX64_GetEntryOfBitString(Obj self, Obj bs, Obj pos)
{
    return INTOBJ_INT(BSBitRead(DataOfBitStringObject(bs), INT_INTOBJ(pos)));
}

Obj MTX64_SetEntryOfBitString(Obj self, Obj bs, Obj pos)
{
    BSBitSet(DataOfBitStringObject(bs), INT_INTOBJ(pos));
    return 0;
}


Obj MTX64_ShallowCopyMatrix(Obj self, Obj m)
{
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m);
    UInt noc = HeaderOfMTX64_Matrix(m)->noc;
    UInt nor = HeaderOfMTX64_Matrix(m)->nor;
    Obj copy = NEW_MTX64_Matrix(f, nor, noc);
    memcpy(DataOfMTX64_Matrix(copy), DataOfMTX64_Matrix(m), Size_Data_Matrix(f,noc,nor));
    return copy;
}

// Assumes matrices are the same shape. Order may not be consistent with GAP lists
Obj MTX64_compareMatrices(Obj self, Obj m1, Obj m2)
{
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m1);
    UInt noc = HeaderOfMTX64_Matrix(m1)->noc;
    UInt nor = HeaderOfMTX64_Matrix(m1)->nor;
    Int res = memcmp(DataOfMTX64_Matrix(m1), DataOfMTX64_Matrix(m2), Size_Data_Matrix(f,noc,nor));
    return INTOBJ_INT((res < 0) ? -1 : (res > 0) ? 1 : 0);
}

Obj MTX64_ShallowCopyBitString(Obj self, Obj bs)
{
    UInt len = DataOfBitStringObject(bs)[0];
    Obj copy = MTX64_MakeBitString(len);
    memcpy(DataOfBitStringObject(copy), DataOfBitStringObject(bs), Size_Data_BitString(len));
    return copy;
}

// Order may not be consistent with GAP lists
Obj MTX64_compareBitStrings(Obj self, Obj bs1, Obj bs2)
{
    UInt len = DataOfBitStringObject(bs1)[0];
    Int res = memcmp(DataOfBitStringObject(bs1), DataOfBitStringObject(bs2), Size_Data_BitString(len));
    return INTOBJ_INT((res < 0) ? -1 : (res > 0) ? 1 : 0);
}

//
// limited to fields up to 2^60 or so by small int, but
// considerably smaller by memory, at least for a while
//
Obj MTX64_MakeFELTfromFFETable (Obj self, Obj field)
{
    FIELD *f = DataOfFieldObject(field);
    UInt q = f->fdef;
    UInt p = f->charc;
    Obj l = NEW_PLIST(T_PLIST_CYC+IMMUTABLE, q);
    // line above may cause GC
    f = DataOfFieldObject(field);
    SET_ELM_PLIST(l,1,INTOBJ_INT(0));
    FELT x = 1;
    FELT z = (p == q) ? f->conp : p;
    for (int i = 0; i < q-1; i++) {
        SET_ELM_PLIST(l, i+2, INTOBJ_INT(x));
        x = FieldMul(f, x, z);
    }
    SET_LEN_PLIST(l, q);
    return l;
}

static Obj MTX64_GetFELTfromFFETable;
static Obj MTX64_GetFFEfromFELTTable;

// Copy a T_VECFFE vector of appropriate field and length into
// the specified row of a meataxe64 matrix
// rownum is zero based 

Obj MTX64_InsertVecFFE(Obj self, Obj d, Obj v, Obj rownum) {
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    UInt len = LEN_PLIST(v);
    if ( len != HeaderOfMTX64_Matrix(d)->noc)
        ErrorMayQuit("row length mismatch",0,0);
    if (len == 0)
        return 0;    
    FF field = FLD_FFE(ELM_PLIST(v,1));    
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
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
    Obj *tptr = ADDR_OBJ(tab);
    for (UInt i = 0; i < len; i++)
        DPak(&ds, i, dptr, INT_INTOBJ(tptr[VAL_FFE(vptr[i+1])+1]));
    // GC OK again
    return INTOBJ_INT(len);
}

// rownum is zero based 
Obj MTX64_ExtractVecFFE(Obj self, Obj d, Obj rownum) {
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    UInt p = DataOfFieldObject(fld)->charc;
    UInt deg = DataOfFieldObject(fld)->pow;
    UInt len = HeaderOfMTX64_Matrix(d)->noc;
    if (len == 0)
        return NEW_PLIST(T_PLIST_EMPTY, 0);    
    Obj v = NEW_PLIST(T_PLIST_FFE, len);
    SET_LEN_PLIST(v,len);
    Obj tab = CALL_1ARGS(MTX64_GetFFEfromFELTTable, fld);
    // from here no GC
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(d, &ds);
    Obj *vptr = ADDR_OBJ(v);
    Dfmt *dptr = DataOfMTX64_Matrix(d);
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
    Obj *tptr = ADDR_OBJ(tab);
    for (UInt i = 0; i < len; i++)
        vptr[i+1] = tptr[1+DUnpak(&ds, i, dptr)];
    // GC OK again
    return v;
}


// This assumes little-endian storage
Obj MTX64_InsertVecGF2(Obj self, Obj d, Obj v, Obj rownum)
{
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    if (q != 2)
        // maybe the matrix is over a bigger field and we need to
        // do this the slow way
        return Fail;    
    GAP_ASSERT(IS_GF2VEC_REP(v));
    UInt len = LEN_GF2VEC(v);
    if ( len != HeaderOfMTX64_Matrix(d)->noc)
        ErrorMayQuit("row length mismatch",0,0);
    if (len == 0)
        return 0;    
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(d, &ds);
    Dfmt *dptr = DataOfMTX64_Matrix(d);
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
    memcpy(dptr,BLOCKS_GF2VEC(v),ds.nob);
    return INTOBJ_INT(len);
}

Obj MTX64_ExtractVecGF2(Obj self, Obj d, Obj rownum)
{
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    if (q != 2)
        ErrorMayQuit("field mismatch",0,0);
    UInt len = HeaderOfMTX64_Matrix(d)->noc;
    Obj v;
    NEW_GF2VEC(v,TYPE_LIST_GF2VEC, len);
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(d, &ds);
    Dfmt *dptr = DataOfMTX64_Matrix(d);
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
    memcpy(BLOCKS_GF2VEC(v),dptr, ds.nob);
    return v;
}


// makes a table such that table[i] is the
// byte of D-format corresponding to i in GAP 8 bit vector
// format

Obj MTX64_Make8BitConversion( Obj self, Obj fld)
{
    UInt q = DataOfFieldObject(fld)->fdef;
    GAP_ASSERT(2 < q && q <= 256);
    UInt p = DataOfFieldObject(fld)->charc;
    Obj tab = CALL_1ARGS(MTX64_GetFELTfromFFETable,fld);
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
            FELT ent3 = INT_INTOBJ(ELM_PLIST(tab, ent2+1));
            y += ent3*z;
            z *=q;
        }
        CHARS_STRING(tabout)[byte] = y;
    }
    return tabout;
}

static Obj MTX64_Get8BitImportTable;
static Obj MTX64_Get8BitExportTable;

Obj MTX64_InsertVec8Bit(Obj self, Obj d, Obj v, Obj rownum)
{
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    if (q != FIELD_VEC8BIT(v))
        // maybe the matrix is over a bigger field and we need to
        // do this the slow way
        return Fail;    
    GAP_ASSERT(IS_VEC8BIT_REP(v));
    UInt len = LEN_VEC8BIT(v);
    if ( len != HeaderOfMTX64_Matrix(d)->noc)
        ErrorMayQuit("row length mismatch",0,0);
    if (len == 0)
        return 0;
    Obj tbl = CALL_1ARGS(MTX64_Get8BitImportTable,fld);
    DSPACE ds;
    SetDSpaceOfMTX64_Matrix(d, &ds);
    UChar *tptr = CHARS_STRING(tbl);
    Dfmt *dptr = DataOfMTX64_Matrix(d);
    UInt1 *vptr = BYTES_VEC8BIT(v);
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);    
    for (UInt i = 0; i < ds.nob; i++)
        dptr[i] = tptr[vptr[i]];
    return INTOBJ_INT(len);
}

Obj MTX64_ExtractVec8Bit(Obj self, Obj d, Obj rownum)
{
    Obj fld = CALL_1ARGS(FieldOfMTX64Matrix,d);
    UInt q = DataOfFieldObject(fld)->fdef;
    if (q == 2 || q > 256)
        ErrorMayQuit("field mismatch",0,0);
    UInt len = HeaderOfMTX64_Matrix(d)->noc;
    Obj v = ZeroVec8Bit(q, len, 1);
    DSPACE ds;
    Obj tbl = CALL_1ARGS(MTX64_Get8BitExportTable,fld);
    SetDSpaceOfMTX64_Matrix(d, &ds);
    Dfmt *dptr = DataOfMTX64_Matrix(d);
    UInt1 *vptr = BYTES_VEC8BIT(v);
    UChar *tptr = CHARS_STRING(tbl);
    dptr = DPAdv(&ds, INT_INTOBJ(rownum), dptr);
    for (UInt i = 0; i < ds.nob; i++)
        vptr[i] = tptr[((UInt1 *)dptr)[i]];
    return v;
}


typedef Obj (* GVarFunc)(/*arguments*/);
#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params) \
  {#name, nparam, \
   params, \
   (GVarFunc)name, \
   srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateField, 1, "q"),

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

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCpy, 4, "src,dst,startrow,nrows"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCut, 5, "m,startrow,nrows,startcol,clip"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DPaste, 5, "clip,startrow,nrows,startcol,m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DAdd, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSub, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMad, 4, "nrows,scalar,d1,d2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMul, 3, "nrows,scalar,d1"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DNzl, 1, "m"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLMultiply, 3, "a, b, c"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLTranspose, 2, "m, t"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLEchelizeDestructive, 1, "a"),


    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_LengthOfBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_WeightOfBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SetEntryOfBitString, 2, "bs, pos"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_GetEntryOfBitString, 2, "bs, pos"),
    
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ShallowCopyMatrix, 1, "m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ShallowCopyBitString, 1, "bs"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_compareMatrices, 2, "m1, m2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_compareBitStrings, 2, "bs1, bs2"),
    
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_MakeFELTfromFFETable, 1, "q"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVecFFE, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVecFFE, 2, "d, row"),
        GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVecGF2, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVecGF2, 2, "d, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_Make8BitConversion, 1, "f"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_InsertVec8Bit, 3, "d, v, row"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractVec8Bit, 2, "d, row"),
        
    { 0 } /* Finish with an empty entry */

};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel(StructInitInfo *module) {
  /* init filters and functionsi */
  InitHdlrFuncsFromTable(GVarFuncs);

  ImportGVarFromLibrary("MTX64_FieldType", &TYPE_MTX64_Field);
  ImportGVarFromLibrary("MTX64_BitStringType", &TYPE_MTX64_BitString);
  ImportFuncFromLibrary("MTX64_FieldEltType", &TYPE_MTX64_Felt);
  ImportFuncFromLibrary("MTX64_MatrixType", &TYPE_MTX64_Matrix);
  ImportFuncFromLibrary("FieldOfMTX64Matrix", &FieldOfMTX64Matrix);
  ImportFuncFromLibrary("MTX64_GetFFEfromFELTTable",
                        &MTX64_GetFFEfromFELTTable);
  ImportFuncFromLibrary("MTX64_GetFELTfromFFETable",
                        &MTX64_GetFELTfromFFETable);
  ImportFuncFromLibrary("MTX64_Get8BitImportTable",&MTX64_Get8BitImportTable);
  ImportFuncFromLibrary("MTX64_Get8BitExportTable",&MTX64_Get8BitExportTable);

  /* return success */
  return 0;
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary( StructInitInfo *module )
{
    /* init filters and functions */
    InitGVarFuncsFromTable( GVarFuncs );

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
 /* postRestore = */ 0
};

StructInitInfo *Init__Dynamic( void )
{
    return &module;
}
