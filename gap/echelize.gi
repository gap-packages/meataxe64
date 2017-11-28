
#
# This file contains an implementation of a more efficient slab scale
# echelize. It is expected that, once the right algorithm is determined,
# it will be implemented in C in slab.c and this file will become redundant.

#
DeclareInfoClass("InfoMTX64_NG");

#
# This function takes a matrix and a records of options which must contain
# all of the following components:
#
# multNeeded -- Boolean     if true the multiplier must be calculated and returned
# cleanerNeeded -- Boolean  if true the cleaner must be calculated and returned
# remnantNeeded -- Boolean  if true the remnant must be calculated and returned
#                           felds not needed MAY be returned.
# pivotsNeeded -- Boolean  if true, the row and column pivots must be returned
#
# failIfSingular -- Boolean  if true return fail if the matrix is non-singular
#
#

MTX64_EchelizeInner := fail;


MTX64_NumZeroRows := function(m)
    local  nor, i;
    nor := MTX64_Matrix_NumRows(m);
    for i in [0..nor-1] do
        if MTX64_DNzl(m, i) <> fail then
            return i;
        fi;
    od;
    return nor;
end;


MTX64_EchelizeLR := function(mat, optrec)
    local  f, n, m, ret, splitAt, a1, a2, optrec2, 
           res, a2s, a2np, res2, rs, a2p, a2ps, 
           r1, r2, k1, k1s, k1a, k1b, kl, ku;
    f := FieldOfMTX64Matrix(mat);
    n := MTX64_Matrix_NumRows(mat);
    m := MTX64_Matrix_NumCols(mat);
    ret := rec();    
    Info(InfoMTX64_NG,1, "EchelizeLR  on ",n,"*",m," matrix over GF(",MTX64_FieldOrder(f),")");    
    splitAt := Minimum(QuoInt(m+1,2),Int(n* (5/4)));
    Info(InfoMTX64_NG,2, "Cutting off ",splitAt," cols");
    a1 := MTX64_NewMatrix(f, n, splitAt);
    MTX64_DCut(mat, 0, n, 0, a1);
    a2 := MTX64_NewMatrix(f, n, m-splitAt);
    MTX64_DCut(mat, 0, n, splitAt, a2);
    optrec2 := ShallowCopy(optrec);
    optrec2.multiplierNeeded := true;        
    optrec2.cleanerNeeded := true;
    optrec2.remnantNeeded := true;
    optrec2.failIfSingular := false;      
    res := MTX64_EchelizeInner(a1,optrec2);
    if res.rank = n then
        ret.rank := n;
        ret.multiplier := res.multiplier;
        ret.rowSelect := res.rowSelect;
        ret.colSelect := MTX64_EmptyBitString(m);
        MTX64_BSShiftOr(res.colSelect, 0, ret.colSelect);
        if optrec.remnantNeeded then
            ret.remnant := MTX64_NewMatrix(f, n, m-n);
            MTX64_DPaste(res.remnant,0,n,0,ret.remnant);
            MTX64_DPaste(res.multiplier*a2,0,n,splitAt-n,ret.remnant);
        fi;
        ret.cleaner := MTX64_NewMatrix(f, 0, n);
        return ret;
    fi;
    a2s := MTX64_RowSelect(res.rowSelect, a2);
    a2np := a2s[2] + res.cleaner*a2s[1];
    res2 := MTX64_EchelizeInner(a2np, optrec);
    ret.rank := res.rank + res2.rank;
    rs := MTX64_BSCombine(res.rowSelect, res2.rowSelect);
    ret.colSelect := MTX64_EmptyBitString(m);
    MTX64_BSShiftOr(res.colSelect,0,ret.colSelect);
    MTX64_BSShiftOr(res2.colSelect,splitAt,ret.colSelect);
    ret.rowSelect := rs[1];
    if not (optrec.multiplierNeeded or optrec.remnantNeeded or optrec.cleanerNeeded) then
        return ret;
    fi;
    if optrec.remnantNeeded then
        a2p := res.multiplier*a2s[1];
        a2ps := MTX64_ColSelect(res2.colSelect, a2p);
        a2np := a2ps[2] + a2ps[1]*res2.remnant;
        ret.remnant := MTX64_NewMatrix(f, ret.rank, m-ret.rank);
        MTX64_DPaste(res.remnant,0, res.rank, 0, ret.remnant);
        MTX64_DPaste(a2np,0, res.rank, splitAt-res.rank, ret.remnant);
        MTX64_DPaste(res2.remnant, res.rank, res2.rank, splitAt - res.rank,ret.remnant);
    fi;
    if optrec.cleanerNeeded or optrec.multiplierNeeded then
        k1 := MTX64_BSColRifZ(rs[2],res.cleaner);
        k1s := MTX64_RowSelect(res2.rowSelect, k1);
        MTX64_BSColPutS(rs[2],k1s[1],One(f));        
    fi;
    if optrec.cleanerNeeded then
        ret.cleaner := k1s[2] + res2.cleaner*k1s[1];
    fi;
    if optrec.multiplierNeeded then
        kl := res2.multiplier*k1s[1];
        ku := MTX64_BSColRifZ(rs[2],res.multiplier);
        ku := ku + a2ps[1]*kl;
        #        ret.multiplier := MTX64_RowCombine(rs[2],ku,kl);
        ret.multiplier := MTX64_NewMatrix(f, ret.rank, ret.rank);
        MTX64_DCpy(ku,ret.multiplier,0,res.rank);
        MTX64_DPaste(kl, res.rank, res2.rank, 0, ret.multiplier);        
    fi;
    return ret;
end;

        
MTX64_EchelizeUD := function(mat, optrec)
    local  f, n, m, ret, splitAt, a1, a2, optrec2, 
           res,  a2s, a2np, res2, cs, a1s, r1, k1, 
           k1s, kl, ku, ml, mu, k1b;
    f := FieldOfMTX64Matrix(mat);
    n := MTX64_Matrix_NumRows(mat);
    m := MTX64_Matrix_NumCols(mat);
    ret := rec();    
    Info(InfoMTX64_NG,1, "EchelizeUD  on ",n,"*",m," matrix over GF(",MTX64_FieldOrder(f),")");    
    splitAt := Minimum(QuoInt(n+1,2),Int(m* (5/4)));
    Info(InfoMTX64_NG,2, "Cutting off ",splitAt," rows");
    a1 := MTX64_NewMatrix(f, splitAt, m);
    MTX64_DCpy(mat, a1, 0, splitAt);
    a2 := MTX64_NewMatrix(f, n-splitAt, m);
    MTX64_DCpy(mat, a2, splitAt, n-splitAt);    
    optrec2 := ShallowCopy(optrec);
    optrec2.remnantNeeded := true;
    if optrec.cleanerNeeded then
        optrec2.multiplierNeeded := true;    
    fi;    
    optrec2.failIfSingular := false;      
    res := MTX64_EchelizeInner(a1,optrec2);
    if res.rank = m then
        ret.rank := m;
        ret.multiplier := res.multiplier;
        ret.colSelect := res.colSelect;
        ret.rowSelect := MTX64_EmptyBitString(n);
        MTX64_BSShiftOr(res.rowSelect, 0, ret.rowSelect);
        ret.remnant := res.remnant;        
        if optrec.cleanerNeeded then
            ret.cleaner := MTX64_NewMatrix(f,n-m,m);
            MTX64_DCpy(res.cleaner, ret.cleaner, 0, splitAt-m);
            MTX64_DPaste(a2*res.multiplier, splitAt-m, n - splitAt, 0, ret.cleaner);
        fi;
        return ret;
    fi;
    a2s := MTX64_ColSelect(res.colSelect, a2);
    a2np := a2s[2] + a2s[1]*res.remnant;
    res2 := MTX64_EchelizeInner(a2np, optrec);
    ret.rank := res.rank + res2.rank;
    if optrec.failIfSingular and (ret.rank < n or ret.rank < m) then
        return fail;
    fi;
        
    cs := MTX64_BSCombine(res.colSelect, res2.colSelect);
    ret.rowSelect := MTX64_EmptyBitString(n);
    MTX64_BSShiftOr(res.rowSelect,0,ret.rowSelect);
    MTX64_BSShiftOr(res2.rowSelect,splitAt,ret.rowSelect);
    ret.colSelect := cs[1];
    if not (optrec.multiplierNeeded or optrec.remnantNeeded or optrec.cleanerNeeded) then
        return ret;
    fi;
    if optrec.remnantNeeded or optrec.multiplierNeeded then
        a1s := MTX64_ColSelect(res2.colSelect, res.remnant);
    fi;
    if optrec.remnantNeeded then
        r1 := a1s[2] + a1s[1]*res2.remnant;
        ret.remnant := MTX64_RowCombine(cs[2],r1,res2.remnant);
    fi;
    if optrec.cleanerNeeded or optrec.multiplierNeeded then
        k1 := a2s[1]*res.multiplier;
        k1s := MTX64_RowSelect(res2.rowSelect, k1);
    fi;
    if optrec.cleanerNeeded then
        ku := MTX64_NewMatrix(f, splitAt-res.rank, ret.rank);
        MTX64_DPaste(res.cleaner,0,splitAt-res.rank,0,ku);
        kl := MTX64_NewMatrix(f, n-splitAt-res2.rank, ret.rank);
        k1b := k1s[2] + res2.cleaner*k1s[1];        
        MTX64_DPaste(k1b,0,n-splitAt-res2.rank,0,kl);
        MTX64_DPaste(res2.cleaner,0,n-splitAt-res2.rank,res.rank,kl);
        ret.cleaner := MTX64_NewMatrix(f, n-ret.rank, ret.rank);
        MTX64_DCpy(ku,ret.cleaner,0,splitAt-res.rank);
        MTX64_DPaste(kl, splitAt-res.rank, n-splitAt-res2.rank, 0, ret.cleaner);
    fi;
    if optrec.multiplierNeeded then
        mu := MTX64_NewMatrix(f,res.rank,ret.rank);
        MTX64_DPaste(res.multiplier, 0, res.rank, 0, mu);
        ml := MTX64_NewMatrix(f,res2.rank, ret.rank);
        MTX64_DPaste(res2.multiplier*k1s[1],0,res2.rank,0,ml);       
        MTX64_DPaste(res2.multiplier,0, res2.rank, res.rank,ml);        
        mu := mu + a1s[1]*ml;
        ret.multiplier := MTX64_RowCombine(cs[2],mu,ml);
    fi;
    return ret;
end;

MTX64_NumZeroColumns := function(mat)
    local  n, m, best, i, x;
    n := MTX64_Matrix_NumRows(mat);
    m := MTX64_Matrix_NumCols(mat);
    best := m;    
    for i in [0..n-1] do
        x := MTX64_DNzl(mat,i);
        if x = 0 then
            return 0;
        fi;
        if x <> fail and x < best then
            best := x;
        fi;
    od;
    return best;
end;

    

        
MTX64_EchelizeInner := function(mat, optrec) 
    local  f, n, zeroRows, colSelect, multiplier, remnant, cleaner, a, 
           res, rs, k, zeroCols, cs, m,
           r;
    f := FieldOfMTX64Matrix(mat);
    n := MTX64_Matrix_NumRows(mat);
    m := MTX64_Matrix_NumCols(mat);
    if optrec.failIfSingular and n <> m then
        return fail;
    fi;    
    Info(InfoMTX64_NG,1, "Starting computation 2 on ",n,"*",m," matrix over GF(",MTX64_FieldOrder(f),")");
    # look for a block of zero rows at the top of matrix 
    zeroRows := MTX64_NumZeroRows(mat);
    if zeroRows = n then
        #
        # This catches the cases of no rows and no columns as well
        #
        Info(InfoMTX64_NG,1,"Matrix is zero");    
        return rec(rank := 0, rowSelect := MTX64_EmptyBitString(n),
                   colSelect := MTX64_EmptyBitString(m),
                   multiplier := MTX64_NewMatrix(f,0,0),
                   remnant := MTX64_NewMatrix(f,0,m),
                   cleaner := MTX64_NewMatrix(f,n,0));
    fi;    
    if zeroRows > 0 then
        Info(InfoMTX64_NG,2,"Found ",zeroRows," initial zero rows");    
        if optrec.failIfSingular then
            return fail;
        fi;
    fi;
    
    if zeroRows > n/10  then
        a := MTX64_NewMatrix(f,n-zeroRows,m);
        MTX64_DCpy(mat,a,zeroRows,n-zeroRows);
        res := MTX64_EchelizeInner(a, optrec);
        rs := MTX64_EmptyBitString(n);
        MTX64_BSShiftOr(res.rowSelect, zeroRows, rs);
        res.rowSelect := rs;        
        if IsBound(res.cleaner) then
            k := MTX64_NewMatrix(f,n-res.rank, res.rank);
            MTX64_DPaste(res.cleaner, zeroRows, n-zeroRows-res.rank, 0, k);
            res.cleaner := k;
        fi;
        return res;
    fi;
    
    zeroCols := MTX64_NumZeroColumns(mat);
    
    if zeroCols > 0 then
        Info(InfoMTX64_NG,2,"Found ",zeroCols," initial zero cols");    
    fi;
    
    
    if zeroCols > m/10 then
        a := MTX64_NewMatrix(f,n,m-zeroCols);
        MTX64_DCut(mat, 0, n, zeroCols, a);
        res := MTX64_EchelizeInner(a, optrec);
        cs := MTX64_EmptyBitString(m);
        MTX64_BSShiftOr(res.colSelect, zeroCols, cs);
        res.colSelect := cs;
        if IsBound(res.remnant) then
            r := MTX64_NewMatrix(f, res.rank, m-res.rank);
            MTX64_DPaste(res.remnant, 0,res.rank, zeroCols, r);
            res.remnant := r;
        fi;
        return res;
    fi;
    
    
    if (m <= optrec.chop2 and n <= optrec.chop2) or
       m <= optrec.chop1 or n <= optrec.chop1 then
        Info(InfoMTX64_NG,2,"Base case");
        return MTX64_SLEchelize(mat);
    fi;
    
    if m > optrec.sq*n then
        return MTX64_EchelizeLR(mat, optrec);
    else
        return MTX64_EchelizeUD(mat, optrec);
    fi;
end;
        
       

BindGlobal("MTX64_Echelize_DefaultOptions", rec(
                              chop2 := 256,
                                               chop1 := 64,
                                               sq := 5/4,
                              aspect  := 2,
                              multiplierNeeded := true,
                              cleanerNeeded := true,
                              remnantNeeded := true,
                              failIfSingular := false));


InstallGlobalFunction(MTX64_Echelize, function(mat, opt...)
    local  optrec, n;
    optrec := ShallowCopy(MTX64_Echelize_DefaultOptions);
    if Length(opt) > 0 then
        for n in NamesOfComponents(opt[1]) do
            optrec.(n) := opt[1].(n);
        od;
    fi;
    return MTX64_EchelizeInner(mat, optrec);
end);

InstallOtherMethod(RankMat, [IsMTX64Matrix],
        m -> MTX64_Echelize(m, rec(multiplierNeeded := false, 
                                                       cleanerNeeded := false, remnantNeeded := false)).rank);
InstallMethod(InverseMutable, [IsMTX64Matrix], 1, 
        function(m)
    local res;
    res := MTX64_Echelize(m, rec(failIfSingular := true, cleanerNeeded := false, remnantNeeded := false));
    if res = fail then
        return res;
    else 
        return -res.multiplier;
    fi;
end);


InstallOtherMethod(NullspaceMat, [IsMTX64Matrix],
        function(m)
    local  res, k;
    res := MTX64_Echelize(m, rec(multiplierNeeded := false, remnantNeeded := false));
    k := res.cleaner;
    k := MTX64_BSColRifZ(res.rowSelect,k);
    MTX64_BSColPutS(res.rowSelect, k, One(FieldOfMTX64Matrix(m)));
    return k;
end);

    
MakeEchTestMatrix := function(f, arg...)
    local  n, d, x, y, r, p, x1, x2, m1, m2;
    if Length(arg) = 1 then
        n := arg[1];
        d := DivisorsInt(n);
        x := First(d, a->a*a>=n);
        y := n/x;
        return MakeEchTestMatrix(f,x,x,y,y);
    elif Length(arg) = 2 then
        n := arg[1];
        r := arg[2];
        d := DivisorsInt(n);
        p := PositionProperty(d,a->a*a> r);
        x1 := d[p];
        x2 := d[p-1];
        if AbsInt(r - x1^2) < AbsInt(r-x2^2) then
            x := x1;
        else
            x := x2;
        fi;
        y := n/x;
        return MakeEchTestMatrix(f,x,y,y,x);
    fi;    
    m1 := RandomMat(arg[1],arg[2],f);
    m2 := RandomMat(arg[3],arg[4],f);
    ConvertToMatrixRep(m1);
    ConvertToMatrixRep(m2);
    return MTX64_Matrix(KroneckerProduct(m1,m2));
end;

   
    
    
        
        
        
        
        
