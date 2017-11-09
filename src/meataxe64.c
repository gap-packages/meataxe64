/*
 * meataxe64: meataxe64
 */

#include "src/compiled.h"          /* GAP headers */

#include <assert.h>
#include "mtx64/field.h"
#include "mtx64/slab.h"

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
   accessed only by their own functions (and maybe a few OtherMethods
   for convenience).


   We might want to change this later and unite these with the
   MatrixObj development, or pay for another layer of wrapping.

   TODO -- consts and asserts in appropriate places.
   

   API for SLEch isn't documentred.
   SLEch  a input mx, rs, cs pre-allocated to size (2x 64 + space for bits)
          det for return of determinant (not used), m, c get multiplier and cleaner
          r remnant, nor number of rows of a

   SLEchS required to produce standard row select
 */

static Obj TYPE_MTX64_Field;     // global variable. All FIELDS have same type 
static Obj TYPE_MTX64_Felt;      // function, takes field
static Obj TYPE_MTX64_Matrix;    // function takes field
static Obj TYPE_MTX64_BitString; // global variable, type of MTX64 Bitstring objects

static inline uint64_t *DataOfBitStringObject(Obj bs) {
    return (uint64_t *)(ADDR_OBJ(bs)+1);
}

static inline Obj MTX64_MakeBitString(UInt len) {
    Obj bs = NewBag(T_DATOBJ, sizeof(Obj) + 2*sizeof(uint64_t) + 8*((len + 63)/64));
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

static inline UInt Size_MTX64_Matrix(Obj f, UInt noc, UInt nor) {
    DSPACE ds;
    DSSet(DataOfFieldObject(f), noc, &ds);
    return sizeof(Obj) + sizeof(MTX64_Matrix_Header) + ds.nob*nor;
}

static inline Obj NEW_MTX64_Matrix(Obj f, UInt noc, UInt nor) {
    Obj m;
    m = NewBag(T_DATOBJ,Size_MTX64_Matrix(f, noc, nor) );
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
    return MakeMtx64Felt(field, INT_INTOBJ(elt));
}

Obj MTX64_ExtractFieldElement(Obj self, Obj elt)
{
    return INTOBJ_INT(GetFELTFromFELTObject(elt));
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
static FELT GetEntryMTX64(Obj m, UInt col, UInt row)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    return DUnpak(&ds, col, d);
}

Obj MTX64_GetEntry(Obj self, Obj m, Obj col, Obj row)
{
    Obj f = CALL_1ARGS(FieldOfMTX64Matrix,m);
    return MakeMtx64Felt(f,GetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row)));
}

void SetEntryMTX64(Obj m, UInt col, UInt row, FELT entry)
{
    DSPACE ds;
    Dfmt * d;
    SetDSpaceOfMTX64_Matrix(m, &ds);
    d = DataOfMTX64_Matrix(m);
    d = DPAdv(&ds, row, d);
    DPak(&ds, col, d, entry);
}

Obj MTX64_SetEntry(Obj self, Obj m, Obj col, Obj row, Obj entry)
{
    SetEntryMTX64(m, INT_INTOBJ(col), INT_INTOBJ(row),GetFELTFromFELTObject(entry));
    return 0;
}

Obj MTX64_DCpy(Obj self, Obj src, Obj dst, Obj nrows)
{
    DSPACE ds;
    Dfmt * sp, *dp;
    SetDSpaceOfMTX64_Matrix(src, &ds);
    sp = DataOfMTX64_Matrix(src);
    dp = DataOfMTX64_Matrix(dst);
    DCpy(&ds, sp, INT_INTOBJ(nrows), dp);
    return 0;
}

Obj MTX64_DCut(Obj self, Obj m, Obj nrows, Obj startcol, Obj clip ) 
{
    DSPACE ms, cs;
    Dfmt * mp, *cp;
    SetDSpaceOfMTX64_Matrix(m, &ms);
    SetDSpaceOfMTX64_Matrix(clip, &cs);
    mp = DataOfMTX64_Matrix(m);
    cp = DataOfMTX64_Matrix(clip);
    DCut(&ms, INT_INTOBJ(nrows), INT_INTOBJ(startcol),
         mp, &cs, cp);
    return 0;
}

Obj MTX64_DPaste(Obj self, Obj clip, Obj nrows, Obj startcol, Obj m)
{
    DSPACE ms, cs;
    Dfmt * mp, *cp;
    SetDSpaceOfMTX64_Matrix(m, &ms);
    SetDSpaceOfMTX64_Matrix(clip, &cs);
    mp = DataOfMTX64_Matrix(m);
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
    ResizeBag(mat, Size_MTX64_Matrix(f, noc, nor));    
}


Obj MTX64_SLEchelize(Obj self, Obj a)
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
    Obj r = NEW_MTX64_Matrix(field, ncols, nrows); // this may be a bit too high
    Obj c = NEW_MTX64_Matrix(field, rklimit, ncols); // this is a bit too high, as both bounds cannot be achieved at once
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

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCpy, 3, "src,dst,nrows"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DCut, 4, "m,nrows,startcol,clip"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DPaste, 4, "clip,nrows,startcol,m"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DAdd, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSub, 4, "nrows,d1,d2,d"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMad, 4, "nrows,scalar,d1,d2"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DSMul, 3, "nrows,scalar,d1"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_DNzl, 1, "m"),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLMultiply, 3, "a, b, c"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLTranspose, 2, "m, t"),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_SLEchelize, 1, "a"),
    
    { 0 } /* Finish with an empty entry */

};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel( StructInitInfo *module )
{
    /* init filters and functionsi */
    InitHdlrFuncsFromTable( GVarFuncs );

    ImportGVarFromLibrary( "MTX64_FieldType", &TYPE_MTX64_Field);
    ImportGVarFromLibrary( "MTX64_BitStringType", &TYPE_MTX64_BitString);
    ImportFuncFromLibrary( "MTX64_FieldEltType", &TYPE_MTX64_Felt);
    ImportFuncFromLibrary( "MTX64_MatrixType", &TYPE_MTX64_Matrix);
    ImportFuncFromLibrary( "FieldOfMTX64Matrix", &FieldOfMTX64Matrix);

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
