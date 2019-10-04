#
# A brute force check for correctness of echelization
#
MTX64_CheckEchelize := function(a,r,o)
    local  q, ga, n, m, rk, gm, gk, gcs, grs, colOrder, rowOrder, ga2, 
           z, one, i, j, x, gr, gm2;
    q := MTX64_FieldOrder(FieldOfMTX64Matrix(a));    
    ga := MTX64_ExtractMatrix(a);
    n := MTX64_NumRows(a);
    m := MTX64_NumCols(a);
    if n <> Length(ga) then
        Error("input matrix number of rows wrong");
    fi;
    if n > 0 and m <> Length(ga[1]) then
        Error("input matrix number of cols wrong");
    fi;        
    rk := r.rank;    
    if o.failIfSingular and (m <> n or rk < m) then
        Error("Should not have succeeded with this matrix");
    fi;
    if o.multiplierNeeded then
        gm := MTX64_ExtractMatrix(r.multiplier);
        if Length(gm) <> rk then
            Error("Multiplier matrix size wrong");
        fi;
        if rk > 0 and Length(gm[1]) <> rk then
            Error("Multiplier matrix size wrong");
        fi;
        if rk > 0 and RankMat(gm) <> rk then
            Error("Multiplier is singular");
        fi;
    else
        gm := NullMat(rk,rk,GF(q));         
    fi;
    if o.cleanerNeeded then
        gk := MTX64_ExtractMatrix(r.cleaner);
        if Length(gk) <> n - rk then
            Error("Cleaner number rows wrong");
        fi;
        if n > rk and Length(gk[1]) <> rk then
            Error("Cleaner number cols wrong");
        fi;
    else
        gk := NullMat(n-rk, rk, GF(q));        
    fi;
    if MTX64_WeightOfBitString(r.colSelect) <> rk or MTX64_LengthOfBitString(r.colSelect) <> m then
        Error("ColSelect bitstring doesn't fit");
    fi;
    if MTX64_WeightOfBitString(r.rowSelect) <> rk or MTX64_LengthOfBitString(r.rowSelect) <> n then
        Error("RowSelect bitstring doesn't fit");
    fi;
    gcs := MTX64_BlistBitString(r.colSelect);
    grs := MTX64_BlistBitString(r.rowSelect);
    colOrder := ListBlist( [1..m], gcs);
    Append(colOrder, Difference([1..m],colOrder));
    rowOrder := ListBlist([1..n], grs);
    Append(rowOrder, Difference([1..n],rowOrder));
    ga2 := ga{rowOrder}{colOrder};  
    gm2 := NullMat(n,n,GF(q));
    gm2{[1..rk]}{[1..rk]} := gm;
    gm2{[rk+1..n]}{[1..rk]} := gk;
    z := Zero(GF(q));
    one := One(GF(q));    
    for i in [rk+1..n] do
        gm2[i][i] := one;
    od;
    gm2 := ImmutableMatrix(GF(q),gm2);
    ga2 := ImmutableMatrix(GF(q),ga2);    
    x := MTX64_ExtractMatrix(MTX64_Matrix(gm2,q)*MTX64_Matrix(ga2,q));
 
    if o.remnantNeeded then
        gr := MTX64_ExtractMatrix(r.remnant);
        if Length(gr) <> rk then
            Error("Wrong number of rows in remnant");
        fi;
        if rk > 0 and Length(gr[1]) <> m - rk then
            Error("Wrong number of cols in remnant");
        fi;
    fi;    
    for i in [1..n] do
        for j in [1..m] do
            if i <= rk then 
                if o.multiplierNeeded then
                    if j <= rk then 
                        if i = j then
                            if x[i][j] <> -one then
                                Error("Product wrong at ",i," ",j);
                            fi;
                        else
                            if x[i][j] <> z then
                                Error("Product wrong at ",i," ",j);
                            fi;
                        fi;
                    else
                        if o.remnantNeeded then
                            if x[i][j] <> gr[i][j-rk] then
                                Error("Product wrong at ",i," ",j);
                            fi;
                        fi;
                    fi;
                fi;                
            else
                if o.cleanerNeeded then
                    if  x[i][j] <> z then
                        Error("Product wrong at ",i," ",j);
                    fi;
                fi;
            fi;
        od;
    od;
end;
   
                
    
testEchelize := function(m, n, q, o)
    local fld, mat, r, rg;
    fld := MTX64_FiniteField(q);
    mat := MTX64_RandomMat(fld, m, n); 
    r := MTX64_Echelize(mat);
    rg := MTX64_GAPEchelize(mat, o);
    MTX64_CheckEchelize(mat, r, o);
    MTX64_CheckEchelize(mat, rg, o);
end;

testSolutionMat := function(n, q)
    local fld, mat, gapmat, v, gapv, sol, gapsol;
    fld := MTX64_FiniteField(q);
    mat := MTX64_RandomMat(fld, n, n);
    v   := MTX64_RandomMat(fld, 1, n); 
    sol := MTX64_SolutionsMat(mat, v)[2];
    if (MTX64_NumRows(sol) = 1) and
       (sol * mat <> v) then
        Error("Not a solution");
    fi;
end;

