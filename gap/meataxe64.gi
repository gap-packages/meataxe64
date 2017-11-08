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
    return NewType(fam, IsMTX64Matrix and IsDataObjectRep);
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





