#
# MatrixObj objects that wrap meataxe 64 objects
#

#
# The idea is to hide the MTX64 FELT's completely and pretend to be arrays of GAP FFEs
# vectors will just be wrapped matrices with one row. 
# we'll also need "row reference" objects although it seems those should be generated generically
#
# we're going with attribute storing, so that we can store rank, inverse, echelization, etc.
#



BindGlobal("MakeMeataxe64Matrix",
        function(m)
    local  f, q, bd, r;
    if not IsMTX64Matrix(m) then
        Error("MakeMeataxe64Matrix: argument must be a matrix");
    fi;
    f := MTX64_FieldOfMatrix(m);
    q := MTX64_FieldOrder(f);
    bd := GF(q);
    r := rec( UnderlyingMeataxe64Matrix := m);
    ObjectifyWithAttributes(r,
            NewType(CollectionsFamily(FamilyObj(bd)), 
                    IsAttributeStoringRep and IsMutable and IsMeataxe64MatrixObj 
                    and HasBaseDomain), 
            BaseDomain, bd);
    return r;    
end);

InstallMethod(NumberRows,[IsMeataxe64MatrixObj],m->MTX64_NumRows(UnderlyingMeataxe64Matrix(m)));

InstallMethod(NumberColumns,[IsMeataxe64MatrixObj], m->MTX64_NumCols(UnderlyingMeataxe64Matrix(m)));

InstallMethod(MatElm, [IsMeataxe64MatrixObj, IsInt, IsInt], 
        {m, r, c} -> FFEfromFELT(MTX64_GetEntry(UnderlyingMeataxe64Matrix(m), r-1, c-1)));

InstallMethod(SetMatElm, [IsMeataxe64MatrixObj, IsInt, IsInt, IsFFE], 
        function(m, r, c, x)
    local  um, f;
    um := UnderlyingMeataxe64Matrix(m);
    f := MTX64_FieldOfMatrix(um);   
    MTX64_SetEntry( um, r-1, c-1, MTX64_FiniteFieldElement(f,x));
    end);

InstallMethod(\<, [IsMeataxe64MatrixObj, IsMeataxe64MatrixObj],
        {m1,m2} -> UnderlyingMeataxe64Matrix(m1) < UnderlyingMeataxe64Matrix(m2));
    
InstallMethod(ConstructingFilter, [IsMeataxe64MatrixObj], m->IsMeataxe64MatrixObj);

if IsBound(CompatibleVectorFilter) then
    InstallMethod(CompatibleVectorFilter, [IsMeataxe64MatrixObj], m-> IsMeataxe64VectorObj);
fi;


InstallMethod(NewZeroMatrix,[IsMeataxe64MatrixObj, IsField and IsFinite, IsInt, IsInt],
        {filt, R, nor, noc} -> MakeMeataxe64Matrix(MTX64_NewMatrix(MTX64_FiniteField(Size(R)), nor, noc)));



InstallMethod(NewMatrix,[IsMeataxe64MatrixObj, IsField and IsFinite, IsInt, IsList],
        function(filt, R, noc, list) 
    local  nor, m, i, l;
    if Length(list) = 0 then
        return MakeMeataxe64Matrix(MTX64_NewMatrix(MTX64_FiniteField(Size(R)), 0, noc));
    fi;
    if IsFFE(list[1]) then
        nor := QuoInt(Length(list),noc);
    else
        nor := Length(list);
    fi;
    m := MTX64_NewMatrix(MTX64_FiniteField(Size(R)), nor, noc);
    if IsFFE(list[1]) then
        for i in [1..nor] do
            MTX64_InsertVector(m, i, list{[noc*(i-1)+1..noc*i]});
        od;
    else
        for i in [1..nor] do
            l := list[i];
            if not IsList(l) then
                l := Unpack(l);
            fi;
            MTX64_InsertVector(m, l, i-1);            
        od;
    fi;
    return MakeMeataxe64Matrix(m);
end);

#
# Methods for performance
#
    

InstallMethod( \*,
        [ IsMeataxe64MatrixObj, IsFFE ],
        {m,x} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m)*x));

InstallMethod( \*,
        [ IsFFE, IsMeataxe64MatrixObj ],
        {x,m} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m)*x));

InstallMethod( \*, [IsMeataxe64MatrixObj, IsMeataxe64MatrixObj],
        {m1,m2} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m1)*
                UnderlyingMeataxe64Matrix(m2)));

InstallMethod( \*, [IsMeataxe64VectorObj, IsMeataxe64MatrixObj],
        {v,m} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(v)*
                UnderlyingMeataxe64Matrix(m)));

InstallMethod( \*, [IsMeataxe64MatrixObj, IsMeataxe64VectorObj],
        {m, v} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m)*
                UnderlyingMeataxe64Matrix(v)));


InstallMethod( \+, [IsMeataxe64MatrixObj, IsMeataxe64MatrixObj],
        {m1,m2} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m1)+
                UnderlyingMeataxe64Matrix(m2)));

InstallMethod( \-, [IsMeataxe64MatrixObj, IsMeataxe64MatrixObj],
        {m1,m2} -> MakeMeataxe64Matrix(UnderlyingMeataxe64Matrix(m1)-
                UnderlyingMeataxe64Matrix(m2)));

InstallMethod( AdditiveInverse, [IsMeataxe64MatrixObj],
        m -> MakeMeataxe64Matrix(-UnderlyingMeataxe64Matrix(m)));

InstallMethod( AdditiveInverse, [IsMeataxe64MatrixObj],
        m -> MakeMeataxe64Matrix(-UnderlyingMeataxe64Matrix(m)));



#
# completely generic method, should be elsewhere I think
#

InstallOtherMethod( Unpack, [IsMatrixObj],
        m -> List([1..NumberRows(m)], i -> List([1..NumberColumns(m)], j-> m[i,j])));



#
# Echelization based methods
#

InstallMethod( Meataxe64Echelonization, [IsMeataxe64MatrixObj], 
        m -> MTX64_Echelize(UnderlyingMeataxe64Matrix(m)));


InstallMethod( RankMat, [IsMeataxe64MatrixObj],
        m -> Meataxe64Echelonization(m).rank);

InstallMethod( RankMatDestructive, [IsMeataxe64MatrixObj and IsMutable],
        m -> Meataxe64Echelonization(m).rank);

InstallMethod( InverseMutable, [IsMeataxe64MatrixObj],
        function(m)
    local e,n;
    n := NumberColumns(m);
    if n <> NumberRows(m) then
        Error("Inverse: Matrix must be square");
    fi;    
    e := Meataxe64Echelonization(m);
    if e.rank <> n then
        return fail;
    fi;
    return MakeMeataxe64Matrix(-e.multiplier);
end);

InstallOtherMethod(NullspaceMat, [IsMeataxe64MatrixObj],
        m -> MakeMeataxe64Matrix(NullspaceMat(UnderlyingMeataxe64Matrix(m))));

BindGlobal("MTX64_SEM_fix", function(r)
    if IsBound(r.vectors) then
        r.vectors := MakeMeataxe64Matrix(r.vectors);
    fi;
    if IsBound(r.coeffs) then
        r.coeffs := MakeMeataxe64Matrix(r.coeffs);
    fi;
    if IsBound(r.relations) then
        r.relations := MakeMeataxe64Matrix(r.relations);
    fi;
    return r;
end);


InstallOtherMethod(SemiEchelonMatTransformationDestructive, [IsMeataxe64MatrixObj and IsMutable], 
        m -> MTX64_SEM_fix(MTX64_SEMT(UnderlyingMeataxe64Matrix(m), Meataxe64Echelonization(m))));
InstallOtherMethod(SemiEchelonMatTransformation, [IsMeataxe64MatrixObj], 
        m -> MTX64_SEM_fix(MTX64_SEMT(UnderlyingMeataxe64Matrix(m), Meataxe64Echelonization(m))));
InstallOtherMethod(SemiEchelonMatDestructive, [IsMeataxe64MatrixObj and IsMutable], 
        m -> MTX64_SEM_fix(MTX64_SEM(UnderlyingMeataxe64Matrix(m), Meataxe64Echelonization(m))));
InstallOtherMethod(SemiEchelonMat, [IsMeataxe64MatrixObj], 
        m -> MTX64_SEM_fix(MTX64_SEM(UnderlyingMeataxe64Matrix(m), Meataxe64Echelonization(m))));
    
InstallOtherMethod(SolutionMat, [IsMeataxe64MatrixObj, IsMeataxe64VectorObj],
        function(m,v)
    local  res,um;
    um := UnderlyingMeataxe64Matrix(m);    
    res := MTX64_SolutionsMat(um,UnderlyingMeataxe64Matrix(v), Meataxe64Echelonization(m));
    if MTX64_GetEntryOfBitString(res[1],0) = 1 then
        return MakeMeataxe64Matrix(res[2]);
    else
        return fail;
    fi;
end);
