#
# meataxe64: meataxe64
#
# Implementations
#

# Cache by prime?


## Finite fields
BindGlobal( "MTX64_FF_CACHE", rec());

BindGlobal( "MTX64_FieldFamily", NewFamily("MTX64_FieldFamily"));
BindGlobal( "MTX64_FieldType",
            NewType(MTX64_FieldFamily, IsMTX64FiniteField ) );

BindGlobal( "MTX64_DSpaceFamily", NewFamily("MTX64_DSpaceFamily"));
BindGlobal( "MTX64_DSpaceType",
            NewType(MTX64_DSpaceFamily, IsMTX64DSpace ) );

InstallMethod( MTX64_FiniteField, "for a size",
               [ IsPosInt ],
function(size)
    local fam, etype;
    if not IsBound(MTX64_FF_CACHE.(size)) then
        fam := NewFamily(STRINGIFY("MTX64_GF(", size, ")_ElementFamily"));
        etype := NewType(fam, IsMTX64FiniteFieldElement);
        MTX64_FF_CACHE.(size) := MTX64_CreateField(size, etype);
        #T Correct?
        fam!.field := MTX64_FF_CACHE.(size);
    fi;
    return MTX64_FF_CACHE.(size);
end);

InstallMethod( MTX64_FiniteField, "for a characteristic, and a degree",
               [ IsPosInt, IsPosInt ],
function(char, deg)
    #TODO: Check.
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

InstallMethod( ViewObj, "for a meataxe64 field",
               [ IsMTX64FiniteField ],
               function(f)
                   Print(ViewString(f));
               end);

InstallMethod( \<, "for meataxe64 fields",
               [IsMTX64FiniteField, IsMTX64FiniteField],
               {F1, F2} -> MTX64_FieldOrder(F1) < MTX64_FieldOrder(F2));

## Elements of finite fields
#
InstallMethod( MTX64_FiniteFieldElement, "",
               [ IsMTX64FiniteField, IsPosInt ],
function(field, elt)
    return MTX64_CreateFieldElement(field, elt);
end);

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

InstallMethod( ViewObj, "for a meataxe64 field",
               [ IsMTX64FiniteField ],
function(f)
    Print(ViewString(f));
end);







