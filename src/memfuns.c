/* Meataxe 64 -- interface to the new memory functions level */

#include "src/compiled.h" /* GAP headers */
#include "mtx64/field.h"
// field.h needs to be first, stop clang-format moving it
#include "mtx64/memfuns.h"



static Obj FuncMTX64_mfMultiply(Obj self, Obj tmp, Obj m1, Obj m2, Obj m3) {
    CHECK_TMPDIR(tmp);
    CHECK_INFILE(m1);
    CHECK_INFILE(m2);
    CHECK_OUTFILE(m3);
    fMultiply(CSTR_STRING(tmp), CSTR_STRING(m1), 0, CSTR_STRING(m2), 0,
              m3 == Fail ? NULL : CSTR_STRING(m3), 0);
    return 0;
}

static Obj FuncMTX64_fMultiplyAdd(Obj self, Obj tmp, Obj m1, Obj m2, Obj m3, Obj m4) {
    CHECK_TMPDIR(tmp);
    CHECK_INFILE(m1);
    CHECK_INFILE(m2);
    CHECK_INFILE(m3);
    CHECK_OUTFILE(m4);
    fMultiplyAdd(CSTR_STRING(tmp), CSTR_STRING(m1), 0, CSTR_STRING(m2), 0,
              CSTR_STRING(m3),0, m4 == Fail ? NULL: CSTR_STRING(m4),0);
    return 0;
}

static Obj FuncMTX64_fProduceNREF(Obj self, Obj tmp, Obj m1, Obj b2, Obj m3) {
    CHECK_TMPDIR(tmp);
    CHECK_INFILE(m1);
    CHECK_OUTFILE(b2);
    CHECK_OUTFILE(m3);
    UInt rk  = fProduceNREF(CSTR_STRING(tmp), CSTR_STRING(m1), 0,
                            b2 == Fail ? NULL : CSTR_STRING(b2), 0,
                            m3 == Fail ? NULL : CSTR_STRING(m3), 0);
    return INTOBJ_INT(rk);
}

static Obj FuncMTX64_fEchelize(Obj self, Obj args ) {
    if (!IS_PLIST(args) || LEN_PLIST(args) != 7)
        ErrorMayQuit("Usage MTX64_fEchelize(tmpdir, a, rs, cs, m, k, r)", 0, 0);
    Obj tmp = ELM_PLIST(args,1);
    Obj a = ELM_PLIST(args,2);
    Obj rs = ELM_PLIST(args,3);
    Obj cs = ELM_PLIST(args,4);
    Obj m = ELM_PLIST(args,5);
    Obj k = ELM_PLIST(args,6);
    Obj r = ELM_PLIST(args,7);
    CHECK_TMPDIR(tmp);
    CHECK_INFILE(a);
    CHECK_OUTFILE(cs);
    CHECK_OUTFILE(rs);
    CHECK_OUTFILE(m);
    CHECK_OUTFILE(k);
    CHECK_OUTFILE(r);
    UInt rk = fFullEchelize(CSTR_STRING(tmp), CSTR_STRING(a), 0,
                            cs == Fail ? NULL : CSTR_STRING(cs), 0,
                            rs == Fail ? NULL : CSTR_STRING(rs), 0,
                            m == Fail ? NULL : CSTR_STRING(m), 0,
                            k == Fail ? NULL : CSTR_STRING(k), 0,
                            r == Fail ? NULL : CSTR_STRING(r), 0);
    return INTOBJ_INT(rk);
    return 0;
}

// Table of functions to export
static StructGVarFunc GVarFuncs[] = {
    GVAR_FUNC(MTX64_fTranspose, 3, "tmpdir, in, out"),
    GVAR_FUNC(MTX64_fMultiply, 4, "tmpdir, m1, m2, m3"),
    GVAR_FUNC(MTX64_fMultiplyAdd, 5, "tmpdir, m1, m2, m3, m4"),
    GVAR_FUNC(MTX64_fProduceNREF, 4, "tmpdir, m1, b2, m3"),
    GVAR_FUNC(MTX64_fEchelize, -1, "tmpdir, a, cs, rs, m, k, r"),
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
 StructInitInfo InitFunctions = {
    .type = MODULE_DYNAMIC,
    .name = "mtx64funcs",
    .initKernel = InitKernel,
    .initLibrary = InitLibrary,
};


