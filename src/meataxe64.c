/*
 * meataxe64: meataxe64
 */

#include "src/compiled.h"          /* GAP headers */

#include <assert.h>
#include "mtx64/field.h"

static Obj TYPE_MTX64_Field;
static Obj TYPE_MTX64_DSpace;

static inline void MTX64_Field_ElementFamily_Set(Obj f, Obj fam)
{ *((Obj *)(&ADDR_OBJ(f)[1])) = fam; }
static inline Obj MTX64_Field_ElementFamily_Get(Obj f)
{ return (Obj)(ADDR_OBJ(f)[1]); }

static inline void MTX64_Field_ElementType_Set(Obj f, Obj type)
{ *((Obj *)(&ADDR_OBJ(f)[2])) = type; }
static inline Obj MTX64_Field_ElementType_Get(Obj f)
{ return (Obj)(ADDR_OBJ(f)[2]); }

static inline FIELD *MTX64_Obj_Field(Obj f)
{ return (FIELD *)(&ADDR_OBJ(f)[3]); }
static inline DSPACE *MTX64_Obj_DSpace(Obj ds)
{ return (DSPACE *)(&ADDR_OBJ(ds)[2]); }

static inline void MTX64_FieldElt_Set(Obj o, UInt8 elt)
{ *((UInt8 *)(&ADDR_OBJ(o)[2])) = elt; }
static inline UInt8 MTX64_FieldElt_Get(Obj o)
{ return (UInt8)(ADDR_OBJ(o)[2]); }

/* Creates a meataxe finite field of size q = p^d */
/* TODO: Consider subbag for field object? */
Obj MTX64_CreateField(Obj self, Obj q, Obj etype)
{
    Obj result;
    UInt8 fdef;

    // TODO: Sufficient checking
    fdef = INT_INTOBJ(q);

    result = NewBag(T_DATOBJ, FIELDLEN + 3);
    SetTypeDatObj(result, TYPE_MTX64_Field);
    MTX64_Field_ElementType_Set(result, etype);
    FieldSet(fdef, MTX64_Obj_Field(result));
    CHANGED_BAG(result);

    return result;
}

Obj MTX64_FieldOrder(Obj self, Obj field)
{
    FIELD * _f = MTX64_Obj_Field(field);
    return INTOBJ_INT(_f->fdef);
}

Obj MTX64_FieldCharacteristic(Obj self, Obj field)
{
    FIELD * _f = MTX64_Obj_Field(field);
    return INTOBJ_INT(_f->charc);
}

Obj MTX64_FieldDegree(Obj self, Obj field)
{
    FIELD * _f = MTX64_Obj_Field(field);
    return INTOBJ_INT(_f->pow);
}

Obj MTX64_BoxFieldElement(Obj self, Obj field, FELT elt)
{
    Obj result;

    result = NewBag(T_DATOBJ, sizeof(Obj) + 2 * sizeof(UInt8)); // Yeah, I know, it's a waste of space
    SetTypeDatObj(result, MTX64_Field_ElementType_Get(field));
    MTX64_FieldElt_Set(result, elt);
    CHANGED_BAG(result);

    return result;
}

Obj MTX64_CreateFieldElement(Obj self, Obj field, Obj elt)
{
    return MTX64_BoxFieldElement(self, field, INT_INTOBJ(elt));
}

Obj MTX64_ExtractFieldElement(Obj self, Obj elt)
{
    return INTOBJ_INT(MTX64_FieldElt_Get(elt));
}

Obj MTX64_FieldAdd(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);
    FELT _b = MTX64_FieldElt_Get(b);

    return MTX64_BoxFieldElement(self, field, FieldAdd(_f, _a, _b));
}

Obj MTX64_FieldNeg(Obj self, Obj field, Obj a)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);

    return MTX64_BoxFieldElement(self, field, FieldNeg(_f, _a));
}

Obj MTX64_FieldSub(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);
    FELT _b = MTX64_FieldElt_Get(b);

    return MTX64_BoxFieldElement(self, field, FieldSub(_f, _a, _b));
}

Obj MTX64_FieldMul(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);
    FELT _b = MTX64_FieldElt_Get(b);

    return MTX64_BoxFieldElement(self, field, FieldMul(_f, _a, _b));
}

Obj MTX64_FieldInv(Obj self, Obj field, Obj a)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);

    return MTX64_BoxFieldElement(self, field, FieldInv(_f, _a));
}

Obj MTX64_FieldDiv(Obj self, Obj field, Obj a, Obj b)
{
    FIELD *_f = MTX64_Obj_Field(field);
    FELT _a = MTX64_FieldElt_Get(a);
    FELT _b = MTX64_FieldElt_Get(b);

    return MTX64_BoxFieldElement(self, field, FieldDiv(_f, _a, _b));
}

// DSpace
Obj MTX64_CreateDSpace(Obj self, Obj field, Obj noc)
{
    Obj result;
    UInt8 fnoc;

    // Number of Columns
    fnoc = INT_INTOBJ(noc);

    result = NewBag(T_DATOBJ, sizeof(DSPACE) + sizeof(UInt8));
    SetTypeDatObj(result, TYPE_MTX64_DSpace);
    DSSet(MTX64_Obj_Field(field), fnoc, MTX64_Obj_DSpace(result));
    CHANGED_BAG(result);

    return result;
}

Obj MTX64_CreateDfmt(Obj self, Obj field, Obj size)
{
    Obj result;
    UInt8 isize = INTOBJ_INT(size); //T Check

    // Number of Columns
    fnoc = INT_INTOBJ(noc);

    result = NewBag(T_DATOBJ, sizeof(DSPACE) + sizeof(UInt8) + isize);
    SetTypeDatObj(result, TYPE_MTX64_Dfmt);
    memset(MTX64_Obj_Dfmt(result), 0, isize);
    CHANGED_BAG(result);

    return result;
}

Obj MTX64_DUnpak(Obj self, Obj col, Obj dfmt)
{
}

Obj MTX64_DPak(Obj self, Obj col, Obj dfmt, Obj felt)
{
}

Obj MTX64_DCpy(Obj self, Obj src, Obj dst, Obj nrows)
{
    
}

Obj MTX64_DCut(Obj self, Obj  )
{
    
}

Obj MTX64_DPaste(Obj self)
{
    
}

Obj MTX64_DAdd(Obj self, Obj nrows, Obj d1, Obj d2)
{
    // return d
}

Obj MTX64_DSub(Obj self, Obj nrows, Obj d1, Obj d2)
{
    // return d
}

Obj MTX64_DSMad(Obj self, Obj nrows, Obj scalar, Obj d1, Obj d2)
{
    // return d
}

// In place?
Obj MTX64_DSMul(Obj self, Obj nrows, Obj scalar, Obj d1)
{
    
}


// Higher level stuff
Obj MTX64_SLEchelize(Obj self, Obj field, Obj a, Obj nrows, Obj ncols)
{
    Obj result;
    uint64_t rank;
    uint64_t *rs, *cs;
    Dfmt *multiply, *remnant, *cleaner;

    // NewBaggin?
    rs = malloc((nrows >> 3) + 2);
    cs = malloc((ncols >> 3) + 2);

    multiply = malloc();
    remnant = malloc();
    cleaner = malloc();

    rank = SLEch(f, a, rs, cs, multiply, cleaner, remnant, nrows, ncols);

    result = NewPRec(4);



    return result;
}

Obj MTX64_SLMultiply(Obj self, Obj field, Obj a, Obj b)
{
    
}

Obj MTX64_SLTranspose(Obj self, Obj field, Obj mat)
{

}

typedef Obj (* GVarFunc)(/*arguments*/);
#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params) \
  {#name, nparam, \
   params, \
   (GVarFunc)name, \
   srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateField, 2, ""),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldOrder, 1, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldCharacteristic, 1, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDegree, 1, ""),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateFieldElement, 2, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_ExtractFieldElement, 1, ""),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldAdd, 3, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldNeg, 2, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldSub, 3, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldMul, 3, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldInv, 2, ""),
    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_FieldDiv, 3, ""),

    GVAR_FUNC_TABLE_ENTRY("meataxe64.c", MTX64_CreateDSpace, 2, ""),
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
    ImportGVarFromLibrary( "MTX64_FieldEltType", &TYPE_MTX64_DSpace);

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
