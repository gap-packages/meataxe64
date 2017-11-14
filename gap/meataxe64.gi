#
# meataxe64: meataxe64
#
# Implementations
#

#
# Types and Families -- these variables and functions are imported and used by the C code
# to create objects
#
BindGlobal( "MTX64_FieldFamily", NewFamily("MTX64_FieldFamily"));
BindGlobal( "MTX64_FieldType",
        NewType(MTX64_FieldFamily, IsMTX64FiniteField and IsDataObjectRep) );

BindGlobal( "MTX64_BitStringFamily", NewFamily("MTX64_BitStringFamily"));
BindGlobal( "MTX64_BitStringType",
        NewType(MTX64_BitStringFamily, IsMutable and IsMTX64BitString and IsDataObjectRep) );

BindGlobal("MTX64_FieldEltType", MemoizePosIntFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_ElementFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMTX64FiniteFieldElement and IsDataObjectRep);
end, rec(flush := true)));

BIND_GLOBAL("MTX64_MatrixType",MemoizePosIntFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_MatrixFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMutable and IsMTX64Matrix and IsDataObjectRep);
end, rec(flush := true)));

BIND_GLOBAL("FieldOfMTX64Matrix", m -> FamilyObj(m)!.field);

#
# Fields
#

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

InstallMethod(\=, [IsMTX64FiniteField, IsMTX64FiniteField],
        IsIdenticalObj);
                            
InstallMethod( \<, "for meataxe64 fields",
               [IsMTX64FiniteField, IsMTX64FiniteField],
               {F1, F2} -> MTX64_FieldOrder(F1) < MTX64_FieldOrder(F2));

#
## Elements of finite fields
#

InstallMethod( MTX64_FiniteFieldElement, "for a meataxe64 field and an integer",
        [ IsMTX64FiniteField, IsInt ],        
        MTX64_CreateFieldElement);

BindGlobal("MTX64_PopulateConversionTable", function(f)
    local  tab1, tab2, q, z, y, i, fam;
    tab1 := MTX64_MakeFELTfromFFETable(f);
    tab2 := [];
    q := Length(tab1);        
    z := Z(q);
    tab2[1] := 0*z;
    y := z^0;        
    for i in [2..q] do
        tab2[1+tab1[i]] := y;
        y := y*z;            
    od;
    fam := FamilyObj(f);
    
    fam!.FFEfromFELTTable := tab2;
    fam!.FELTfromFFETable := tab1;
    return;
end);



BindGlobal("MTX64_GetFFEfromFELTTable", function(f)
    local  fam;
    fam := FamilyObj(f);    
    if not IsBound(fam!.FFEfromFELTTable) then
        MTX64_PopulateConversionTable(f);        
    fi;
    return fam!.FFEfromFELTTable;
end);

    
BindGlobal("MTX64_GetFELTfromFFETable", function(f)
    local  fam;
    fam := FamilyObj(f);    
    if not IsBound(fam!.FFEfromFELTTable) then
        MTX64_PopulateConversionTable(f);        
    fi;
    return fam!.FELTfromFFETable;
end);

    
    

InstallMethod( MTX64_FiniteFieldElement, "for a meataxe64 field and an FFE",
        [ IsMTX64FiniteField, IsFFE ],        
        function(field, ffe)
    local  d, p, tab, x, cb, vec, pp, c;
    d := MTX64_FieldDegree(field);
    p := Characteristic(ffe);    
    if p <> MTX64_FieldCharacteristic(field) or 
       d < DegreeFFE(ffe) then
        Error("Element not in field");        
    fi;
    if p^d <= 65536 then
        tab := MTX64_MakeFELTfromFFETable(field);
        if IsZero(ffe) then
            x := 0;
        else 
            x := tab[LogFFE(ffe,Z(p,d))+1];
        fi;
    else      
        cb := CanonicalBasis(AsVectorSpace(GF(p),GF(p,d)));
        vec := Coefficients(cb, ffe);
        x := 0;
        pp := 1;    
        for c  in vec do        
            x := x+IntFFE(c)*pp;
            pp := pp*p;
        od;
    fi;
    return MTX64_FiniteFieldElement(field,x);
end);

InstallOtherMethod( MTX64_FiniteFieldElement, "for an FFE, choose default field",
        [IsFFE],
        ffe-> MTX64_FiniteFieldElement(MTX64_FiniteField(Characteristic(ffe), DegreeFFE(ffe)), ffe));




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

InstallMethod(\=, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_ExtractFieldElement(x) = MTX64_ExtractFieldElement(y);
end);

InstallMethod(\<, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_ExtractFieldElement(x) < MTX64_ExtractFieldElement(y);
end);


InstallMethod(Zero, "for a meataxe64 field element",
        [IsMTX64FiniteFieldElement],
        x -> Zero(MTX64_FieldOfElement(x)));

InstallMethod(One, "for a meataxe64 field element",
        [IsMTX64FiniteFieldElement],
        x -> One(MTX64_FieldOfElement(x)));

InstallMethod(\+, "meataxe64 field elements", IsIdenticalObj,   [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldAdd(MTX64_FieldOfElement(x), x, y);    
end);

InstallMethod(\-, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldAdd(MTX64_FieldOfElement(x), x, y);    
end);

InstallMethod(AdditiveInverse, "meataxe64 field element",  
        [IsMTX64FiniteFieldElement],
        x -> MTX64_FieldNeg(MTX64_FieldOfElement(x), x));

InstallMethod(\*, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldMul(MTX64_FieldOfElement(x), x, y);    
end);

InstallMethod(QUO, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldDiv(MTX64_FieldOfElement(x), x, y);    
end);

InstallMethod(Inverse,  "meataxe64 field element",  
        [IsMTX64FiniteFieldElement],
    x ->  MTX64_FieldInv(MTX64_FieldOfElement(x), x));


FFEfromFELT := function(felt)
    local  fld, p, d, x, tab, z, zp, y;
    fld := MTX64_FieldOfElement(felt);    
    p := MTX64_FieldCharacteristic(fld);
    d := MTX64_FieldDegree(fld);
    x := MTX64_ExtractFieldElement(felt);
    if p^d <= 65536 then
        tab := MTX64_GetFFEfromFELTTable(fld);
        return tab[x+1];
    fi;
    z := Z(p,d);
    zp := z^0;    
    y := 0*z;    
    while x <> 0 do
        y := y+ zp*(x mod p);
        zp := zp*z;  
        x := QuoInt(x,p);        
    od;
    return y;    
end;


InstallOtherMethod(Zero, "for a meataxe64 field",
        [IsMTX64FiniteField],
        f->MTX64_FiniteFieldElement(f,0));

InstallOtherMethod(One, "for a meataxe64 field",
        [IsMTX64FiniteField],
        f->MTX64_FiniteFieldElement(f,1));


#
# Matrices
#
#
# row is zero based
#

BindGlobal( "MTX64_InsertVector",
        function(m,v,row)
    local  f, i;
    if IsGF2VectorRep(v) and fail <> MTX64_InsertVecGF2(m,v,row) then
        return;
    fi;
    if (TNUM_OBJ_INT(v) = T_PLIST_FFE or TNUM_OBJ_INT(v) = T_PLIST_FFE+1)
        and fail <> MTX64_InsertVecFFE(m, v, row) then
        return;
    fi;
    f := FieldOfMTX64Matrix(m);    
    for i in [1..Length(v)] do
        m[row+1, i] := MTX64_FiniteFieldElement(f, v[i]);
    od;
    return;
end);

InstallOtherMethod(\[\]\:\=, [IsMTX64Matrix and IsMutable, IsPosInt, IsRowVector and IsFFECollection],
                          function(m,i,v)
    MTX64_InsertVector(m,v,i-1);
end);


#
# row is zero based
#
BindGlobal( "MTX64_ExtractVector", 
        function(m,row)
    local  f,q;
    f := FieldOfMTX64Matrix(m);    
    q := MTX64_FieldOrder(f);
    if q = 2 then
        return MTX64_ExtractVecGF2(m, row);        
    elif q<= 2^16 then
        return MTX64_ExtractVecFFE(m,row);
    else
        return List([1..MTX64_Matrix_NumRows(m)], i->
                    FFEfromFELT(m[row+1,i]));
    fi;
end);

InstallOtherMethod(\[\], [IsMTX64Matrix, IsPosInt], 
        function(m,i)
    return MTX64_ExtractVector(m, i-1);
end);

        
        

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

InstallMethod( ShallowCopy, "for a meataxe matrix", [IsMTX64Matrix], MTX64_ShallowCopyMatrix);

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

InstallMethod(\*, "for meataxe64 matrix and FELT", [IsMTX64Matrix, IsMTX64FiniteFieldElement],
        function(m,x)
    local  copy;
    if FieldOfMTX64Matrix(m) <> MTX64_FieldOfElement(x) then
        #
        # Could possibly write a family predicate for this
        #
        Error("incompatible fields");
    fi;
    copy := ShallowCopy(m);    
    MTX64_DSMul(MTX64_Matrix_NumRows(m), x, copy);
    return copy;
end);

InstallMethod(\*, "for FELT and meataxe64 matrix ", [IsMTX64FiniteFieldElement, IsMTX64Matrix ],
        function(x,m)
    return m*x;
end);

    
        

InstallOtherMethod(\[\], "for a meataxe64 matrix and two indices", [IsMTX64Matrix, IsPosInt, IsPosInt], 
        function(m, i, j)
    if i > MTX64_Matrix_NumRows(m) or 
       j > MTX64_Matrix_NumCols(m) then
        Error("Indices out of range");
    fi;
    return MTX64_GetEntry(m, i-1, j-1);
end);


InstallOtherMethod(\[\]\:\=, "for a meataxe64 matrix and two indices and a FELT", 
        [IsMTX64Matrix and IsMutable, IsPosInt, IsPosInt, IsMTX64FiniteFieldElement], 
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
        m->MakeImmutable(TransposedMatMutable(m)));

InstallMethod(Display, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    Display(List([1..MTX64_Matrix_NumRows(m)], i->
            List([1..MTX64_Matrix_NumCols(m)], j -> MTX64_ExtractFieldElement(m[i,j]))));
    
end );
      

#
# Richard plans to build the copying into the slab level
# so this extra indirection can eventually go away.
#
BindGlobal("MTX64_SLEchelize",
        a -> MTX64_SLEchelizeDestructive(ShallowCopy(a)));


InstallMethod(InverseMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local  len, copy, res;
    len := MTX64_Matrix_NumCols(m);
    if len <> MTX64_Matrix_NumRows(m) then
        Error("not square");
    fi;
    res := MTX64_SLEchelize(m);
    if res.rank <> len then
        Error("not invertible");
    fi;
    return -res.multiplier;    
end);

InstallMethod(InverseSameMutability, "for a meataxe64 matrix", 
        [IsMTX64Matrix],
        function(m)
    if not IsMutable(m) then
        return Inverse(m);
    else
        return InverseMutable(m);        
    fi;
end);

InstallMethod(AdditiveInverseSameMutability, "for a meataxe64 matrix", 
        [IsMTX64Matrix],
        function(m)
    if not IsMutable(m) then
        return AdditiveInverse(m);
    else
        return AdditiveInverseMutable(m);        
    fi;
end);


InstallMethod(\+, "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    local  nrows, ncols, m;
    nrows := MTX64_Matrix_NumRows(m1);
    ncols := MTX64_Matrix_NumCols(m1);
    if nrows <> MTX64_Matrix_NumRows(m2) or 
        ncols <> MTX64_Matrix_NumCols(m2) then
        Error("matrices not the same size");
    fi;
    m := MTX64_NewMatrix(FieldOfMTX64Matrix(m1), ncols, nrows);
    MTX64_DAdd(nrows, m1,m2,m);
    return m;
end);

InstallMethod(\-, "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    local  nrows, ncols, m;
    nrows := MTX64_Matrix_NumRows(m1);
    ncols := MTX64_Matrix_NumCols(m1);
    if nrows <> MTX64_Matrix_NumRows(m2) or 
        ncols <> MTX64_Matrix_NumCols(m2) then
        Error("matrices not the same size");
    fi;
    m := MTX64_NewMatrix(FieldOfMTX64Matrix(m1), ncols, nrows);
    MTX64_DSub(nrows, m1,m2,m);
    return m;
end);

InstallMethod(AdditiveInverseMutable, "for meataxe64 matrices",
        [IsMTX64Matrix],
        m-> -One(FieldOfMTX64Matrix(m))*m);

InstallMethod(ZeroMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        m ->  MTX64_NewMatrix(FieldOfMTX64Matrix(m), MTX64_Matrix_NumCols(m),
                MTX64_Matrix_NumRows(m)));


MTX64_IdentityMat := function(n, field)
    local  m, o, i;
    m := MTX64_NewMatrix(field,n,n);
    o := One(field);    
    for i in [1..n] do 
        m[i,i] := o;
    od;
    return m;
    
end;


InstallMethod(OneMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local  n;
    n := MTX64_Matrix_NumRows(m);
    if n <> MTX64_Matrix_NumCols(m) then
        Error("Not square");
    fi;
    return MTX64_IdentityMat(n,FieldOfMTX64Matrix(m));
end);

InstallMethod(\=,  "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    return MTX64_Matrix_NumCols(m1) = MTX64_Matrix_NumCols(m2) and
           MTX64_Matrix_NumRows(m1) = MTX64_Matrix_NumRows(m2) and
           0 = MTX64_compareMatrices(m1,m2);
end);

InstallMethod(\<,  "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    if MTX64_Matrix_NumCols(m1) <> MTX64_Matrix_NumCols(m2) or
       MTX64_Matrix_NumRows(m1) <> MTX64_Matrix_NumRows(m2) then
        Error("No ordering for matrices of different shapes");
    fi;
    return 0 > MTX64_compareMatrices(m1,m2);
end);



#
# BitStrings
#

InstallMethod( ViewString, "for a meataxe64 bitstring",
        [IsMTX64BitString],
        bs -> STRINGIFY("< MTX64 bitstring ",MTX64_WeightOfBitString(bs),"/",MTX64_LengthOfBitString(bs),">"));

InstallMethod( ShallowCopy, "for a meataxe bitstring", [IsMTX64BitString], MTX64_ShallowCopyBitString);


InstallOtherMethod(Length, "for a meataxe bitstring", [IsMTX64BitString],
        MTX64_LengthOfBitString);

InstallOtherMethod(\[\], "for a meataxe bitstring", [IsMTX64BitString, IsPosInt],
        function(bs, i)
    if i > Length(bs) then
        Error("Index too big");
    fi;
    return MTX64_GetEntryOfBitString(bs, i-1);
end);

InstallOtherMethod(\[\]\:\=, "for a meataxe bitstring", [IsMTX64BitString, IsPosInt, IsInt],
        function(bs, i, x)
    if i > Length(bs) then
        Error("Index too big");
    fi;
    if x <> 1 then
        Error("meataxe64 bitstring entries can ONLY be set to 1");
    fi;
    MTX64_SetEntryOfBitString(bs, i-1);
end);

InstallMethod(\=,  "for meataxe bitstrings",
        [IsMTX64BitString, IsMTX64BitString],
        function(bs1,bs2) 
    return 0 = MTX64_compareBitStrings(bs1,bs2);
end);

InstallMethod(\<,  "for meataxe bitstrings",
        [IsMTX64BitString, IsMTX64BitString],
        function(bs1,bs2) 
    return 0 > MTX64_compareBitStrings(bs1,bs2);
end);


