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
    f := FieldOfMTX64Matrix(m);
    q := MTX64_FieldOrder(f);
    bd := GF(q);
    r := rec();
    ObjectifyWithAttributes(r,
            NewType(CollectionsFamily(FamilyObj(bd)), 
                    IsAttributeStoringRep and IsMutable and IsMeataxe64MatrixObj and HasBaseDomain), 
            BaseDomain, bd, UnderlyingMeataxe64Matrix, m);
    return r;    
end);

InstallMethod(NumberRows,[IsMeataxe64MatrixObj],MTX64_Matrix_NumRows);
InstallMethod(NumberColumns,[IsMeataxe64MatrixObj],MTX64_Matrix_NumCols);
InstallMethod(Length,[IsMeataxe64MatrixObj],MTX64_Matrix_NumRows);
InstallMethod(DimensionsMat, [IsMeataxe64MatrixObj], 
        m -> [MTX64_Matrix_NumRows, MTX64_Matrix_NumCols]);
InstallMethod(RankMat, [IsMeataxe64MatrixObj], m->RankMat(UnderlyingMeataxe64Matrix(m)));
InstallMethod(RankMatDestructive, [IsMeataxe64MatrixObj], RankMat);


DeclareCategory("IsMeataxe64MatrixRowReference",IsVectorObj);
DeclareRepresentation("IsMeataxe64MatrixRowReferenceRep", IsPositionalObjectRep, 2);

InstallMethod(\[\], [IsMeataxe64MatrixObj, IsPosInt],
        function(m,i)
    local  ref, filts, type;
    ref := [m,i];
    filts := IsMeataxe64MatrixRowReferenceRep and IsMeataxe64MatrixRowReference;
    if IsMutable(m) then
        filts := filts and IsMutable;
    else
        filts := filts and IsCopyable;
    fi;
    type := NewType(ElementsFamily(FamilyObj(m)),  filts);
    Objectify(type,ref);
    return ref;
end);

DeclareAttribute("UnderlyingMatrixObj", IsMeataxe64MatrixRowReference);
DeclareAttribute("RowNumber", IsMeataxe64MatrixRowReference);


InstallMethod(UnderlyingMatrixObj, [IsMeataxe64MatrixRowReferenceRep and IsMeataxe64MatrixRowReference],
        x->x![1]);
InstallMethod(RowNumber, [IsMeataxe64MatrixRowReferenceRep and IsMeataxe64MatrixRowReference],
        x->x![2]);


InstallMethod(BaseDomain, [IsMeataxe64MatrixRowReference], 
        r->BaseDomain(UnderlyingMatrixObj(r)));

InstallMethod(Length, [IsMeataxe64MatrixRowReference],
        r->NumberRows(UnderlyingMatrixObj(r)));

InstallMethod(\[\], [IsMeataxe64MatrixRowReference, IsPosInt],
        {r,i} -> MatElm(UnderlyingMatrixObj(r),RowNumber(r),i));

InstallMethod(\[\]\:\=, [IsMeataxe64MatrixRowReference, IsPosInt, IsFFE],
        {r,i,x} -> SetMatElm(UnderlyingMatrixObj(r),RowNumber(r),i,x));

InstallMethod(PositionNonZero, [IsMeataxe64MatrixRowReference],
        function(r)
    local  u, m, res;
    u := UnderlyingMatrixObj(r);
    m := UnderlyingMeataxe64Matrix(u);    
    res := MTX64_DNzl(m, RowNumber(r));
    if res = fail then
        return MTX64_Matrix_NumCols(m)+1;
    else
        return res+1;
    fi;
end);

InstallMethod(ListOp, [IsMeataxe64MatrixRowReference],
        function(r)
    local  u, i, m;
    u := UnderlyingMeataxe64Matrix(r);
    i := RowNumber(r);
    m := NumberColumns(u);
    return List([1..m], j->u[i,j]);
end);

InstallMethod(ListOp, [IsMeataxe64MatrixRowReference, IsFunction],
        function(r,f)
    local  u, i, m;
    u := UnderlyingMeataxe64Matrix(r);
    i := RowNumber(r);
    m := NumberColumns(u);
    return List([1..m], j->f(u[i,j]));
end);

InstallMethod(Unpack, [IsMeataxe64MatrixRowReference], ListOp);

InstallMethod(ShallowCopy, [IsMeataxe64MatrixRowReference],
        function(r)
    local  u, m, c;
    u := UnderlyingMatrixObj(r);
    m := UnderlyingMeataxe64Matrix(u);
    c := MTX64_NewMatrix(FieldOfMTX64Matrix(m), 1, MTX64_Matrix_NumCols(m));
    MTX64_DCpy(m, c, RowNumber(r), 1);
    return MakeMeataxe64Vector(c);
end);

   
         
         
        
