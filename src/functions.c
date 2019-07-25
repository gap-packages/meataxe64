/* Meataxe 64 -- interface to the functions level */

#include "src/compiled.h" /* GAP headers */
#include "mtx64/field.h"
// field.h needs to be first, stop clang-format moving it
#include "mtx64/funs.h"

/* This is really just a lot of wrappers, since inputs and outputs are strings 
   for now we ignore logging and set all silent flags to 1.
   We take Fail as the equivalent of NULL for output filenames (meaning 
   don't write this output)

   We can write and read matrices using the MTX64_WriteMatrix and MTX64_ReadMatrix 
   functions to move between in-core and on-disk working. 

   TODO:  functions to read and write maps and bitstrings
 */

static inline const char * CHECK_STRING(Obj s) {
  if (!IsStringConv(s))
    ErrorMayQuit("MTX64: Given path is not a string", 0, 0);
  return CSTR_STRING(s);
}


// All these Sy testers actually return 0 for true and -1 for false!

static inline void CHECK_TMPDIR(Obj tmp) {
  const char *path = CHECK_STRING(tmp);
  if (SyIsDirectoryPath(path) || SyIsWritableFile(path) ||
      SyIsReadableFile(path))
    ErrorMayQuit("MTX64: Given temp directory %s is not a directory path"
                 " or not accessible",
                 (Int)path, 0);
}

static inline void CHECK_INFILE(Obj fn) {
  const char *path = CHECK_STRING(fn);
  if (!SyIsDirectoryPath(path) || SyIsReadableFile(path))
    ErrorMayQuit("MTX64: Given input filename %s is a directory"
                 " or not readable",
                 (Int)path, 0);
}

static inline void CHECK_OUTFILE(Obj fn) {
  if (fn == Fail)
      return;
  const char *path = CHECK_STRING(fn);
  if (!SyIsDirectoryPath(path))
      ErrorMayQuit("MTX64: Given output filename %s is a directory",
                 (Int)path, 0);
}

static Obj FuncMTX64_fTranspose(Obj self, Obj tmp, Obj in, Obj out) {
    CHECK_TMPDIR(tmp);
    CHECK_INFILE(in);
    CHECK_OUTFILE(out);
    fTranspose(CSTR_STRING(tmp), CSTR_STRING(in), 0,
               out == Fail ? NULL : CSTR_STRING(out), 0);
    return 0;
}

static Obj FuncMTX64_fMultiply(Obj self, Obj tmp, Obj m1, Obj m2, Obj m3) {
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


