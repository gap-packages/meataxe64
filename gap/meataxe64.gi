#
# meataxe64: meataxe64
#
# Implementations
#

# Cache by prime?


## Finite fields

#
# Type of field objects (all the same)
#
BindGlobal( "MTX64_FieldFamily", NewFamily("MTX64_FieldFamily"));
BindGlobal( "MTX64_FieldType",
        NewType(MTX64_FieldFamily, IsMTX64FiniteField and IsDataObjectRep) );

BindGlobal( "MTX64_BitStringFamily", NewFamily("MTX64_BitStringFamily"));
BindGlobal( "MTX64_BitStringType",
        NewType(MTX64_BitStringFamily, IsMutable and IsMTX64BitString and IsDataObjectRep) );


InstallMethod( MTX64_FiniteField, "for a size",
               [ IsPosInt ],
        MemoizePosIntFunction(MTX64_CreateField,
                rec(flush := true)) );

InstallMethod( MTX64_FiniteField, "for a characteristic, and a degree",
               [ IsPosInt, IsPosInt ],
function(char, deg)
    return MTX64_FiniteField(char ^ deg);
end);

InstallMethod( ViewString, "for a meataxe64 field",
               [ IsMTX64FiniteField ],
               function(f)
                   local r;

                   r := "<MTX64 GF(";
                   Append(r, String(MTX64_FieldCharacteristic(f)));
                   if MTX64_FieldDegree(f) > 1 then
                       Append(r, "^");
                       Append(r, String(MTX64_FieldDegree(f)));
                   fi;
                   Append(r, ")>");
                   return r;
               end);


InstallMethod( \<, "for meataxe64 fields",
               [IsMTX64FiniteField, IsMTX64FiniteField],
               {F1, F2} -> MTX64_FieldOrder(F1) < MTX64_FieldOrder(F2));

## Elements of finite fields
#

BindGlobal("MTX64_FieldEltType", MemoizePosIntFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_ElementFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMTX64FiniteFieldElement and IsDataObjectRep);
end, rec(flush := true)));



InstallMethod( MTX64_FiniteFieldElement, "",
        [ IsMTX64FiniteField, IsInt ],        
        MTX64_CreateFieldElement);

InstallGlobalFunction( MTX64_FieldOfElement, "",
function(e)
    return FamilyObj(e)!.field;
end);

InstallMethod( ViewString, "for a meataxe64 field element",
               [ IsMTX64FiniteFieldElement ],
function(e)
    local r;
    return STRINGIFY("<"
                  , ViewString(MTX64_ExtractFieldElement(e))
                  , " : "
                  , ViewString(MTX64_FieldOfElement(e))
                  , ">");
end);

BIND_GLOBAL("MTX64_MatrixType",MemoizePosIntFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_MatrixFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMutable and IsMTX64Matrix and IsDataObjectRep);
end, rec(flush := true)));

BIND_GLOBAL("FieldOfMTX64Matrix", m -> FamilyObj(m)!.field);


InstallMethod( ViewString, "for a meataxe64 matrix",
               [ IsMTX64Matrix ],
function(m)
    local f;
    f := FieldOfMTX64Matrix(m);    
    return STRINGIFY("< matrix "
                   , MTX64_Matrix_NumRows(m)
                   , "x"
                   , MTX64_Matrix_NumCols(m)
                  , " : "
                  , ViewString(f)
                  , ">");
end);

InstallMethod( ViewString, "for a meataxe64 bitstring",
        [IsMTX64BitString],
        bs -> STRINGIFY("< MTX64 bitstring ",MTX64_WeightOfBitString(bs),"/",MTX64_LengthOfBitString(bs),">"));

InstallMethod( ShallowCopy, "for a meataxe matrix", [IsMTX64Matrix], MTX64_ShallowCopyMatrix);
InstallMethod( ShallowCopy, "for a meataxe bitstring", [IsMTX64BitString], MTX64_ShallowCopyBitString);

InstallOtherMethod(\*, "for meataxe64 matrices", IsIdenticalObj, [IsMTX64Matrix, IsMTX64Matrix], 
        function(m1,m2)
    local m;    
    if MTX64_Matrix_NumCols(m1) <> MTX64_Matrix_NumRows(m2) then
        Error("Incompatible matrices");
    fi;
    m := MTX64_NewMatrix(FieldOfMTX64Matrix(m1), MTX64_Matrix_NumRows(m1), MTX64_Matrix_NumCols(m2));
    MTX64_SLMultiply(m1,m2,m);
    return m;
end);

InstallOtherMethod(\[\], "for a meataxe64 matrix and two indices", [IsMTX64Matrix, IsPosInt, IsPosInt], 
        function(m, i, j)
    if i > MTX64_Matrix_NumRows(m) or 
       j > MTX64_Matrix_NumCols(m) then
        Error("Indices out of range");
    fi;
    return MTX64_GetEntry(m, i-1, j-1);
end);

InstallOtherMethod(\=, [IsMTX64FiniteField, IsMTX64FiniteField],
        IsIdenticalObj);


InstallOtherMethod(\[\]\:\=, "for a meataxe64 matrix and two indices and a FELT", [IsMTX64Matrix and IsMutable, IsPosInt, IsPosInt, IsMTX64FiniteFieldElement], 
        function(m, i, j, x)
    if i > MTX64_Matrix_NumRows(m) or 
       j > MTX64_Matrix_NumCols(m) then
        Error("Indices out of range");
    fi;
    if MTX64_FieldOfElement(x) <> FieldOfMTX64Matrix(m) then
        Error("ELement in wrong field");
    fi;
    MTX64_SetEntry(m, i-1, j-1, x);
end);

InstallOtherMethod(TransposedMatMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local t;    
    t := MTX64_NewMatrix(FieldOfMTX64Matrix(m), MTX64_Matrix_NumRows(m), MTX64_Matrix_NumCols(m));
    MTX64_SLTranspose(m,t);
    return t;
end);

InstallOtherMethod(TransposedMatImmutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local t;    
    t := MTX64_NewMatrix(FieldOfMTX64Matrix(m), MTX64_Matrix_NumRows(m), MTX64_Matrix_NumCols(m));
    MTX64_SLTranspose(m,t);
    MakeImmutable(t);    
    return t;
end);

InstallMethod(Display, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    Display(List([1..MTX64_Matrix_NumRows(m)], i->
            List([1..MTX64_Matrix_NumCols(m)], j -> MTX64_ExtractFieldElement(m[i,j]))));
    
      end );


        




