#
# meataxe64: meataxe64
#
# Implementations
#
#! @Chapter datatypes
#! @Section Standard Operations applicable to Meataxe64 objects
#! 
#! Methods are installed for many standard operations applied to Meataxe64
#! objects. In this section we briefly list the more important ones  with 
#! some notes where the behaviour may not be as expected.
#!
#! <List>
#! <Item><Ref Oper="&lt;" BookName="ref"/> for Meataxe64 fields orders fields by their size. For
#! Meataxe64 finite field elements, it orders them according to the internal
#! numbering used by the C meataxe.</Item>
#!
#! <Item> Standard arithmetic operations such as <Ref Attr="Zero" BookName="ref"/> and <Ref
#! Attr="*" BookName="ref"/> are installed for Meataxe64 finite field elements, and where
#! relevant for the fields.</Item>
#!
#! <Item>A limited set of collection operations such as <Ref Attr="Size" BookName="ref"/>, <Ref
#! Attr="AsList" BookName="ref"/>, <Ref Attr="AsSSortedList" BookName="ref"/> and <Ref Attr="Random" BookName="ref"/> are
#! provided for Meataxe64 finite fields, for convenience, although such fields
#! are not properly collections.</Item>
#! 
#! <Item>The list access operation <C>\[\]</C> can be used to do the same
#! thing as <Ref Func="MTX64_ExtractVector"/>, except that in this case the row
#! indexing is one based.</Item>
#! 
#! <Item>The list assignment operation <C>\[\]\:\=</C> can be used to do the same
#! thing as <Ref Func="MTX64_InsertVector"/>, except that in this case the row
#! indexing is one based.</Item>
#! 
#! <Item><Ref Oper="ShallowCopy" BookName="ref"/> applied to a Meataxe64 matrix produces a new
#! matrix which does not share its rows with the original, unlike standard
#! &GAP;  matrices.</Item>
#! 
#! <Item>Arithmetic operations are installed for operations among Meataxe64 matrices and
#! between matrices and finite field elements. There is no automatic coercion
#! between fields, and matrix dimensions must match correctly (unbound entries
#! are not treated as zero). This includes transposition.</Item>
#!
#! <Item><Ref Oper="MatElm" BookName="ref"/> and <Ref Oper="SetMatElm" BookName="ref"/> methods are installed for 
#! matrices which do the same as <Ref Func="MTX64_GetEntry"/> and
#! <Ref Func="MTX64_SetEntry"/> but using one-based indexing. This supports access
#! like <C>m[i,j]</C> for reading and writing.</Item>
#! 
#! <Item>Meataxe64 matrices are only equal or comparable with <Ref Oper="&lt;" BookName="ref"/> if they
#! are defined over the same field and of the same shape. The ordering is a
#! linear ordering, but is not otherwise defined.</Item>
#!
#! <Item>List access via <C>\[\]</C> and <C>\[\]\:\=</C> is supported for
#! bit strings, with the bits returned as the integers 0 and 1. Note
#! however that bits can only be set to 1, not to 0. Indices in this case
#! are one-based. </Item>
#! 
#! 
#! </List>

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

BindGlobal("MTX64_FieldEltType", MemoizeFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_ElementFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMTX64FiniteFieldElement and IsDataObjectRep);
end, rec(flush := false)));

BIND_GLOBAL("MTX64_MatrixType",MemoizeFunction(function(q)
    local fam, type;
    fam := NewFamily(STRINGIFY("MTX64_GF(", q, ")_MatrixFamily"));
    fam!.q := q;
    fam!.field := MTX64_FiniteField(q);    
    return NewType(fam, IsMutable and IsMTX64Matrix and IsDataObjectRep);
end, rec(flush := false)));

InstallGlobalFunction(MTX64_FieldOfMatrix, m -> FamilyObj(m)!.field);
InstallGlobalFunction(MTX64_FieldOfElement, e -> FamilyObj(e)!.field);

#
# Fields
#

InstallMethod( MTX64_FiniteField, "for a size",
               [ IsPosInt ],
        MemoizeFunction(function(q)
    if  q >= 2^64 or not IsPrimePowerInt(q) then
        Error("MTX64_FiniteField: field order must be a prime power < 2^64");
    fi;
    return MTX64_CREATE_FIELD(q);
end,
  rec(flush := false)) );

InstallMethod( MTX64_FiniteField, "for a characteristic, and a degree",
               [ IsPosInt, IsPosInt ],
function(char, deg)
    return MTX64_FiniteField(char ^ deg);
end);

InstallOtherMethod(Size, [IsMTX64FiniteField],
        MTX64_FieldOrder);


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

BindGlobal("MTX64_HashField", f->HashBasic(MTX64_FieldOrder(f),"Meataxe64_Field"));


#
## Elements of finite fields
#

InstallMethod( MTX64_FiniteFieldElement, "for a meataxe64 field and an integer",
        [ IsMTX64FiniteField, IsInt ],        
        MTX64_CreateFieldElement);

BindGlobal("MTX64_ConversionTables", MemoizeFunction(function(q)
    local  f, tab1, tab2, z, y, i, tab3, tab4;
    f := MTX64_FiniteField(q);    
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
    return [tab1,tab2];
end));

BindGlobal("MTX64_ByteTables", MemoizeFunction(function(q)
    local  tab3, tab4, i, f;
    f := MTX64_FiniteField(q);    
    tab3 := MTX64_Make8BitConversion(f);
    ConvertToStringRep(tab3);    
    tab4 := [];
    for i in [1..Length(tab3)] do
        tab4[IntChar(tab3[i])+1] := CharInt(i-1);
    od;
    ConvertToStringRep(tab4);    
    return [tab3,tab4];
end));
    

BindGlobal("MTX64_GetFFEfromFELTTable", function(f)
    return MTX64_ConversionTables(MTX64_FieldOrder(f))[2];    
end);

    
BindGlobal("MTX64_GetFELTfromFFETable", function(f)
    return MTX64_ConversionTables(MTX64_FieldOrder(f))[1];    
end);

BindGlobal("MTX64_Get8BitImportTable", function(f)
    return MTX64_ByteTables(MTX64_FieldOrder(f))[1];    
end);

BindGlobal("MTX64_Get8BitExportTable", function(f)
    return MTX64_ByteTables(MTX64_FieldOrder(f))[2];    
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
    if p^d <= MAXSIZE_GF_INTERNAL then
        tab := MTX64_GetFELTfromFFETable(field);
        if IsZero(ffe) then
            x := 0;
        else 
            x := tab[LogFFE(ffe,Z(p,d))+2];
        fi;
    elif d = 1 then
        x := IntFFE(ffe);
    elif IsCoeffsModConwayPolRep(ffe) and ffe![2] = d and Length(ffe![1]) = d then
        # Ugly dependency on the Conway Pol internal rep
        x := NumberFFVector(Reversed(ffe![1]),p);
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

InstallMethod(IsZero, "for a meataxe64 field element",
        [IsMTX64FiniteFieldElement],
        x -> MTX64_ExtractFieldElement(x) = 0);


InstallMethod(IsOne, "for a meataxe64 field element",
        [IsMTX64FiniteFieldElement],
        x -> MTX64_ExtractFieldElement(x) = 1);

InstallMethod(\+, "meataxe64 field elements", IsIdenticalObj,   [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldAdd(MTX64_FieldOfElement(x), x, y);    
end);

InstallMethod(\-, "meataxe64 field elements", IsIdenticalObj,  [IsMTX64FiniteFieldElement, IsMTX64FiniteFieldElement],
        function(x,y)
    return MTX64_FieldSub(MTX64_FieldOfElement(x), x, y);    
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

InstallMethod(InverseOp,  "meataxe64 field element",  
        [IsMTX64FiniteFieldElement],
    x ->  MTX64_FieldInv(MTX64_FieldOfElement(x), x));



FFEfromFELT := function(felt)
    local  fld, p, d, x, tab, v, zp, z, fam, i;
    fld := MTX64_FieldOfElement(felt);    
    p := MTX64_FieldCharacteristic(fld);
    d := MTX64_FieldDegree(fld);
    x := MTX64_ExtractFieldElement(felt);
    if p^d <= MAXSIZE_GF_INTERNAL then
        tab := MTX64_GetFFEfromFELTTable(fld);
        return tab[x+1];
    fi;
    if d = 1 then
        return Z(p)^0*x;
    fi;
    v := [];
    zp := Z(p);    
    for i in [1..d] do
        Add(v, zp* (x mod p));
        x := QuoInt(x,p);
    od;
    ConvertToVectorRep(v,p);
    z := Z(p,d);
    fam := FamilyObj(z);
    return Objectify(fam!.ConwayFldEltDefaultType, [v, d, fail]);    
end;


InstallOtherMethod(Zero, "for a meataxe64 field",
        [IsMTX64FiniteField],
        f->MTX64_FiniteFieldElement(f,0));

InstallOtherMethod(One, "for a meataxe64 field",
        [IsMTX64FiniteField],
        f->MTX64_FiniteFieldElement(f,1));


BindGlobal("MTX64_HashFELT", 
        x->HashBasic( MTX64_FieldOrder(MTX64_FieldOfElement(x)), MTX64_ExtractFieldElement(x), "MTX64 FELT"));

#
# MTX64 fields as collections
#

InstallOtherMethod(AsList, [IsMTX64FiniteField ],
        f -> List([0..Size(f)-1], i-> MTX64_FiniteFieldElement(f,i)));

InstallOtherMethod(AsSSortedList, [IsMTX64FiniteField ],
        f -> List([0..Size(f)-1], i-> MTX64_FiniteFieldElement(f,i)));

InstallOtherMethod(Random, [IsRandomSource, IsMTX64FiniteField],
        {rs, f} -> MTX64_FiniteFieldElement(f, Random(rs, [0..MTX64_FieldOrder(f)-1])));

InstallOtherMethod(Random, [IsMTX64FiniteField],
        f -> Random(GlobalMersenneTwister, f));


#
# Matrices
#
#
# row is zero based
#

InstallGlobalFunction( "MTX64_InsertVector",
        function(m,v,row)
    local  f, i;
    if IsGF2VectorRep(v) and fail <> MTX64_InsertVecGF2(m,v,row) then
        return;
    fi;
    if Is8BitVectorRep(v) and fail <> MTX64_InsertVec8Bit(m,v,row) then
        return;
    fi;    
    if IS_VECFFE(v) and fail <> MTX64_InsertVecFFE(m, v, row) then
        return;
    fi;
    f := MTX64_FieldOfMatrix(m);    
    if Length(v) <> MTX64_NumCols(m) then
        Error("MTX64_InsertVector: row length mismatch");
    fi;
    for i in [1..Length(v)] do
        m[row+1, i] := MTX64_FiniteFieldElement(f, v[i]);
    od;
    return;
end);

InstallOtherMethod(\[\]\:\=, [IsMTX64Matrix and IsMutable, IsPosInt, IsRowVector and IsFFECollection],
                          function(m,i,v)
    MTX64_InsertVector(m,v,i-1);
end);

InstallMethod(MTX64_Matrix, [IsMatrix and IsFFECollColl],
        function(m)
    local f;    
    f := DefaultFieldOfMatrix(m);
    return MTX64_Matrix(m, f);
end);

InstallMethod(MTX64_Matrix, [IsMatrix and IsFFECollColl,
            IsField and IsFinite and IsFFECollection],
        function(m, f)
    return MTX64_Matrix(m,Size(f));
end);

InstallOtherMethod(MTX64_Matrix, [IsList,
            IsField and IsFinite and IsFFECollection],
        function(m, f)
    return MTX64_Matrix(m,Size(f));
end);

    

InstallMethod(MTX64_Matrix, [IsMatrix and IsFFECollColl,
            IsPosInt],
        function(m, q)
    local  nor, noc, fld, mm, i;
    nor := Length(m);
    Assert(2,nor > 0);
    noc := Length(m[1]);
    Assert(2, noc > 0);
    return MTX64_Matrix(m, q, nor, noc);
end);

InstallOtherMethod(MTX64_Matrix, [IsList,
            IsPosInt],
        function(m, q)
    local  nor, noc, fld, mm, i;
    nor := Length(m);
    if nor = 0 then
        TryNextMethod();
    fi;
    if ForAny(m, x-> x<> []) then
        TryNextMethod();
    fi;
    return MTX64_NewMatrix(MTX64_FiniteField(q), nor, 0);    
end);

InstallMethod(MTX64_Matrix, [IsMatrix and IsFFECollColl,
        IsPosInt, IsInt, IsInt],
        function(m,q,nor,noc)
    local fld, mm, i;    
    Assert(1, Length(m) = nor);
    Assert(1, nor = 0 or Length(m[1]) = noc);
    Assert(2, nor = 0 or noc = 0 or q mod Characteristic(m) = 0);    
    fld := MTX64_FiniteField(q);        
    mm := MTX64_NewMatrix(fld, nor, noc);
    for i in [1..nor] do
        MTX64_InsertVector(mm,m[i], i-1);        
    od;
    return mm;
end);

InstallOtherMethod(MTX64_Matrix, [IsList,
        IsPosInt, IsInt, IsInt],
        function(m,q,nor,noc)
    local fld;    
    Assert(1, noc = 0);
    Assert(1, Length(m) = nor);
    Assert(2, ForAll(m, x-> x = []));    
    fld := MTX64_FiniteField(q);        
    return MTX64_NewMatrix(fld, nor, noc);
end);
        

InstallGlobalFunction("MTX64_RandomMat", 
        function(f, n, m)
    return MTX64_RANDOM_MAT( f, n , m, GlobalMersenneTwister!.state);
end);

    

#
# row is zero based
#
InstallGlobalFunction( "MTX64_ExtractVector", 
        function(m,row)
    local  f,q;
    f := MTX64_FieldOfMatrix(m);    
    q := MTX64_FieldOrder(f);
    if q = 2 then
        return MTX64_ExtractVecGF2(m, row);        
    elif q <= 256 then
        return MTX64_ExtractVec8Bit(m, row);        
    elif q<= MAXSIZE_GF_INTERNAL then
        return MTX64_ExtractVecFFE(m,row);
    else
        return List([1..MTX64_NumCols(m)], i->
                    FFEfromFELT(m[row+1,i]));
    fi;
end);

InstallOtherMethod(\[\], [IsMTX64Matrix, IsPosInt], 
        function(m,i)
    return MTX64_ExtractVector(m, i-1);
end);

InstallMethod(MTX64_ExtractMatrix, [IsMTX64Matrix],
        function(m)
    local  f, q, gm, len, i;
    f := MTX64_FieldOfMatrix(m);
    q := MTX64_FieldOrder(f);    
    gm := [];
    len := MTX64_NumRows(m);
    for i in [1..len] do
        gm[i] := MTX64_ExtractVector(m, i-1);
    od;
    if q < 256 then
        ConvertToMatrixRep(gm, q);
    fi;
    return gm;
end);

        
        

InstallMethod( ViewString, "for a meataxe64 matrix",
               [ IsMTX64Matrix ],
function(m)
    local f;
    f := MTX64_FieldOfMatrix(m);    
    return STRINGIFY("< matrix "
                   , MTX64_NumRows(m)
                   , "x"
                   , MTX64_NumCols(m)
                  , " : "
                  , ViewString(f)
                  , ">");
end);

#
# DisplayString would be better, but we need DisplayString for GAP matrices first
#
InstallMethod( Display, "for a meataxe64 matrix", [IsMTX64Matrix],
        function(m)
    Display(MTX64_ExtractMatrix(m));
end);


InstallMethod( ShallowCopy, "for a meataxe matrix", [IsMTX64Matrix], MTX64_ShallowCopyMatrix);

InstallOtherMethod(\*, "for meataxe64 matrices", IsIdenticalObj, [IsMTX64Matrix, IsMTX64Matrix], 
        MTX64_SLMultiply);

InstallMethod(\*, "for meataxe64 matrix and FELT", [IsMTX64Matrix, IsMTX64FiniteFieldElement],
        function(m,x)
    local  copy;
    copy := ShallowCopy(m);        
    MTX64_DSMul(MTX64_NumRows(m), x, copy);
    return copy;
end);

InstallMethod(\*, "for FELT and meataxe64 matrix ", [IsMTX64FiniteFieldElement, IsMTX64Matrix ],
        function(x,m)
    return m*x;
end);

    
        

InstallOtherMethod(MatElm, "for a meataxe64 matrix and two indices", [IsMTX64Matrix, IsPosInt, IsPosInt], 
        function(m, i, j)
    return MTX64_GetEntry(m, i-1, j-1);
end);


InstallOtherMethod(SetMatElm, "for a meataxe64 matrix and two indices and a FELT", 
        [IsMTX64Matrix and IsMutable, IsPosInt, IsPosInt, IsMTX64FiniteFieldElement], 
        function(m, i, j, x)
    MTX64_SetEntry(m, i-1, j-1, x);
end);

InstallOtherMethod(TransposedMatMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
    MTX64_SLTranspose);

InstallOtherMethod(TransposedMatImmutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        m->MakeImmutable(TransposedMatMutable(m)));


#
# Richard plans to build the copying into the slab level
# so this extra indirection can eventually go away.
#
BindGlobal("MTX64_SLEchelize",
        a -> MTX64_SLEchelizeDestructive(ShallowCopy(a)));


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
    local  nrows;
    nrows := MTX64_NumRows(m1);
    return MTX64_DAdd(nrows, m1,m2);
end);

InstallMethod(\-, "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    local  nrows;
    nrows := MTX64_NumRows(m1);
    return MTX64_DSub(nrows, m1,m2);
end);

InstallMethod(AdditiveInverseMutable, "for meataxe64 matrices",
        [IsMTX64Matrix],
        m-> -One(MTX64_FieldOfMatrix(m))*m);

InstallMethod(ZeroMutable, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        m ->  MTX64_NewMatrix(MTX64_FieldOfMatrix(m),
                MTX64_NumRows(m), MTX64_NumCols(m)));

InstallOtherMethod(IsZero, [IsMTX64Matrix],
        m -> ForAll([0..MTX64_NumRows(m)-1], i-> fail = MTX64_DNzl(m,i)));

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
    n := MTX64_NumRows(m);
    if n <> MTX64_NumCols(m) then
        Error("Not square");
    fi;
    return MTX64_IdentityMat(n,MTX64_FieldOfMatrix(m));
end);

InstallMethod(OneSameMutability, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local o;
    o := OneMutable(m);
    if not IsMutable(m) then
        MakeImmutable(o);
    fi;
    return o;
end);

InstallMethod(One, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local o;
    o := OneMutable(m);
    MakeImmutable(o);
    return o;
end);

InstallMethod(IsOne, "for a meataxe64 matrix",
        [IsMTX64Matrix],
        function(m)
    local  f, n, i, j;
    f := MTX64_FieldOfMatrix(m);
    n := MTX64_NumCols(m);
    if n <> MTX64_NumRows(m) then
        return false;
    fi;
    for i in [1..n-1] do
        for j in [i+1..n] do
            if not IsZero(m[i,j]) or not IsZero(m[j,i]) then
                return false;
            fi;
        od;
    od;
    return ForAll([1..n], i-> IsOne(m[i,i]));
end);

    
InstallMethod(\=,  "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    return MTX64_NumCols(m1) = MTX64_NumCols(m2) and
           MTX64_NumRows(m1) = MTX64_NumRows(m2) and
           0 = MTX64_CompareMatrices(m1,m2);
end);

InstallMethod(\<,  "for meataxe64 matrices", IsIdenticalObj,
        [IsMTX64Matrix, IsMTX64Matrix],
        function(m1,m2)
    if MTX64_NumCols(m1) <> MTX64_NumCols(m2) or
       MTX64_NumRows(m1) <> MTX64_NumRows(m2) then
        Error("No ordering for matrices of different shapes");
    fi;
    return 0 > MTX64_CompareMatrices(m1,m2);
end);


InstallGlobalFunction("MTX64_Submatrix",
        function(m, starty, leny, startx, lenx)
    local  nor, noc, sm;
    nor := MTX64_NumRows(m);
    noc := MTX64_NumCols(m);
    if startx = 1 and lenx = noc then
        sm := MTX64_NewMatrix(MTX64_FieldOfMatrix(m), leny, noc);
        MTX64_DCpy(m, sm, starty-1, leny);
    else
        sm := MTX64_NewMatrix(MTX64_FieldOfMatrix(m), leny, lenx);
        MTX64_DCut(m, starty-1, leny, startx-1, sm);
    fi;
    return sm;
end);



#
# BitStrings
#

InstallMethod( ViewString, "for a meataxe64 bitstring",
        [IsMTX64BitString],
        bs -> STRINGIFY("< MTX64 bitstring ",MTX64_WeightOfBitString(bs),"/",MTX64_LengthOfBitString(bs),">"));

InstallMethod( DisplayString, "for a meataxe64 bitstring",
        [IsMTX64BitString],
        function(bs)
    local  s, i;
    s := [];    
    for i in [1..MTX64_LengthOfBitString(bs)] do
        Add(s, String(MTX64_GetEntryOfBitString(bs,i-1)));
    od;
    return Concatenation("[", JoinStringsWithSeparator(s,","), "]\n");    
end);



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
    return 0 = MTX64_CompareBitStrings(bs1,bs2);
end);

InstallMethod(\<,  "for meataxe bitstrings",
        [IsMTX64BitString, IsMTX64BitString],
        function(bs1,bs2) 
    return 0 > MTX64_CompareBitStrings(bs1,bs2);
end);


InstallGlobalFunction("MTX64_RowSelect", function(bs,m)
    return MTX64_RowSelectShifted(bs,m,0);
end);

# This should probably be in a different file
#! @Chapter Parallel Computations
#! @Section File-based Parallel Algorithms
#! The C Meataxe64 includes a number of parallel implementations of
#! challenging computations. These take their inputs and return their
#! results in disk files, which can be read and written using <Ref
#! Func="MTX64_WriteMatrix"/> and <Ref Func="MTX64_ReadMatrix"/>. They use
#! a number of threads specified in the file <C>src/mtx64/tuning.h</C>.
#! Each requires a <A>tmpdir</A> parameter which should be the pathname of
#! a directory suitable for temporary files. 
#!
#! <ManSection> <Func Name="MTX64_fMultiply" Arg="tmpdir, fn1, fn2, fn3"/>
#! <Description> This function multiplies the matrices in files <A>fn1</A>
#! and <A>fn2</A> (in that order) and writes the result into file
#! <A>fn3</A>.</Description></ManSection> 
#!
#! <ManSection> <Func Name="MTX64_fMultiplyAdd" Arg="tmpdir, fn1, fn2, fn3,
#! fn4"/>
#! <Description> This function multiplies the matrices in files <A>fn1</A>
#! and <A>fn2</A> (in that order) adds the result to teh matrix in
#! <A>fn3</A> and writes the result into file
#! <A>fn4</A>.</Description></ManSection> 
#!
#! <ManSection> <Func Name="MTX64_fTranspose" Arg="tmpdir, fn1, fn2"/>
#! <Description> This function computes the transpose of the matrix in
#! filex <A>fn1</A>  and writes the result into file
#! <A>fn2</A>.</Description></ManSection> 
#!
#! <ManSection> <Func Name="MTX64_fProduceNREF" Arg="tmpdir, fn1, fn2, fn3"/>
#! <Description> This function computes the negative reduced echelon form
#! of the matrix in file <A>fn1</A>, which is returned in two parts. A
#! bitsstring in file <A>fn2</A> which indicates the locations of pivot
#! columns and a remnant in <A>fn3</A> which contains the entries from the
#! non-pivot columns of the pivot rows. The rank is returned </Description></ManSection> 
#!
#! <ManSection> <Func Name="MTX64_fEchelize" Arg="tmpdir, a, cs, rs, m, k, r"/>
#! <Description> This function computes the negative reduced echelon form
#! of the matrix in file <A>a</A>, which is returned in two parts. A
#! bitsstring in file <A>rs</A> which indicates the locations of pivot
#! columns and a remnant in <A>r</A> which contains the entries from the
#! non-pivot columns of the pivot rows.  In addition, the multiplier,
#! cleaner and row select are returned in files <A>m</A>, <A>k</A> and
#! <A>rs</A>, respectively. The rank is returned
#! </Description></ManSection>  
#!


