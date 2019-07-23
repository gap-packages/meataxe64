#
# VectorObj objects that wrap meataxe 64 matrices
#

#
# The idea is to hide the MTX64 FELT's completely and pretend to be arrays of GAP FFEs
# vectors will just be wrapped matrices with one row. 
# we'll also need "row reference" objects although it seems those should be 
# generated generically
#
# we're going with attribute storing. Meataxe64  is never a good choice for small objects anyway.
#




BindGlobal("MakeMeataxe64Vector",
        function(m)
    local  f, q, bd, r;
    if not IsMTX64Matrix(m) or MTX64_Matrix_NumRows(m) <> 1 then
        Error("MakeMeataxe64Vector: argument must be a matrix with one row");
    fi;
    f := MTX64_FieldOfMatrix(m);
    q := MTX64_FieldOrder(f);
    bd := GF(q);
    r := rec();
    ObjectifyWithAttributes(r,
            NewType(FamilyObj(bd), IsAttributeStoringRep and IsMutable and 
                    IsMeataxe64VectorObj and HasBaseDomain), 
            BaseDomain, bd, UnderlyingMeataxe64Matrix, m);
    return r;    
end);


InstallMethod(Length, [IsMeataxe64VectorObj],
        v-> MTX64_Matrix_NumCols(UnderlyingMeataxe64Matrix(v)));

InstallMethod(\[\], [IsMeataxe64VectorObj, IsPosInt],
        {v,i} -> FFEfromFELT(MTX64_GetEntry(UnderlyingMeataxe64Matrix(v), 0, i-1)));

InstallMethod(\[\]\:\=, IsCollsXElms, [IsMeataxe64VectorObj, IsPosInt, IsFFE],
        function(v,i,x)
    local  m, f, z;
    m := UnderlyingMeataxe64Matrix(v);
    f := MTX64_FieldOfMatrix(m);
    z := MTX64_FiniteFieldElement(f, x);
    MTX64_SetEntry(m,0,i-1,z);
    return;
end);

InstallMethod(\{\}, [IsMeataxe64VectorObj, IsEmpty and IsList], 
        {v,r} -> MakeMeataxe64Vector(MTX64_NewMatrix(MTX64_FieldOfMatrix(UnderlyingMeataxe64Matrix(v)),1,0)));

InstallMethod(\{\}, [IsMeataxe64VectorObj, IsRange], 
        function(v,r)
    if Length(r) = 1 or r[2]-r[1] = 1 then
        return MakeMeataxe64Vector(MTX64_Submatrix(UnderlyingMeataxe64Matrix(v),1,1,r[1],Length(r)));
    else
        TryNextMethod();
    fi;
end);

InstallMethod(\{\}, [IsMeataxe64VectorObj, IsList], 
        function(v,r)
    local  u, sub, i;
    u := UnderlyingMeataxe64Matrix(v);
    sub := MTX64_NewMatrix(MTX64_FieldOfMatrix(u),1,Length(r));
    for i in [1..Length(r)] do
        MTX64_SetEntry(sub, 0, i-1, MTX64_GetEntry(u, 0, r[i]-1));
    od;
    return MakeMeataxe64Vector(sub);
end);

InstallMethod(PositionNonZero, [IsMeataxe64VectorObj],
        function(v)
    local  u, p;
    u := UnderlyingMeataxe64Matrix(v);
    p := MTX64_DNzl(u,0);
    if p = fail then
        return Length(v)+1;
    else
        return p+1;
    fi;
end);

InstallMethod(PositionLastNonZero, [IsMeataxe64VectorObj],
        function(v)
    local  u, len, z, i;
    u := UnderlyingMeataxe64Matrix(v);
    len := MTX64_Matrix_NumCols(u);
    z := Zero(MTX64_FieldOfMatrix(u));    
    for i in [len-1, len-2..0] do
        if MTX64_GetEntry(u,0,i) <> z then
            return i+1;
        fi;
    od;
    return 0;
end);

InstallMethod(ListOp, [IsMeataxe64VectorObj],
        v->List([1..Length(v)], i->v[i]));

InstallMethod(ListOp, [IsMeataxe64VectorObj, IsFunction],
        {v,f}->List([1..Length(v)], i->f(v[i])));

InstallMethod(Unpack, [IsMeataxe64VectorObj],
        v->List([1..Length(v)], i->v[i]));

InstallMethod(ExtractSubVector, [IsMeataxe64VectorObj, IsList],
        {v, poss} -> v{poss});

InstallMethod(ShallowCopy, [IsMeataxe64VectorObj],
        v->MakeMeataxe64Vector(ShallowCopy(UnderlyingMeataxe64Matrix(v))));

InstallMethod(ViewString, [IsMeataxe64VectorObj],
        v->STRINGIFY("<mtx64 vector obj, len ",Length(v)," over GF(",Size(BaseDomain(v)),")>"));

InstallMethod(Display, [IsMeataxe64VectorObj],
        function(v)
    Display(UnderlyingMeataxe64Matrix(v));
end);

InstallMethod(\+, IsIdenticalObj, [IsMeataxe64VectorObj, IsMeataxe64VectorObj],
        {v1,v2} -> MakeMeataxe64Vector(UnderlyingMeataxe64Matrix(v1)+UnderlyingMeataxe64Matrix(v2)));

InstallMethod(\-, IsIdenticalObj, [IsMeataxe64VectorObj, IsMeataxe64VectorObj],
        {v1,v2} -> MakeMeataxe64Vector(UnderlyingMeataxe64Matrix(v1)-UnderlyingMeataxe64Matrix(v2)));

InstallMethod(\<, IsIdenticalObj, [IsMeataxe64VectorObj, IsMeataxe64VectorObj],
        {v1,v2} -> UnderlyingMeataxe64Matrix(v1) < UnderlyingMeataxe64Matrix(v2));

InstallMethod(\=, IsIdenticalObj, [IsMeataxe64VectorObj, IsMeataxe64VectorObj],
        {v1,v2} -> UnderlyingMeataxe64Matrix(v1) = UnderlyingMeataxe64Matrix(v2));

InstallMethod(AddRowVector, [IsMeataxe64VectorObj and IsMutable, IsMeataxe64VectorObj],
        function(v1,v2)
    AddRowVector(v1,v2, One(BaseDomain(v1)));
end);

InstallMethod(AddRowVector, IsCollsCollsElms,[IsMeataxe64VectorObj and IsMutable, IsMeataxe64VectorObj, IsFFE],
        function(v1,v2, x)
    local  u1;
    u1 := UnderlyingMeataxe64Matrix(v1);    
    MTX64_DSMad(1,u1, UnderlyingMeataxe64Matrix(v2), MTX64_FiniteFieldElement(MTX64_FieldOfMatrix(u1),x));
end);

InstallMethod(AddRowVector, IsCollsCollsElmsXX,[IsMeataxe64VectorObj and IsMutable, IsMeataxe64VectorObj, IsFFE, IsPosInt, IsPosInt],
        function(v1,v2, x, from, to)
    #
    # This might be slower if only a few positions in a very long vector are 
    # being changed, but otherwise not. 
    #
    AddRowVector(v1,v2,x);
end);

InstallMethod(MultVector, IsCollsElms, [IsMeataxe64VectorObj and IsMutable, IsFFE],
        function(v,x)
    local  u;
    u := UnderlyingMeataxe64Matrix(v);    
    MTX64_DSMad(1,MTX64_FiniteFieldElement(MTX64_FieldOfMatrix(u),x), u);
end);

InstallMethod(\*, IsCollsElms, [IsMeataxe64VectorObj, IsFFE],
        function(v,x)
    local  u, f;
    u := UnderlyingMeataxe64Matrix(v);
    f := MTX64_FieldOfMatrix(u);
    return MakeMeataxe64Vector(u*MTX64_FiniteFieldElement(f,x));
end);

InstallMethod(\*, IsElmsColls, [IsFFE, IsMeataxe64VectorObj],
        {x,v} -> v*x);


InstallMethod(QUO, IsCollsElms, [ IsMeataxe64VectorObj, IsFFE],
        {v,x} -> v*(x^-1));

InstallMethod(AdditiveInverseMutable, [IsMeataxe64VectorObj],
        v->MakeMeataxe64Vector(-UnderlyingMeataxe64Matrix(v)));

InstallMethod(AdditiveInverseImmutable, [IsMeataxe64VectorObj],
        v->MakeImmutable(MakeMeataxe64Vector(-UnderlyingMeataxe64Matrix(v))));

InstallMethod(AdditiveInverseSameMutability, [IsMeataxe64VectorObj],
        function(v)
    if IsMutable(v) then
        return AdditiveInverseMutable(v);
    else
        return AdditiveInverseImmutable(v);
    fi;
end);

InstallMethod(ZeroMutable, [IsMeataxe64VectorObj],
        v->MakeMeataxe64Vector(ZeroMutable(UnderlyingMeataxe64Matrix(v))));

InstallMethod(ZeroImmutable, [IsMeataxe64VectorObj],
        v->MakeImmutable(MakeMeataxe64Vector(ZeroImmutable(UnderlyingMeataxe64Matrix(v)))));

InstallMethod(ZeroSameMutability, [IsMeataxe64VectorObj],
        function(v)
    if IsMutable(v) then
        return ZeroMutable(v);
    else
        return ZeroImmutable(v);
    fi;
end);

InstallMethod(IsZero, [IsMeataxe64VectorObj],
        v->IsZero(UnderlyingMeataxe64Matrix(v)));

InstallMethod(Characteristic, [IsMeataxe64VectorObj],
        v->Characteristic(BaseDomain(v)));

InstallMethod(ZeroVector, [IsInt, IsMeataxe64VectorObj],
  function(len, v)
    local  f;
    f := MTX64_FieldOfMatrix(UnderlyingMeataxe64Matrix(v));
    return MakeMeataxe64Vector(MTX64_NewMatrix(f, 1, len));
end);


InstallMethod(ZeroVector, [IsInt, IsMeataxe64MatrixObj],
  function(len, m)
    local  f;
    f := MTX64_FieldOfMatrix(UnderlyingMeataxe64Matrix(m));
    return MakeMeataxe64Vector(MTX64_NewMatrix(f, 1, len));
end);

InstallMethod(Vector, [IsList, IsMeataxe64VectorObj],
        function(l,v)
    local  f,w;
    f := MTX64_FieldOfMatrix(UnderlyingMeataxe64Matrix(v));
    w := MTX64_NewMatrix(f,1,Length(l));
    MTX64_InsertVector(w,l,0);
    return MakeMeataxe64Vector(w);
end);

InstallMethod(ConstructingFilter, [IsMeataxe64VectorObj],
        v->IsMeataxe64VectorObj);

InstallMethod(NewVector, [IsMeataxe64VectorObj, IsField and IsFinite, IsList],
        function (t, f, l)
    local  m;
    m := MTX64_NewMatrix(MTX64_FiniteField(Size(f)), 1, Length(l));
    MTX64_InsertVector(m,l,0);
    return MakeMeataxe64Vector(m);
end);

InstallMethod(NewZeroVector, [IsMeataxe64VectorObj, IsField and IsFinite, IsInt],
        function (t, f, len)
    local  m;
    m := MTX64_NewMatrix(MTX64_FiniteField(Size(f)), 1,len);
    return MakeMeataxe64Vector(m);
end);

InstallMethod(ChangedBaseDomain, IsIdenticalObj, [IsMeataxe64VectorObj, IsFinite and IsField],
        function(v, f)
    local  m;
    m := MTX64_NewMatrix(MTX64_FiniteField(Size(f)), 1, Length(v));
    MTX64_InsertVector(m, MTX64_ExtractVector(UnderlyingMeataxe64Matrix(v),0),0);
    return MakeMeataxe64Vector(m);
end);


InstallMethod(ScalarProduct, IsIdenticalObj, [IsMeataxe64VectorObj, IsMeataxe64VectorObj],
        function(v1,v2)
    local  x;
    x :=  UnderlyingMeataxe64Matrix(v2)*MTX64_SLTranspose(UnderlyingMeataxe64Matrix(v1));
    return FFEfromFELT(x[1,1]);
end);

InstallMethod(Randomize, [IsMeataxe64VectorObj and IsMutable, IsRandomSource],
        function(v, rs)
    local  u, f, q, m, i;
    u := UnderlyingMeataxe64Matrix(v);
    f := MTX64_FieldOfMatrix(u);
    q := MTX64_FieldOrder(f);
    m := MTX64_Matrix_NumCols(u);
    for i in [0..m-1] do
        MTX64_SetEntry(u, 0, i, MTX64_FiniteFieldElement(f, Random(rs, [0..q-1])));
    od;
end);

InstallMethod(Randomize, [IsMeataxe64VectorObj and IsMutable],
        function(v)
    Randomize(v, GlobalMersenneTwister);
end);

InstallMethod(CopySubVector, [IsMeataxe64VectorObj, IsMeataxe64VectorObj and IsMutable, IsList, IsList],
        function(src, dst, scols, dcols)
    local  us, ud, v, i;
    if Length(scols) <> Length(dcols) then
        Error("CopySubVector, index lists are not the same length");
    fi;
    if Length(scols) = 0 then
        return;
    fi;
    if Length(scols) = 1 then
        dst[dcols[1]] := src[scols[1]];
    fi;
    us := UnderlyingMeataxe64Matrix(src);
    ud := UnderlyingMeataxe64Matrix(dst);
    
    if IsRange(scols) and IsRange(dcols) and
       scols[2]-scols[1] = 1 and dcols[2] - dcols[1] = 1 then
        v := MTX64_NewMatrix(MTX64_FieldOfMatrix(us),Length(scols));
        MTX64_DCut(us,0,1,scols[1]-1,v);
        MTX64_DPaste(v,0,1,dcols[1]-1,ud);
        return;
    fi;
    
    for i in [1..Length(scols)] do
        MTX64_SetEntry(ud, 0, dcols[i]-1, MTX64_GetEntry(us, 0, scols[i]-1));
    od;
end);

InstallMethod(WeightOfVector, [IsMeataxe64VectorObj],
        function(v)
    local  u, wt, i;
    u := UnderlyingMeataxe64Matrix(v);
    wt := 0;    
    for i in [0..MTX64_Matrix_NumCols-1] do
        if 0 <> MTX64_ExtractFieldElement(MTX64_GetEntry(u,0,i)) then
            wt := wt+1;
        fi;
    od;
    return wt;
end);

InstallMethod(DistanceOfVectors, [IsMeataxe64VectorObj,IsMeataxe64VectorObj],
        {v1,v2} -> WeightOfVector(v1-v2));

        
        
        

    
        



    







    
