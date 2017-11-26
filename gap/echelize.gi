
#
# This file contains an implementation of a more efficient slab scale
# echelize. It is expected that, once the right algorithm is determined,
# it will be implemented in C in slab.c and this file will become redundant.

#
if not IsBound(InfoMTX64_NG) then
    DeclareInfoClass("InfoMTX64_NG");
fi;

#
# This function takes a matrix and a records of options which must contain
# all of the following components:
#
# recurseTop -- Boolean    if true use this algorithm recursively on top-left submatrix
# recurseBottom -- Boolean if true use this algorithm recursively on bottom-right submatrix
# splitMethod -- integer   1 signifies a fixed size top block;
#                          2 signifies a fixed ratio between the two
# rowSplitParam 
# colSplitParam  -- integert meaning depends on splitMethod
#                          splitmethod 1 it should be an integer giving the size of the
#                                fixed size block
#                          splitmethod 2 it should be a minimum block size
# splitRatio     -- rational or float, only meaningful for splitMethod 2
#                               it should be a target for the ratio 
#                               <top block size>/<bottom block size>
# multNeeded -- Boolean     if true the multiplier must be calculated and returned
# cleanerNeeded -- Boolean  if true the cleaner must be calculated and returned
# remnantNeeded -- Boolean  if true the remnant must be calculated and returned
#                           felds not needed MAY be returned.
# pivotsNeeded -- Boolean  if true, the row and column pivots must be returned
#
# failIfSingular -- Boolean  if true return fail if the matrix is non-singular
#
#



MTX64_EchelizeInner :=  function(mat, optrec)
    local  n, m, f, rowSplit, colSplit, baseCase, res, a1, optrec2, 
           r1, xrs, xcs, hilo, a2, a34, a3, a4, 
           keep1, keep2, res2, ret, comb, pc, rifc, pr, rifr, k2ab, k2a, k2b,  
           a2ab, a2a, a2b;
    
    
    n := MTX64_Matrix_NumRows(mat);
    m := MTX64_Matrix_NumCols(mat);
    if optrec.failIfSingular and n <> m then
        return fail;
    fi;
    f := FieldOfMTX64Matrix(mat);
    Info(InfoMTX64_NG,1, "Starting computation on ",n,"*",m," matrix over GF(",MTX64_FieldOrder(f),")"); 
    if optrec.splitMethod = 1 then
        rowSplit := Minimum(optrec.rowSplitParam, n);
        colSplit := Minimum(optrec.colSplitParam, m);
        baseCase := rowSplit = n and colSplit= m;
    elif optrec.splitMethod = 2 then
        rowSplit := Int(n*optrec.splitRatio/(1+optrec.splitRatio));
        colSplit := Int(m*optrec.splitRatio/(1+optrec.splitRatio));
        baseCase := n*optrec.rowSplitMin < optrec.rowSplitParam or
                    m*optrec.colSplitMin < optrec.colSplitParam or
                    (n < optrec.rowSplitParam and m < optrec.colSplitParam);
    elif optrec.splitMethod = 3 then
        rowSplit := Int(n*optrec.splitRatio/(1+optrec.splitRatio));
        colSplit := Int(m*optrec.splitRatio/(1+optrec.splitRatio));
        rowSplit := Minimum(rowSplit, colSplit);
        colSplit := rowSplit;
        baseCase := n*optrec.rowSplitMin < optrec.rowSplitParam or
                    m*optrec.colSplitMin < optrec.colSplitParam or
                    (n < optrec.rowSplitParam and m < optrec.colSplitParam);
    fi;
    if baseCase then
        res := MTX64_SLEchelize(mat);
        Info(InfoMTX64_NG,2, "Base case rank ",res.rank);
        return res;
    fi;
    a1 := MTX64_Submatrix(mat, 1, rowSplit, 1, colSplit); # k x k
    Info(InfoMTX64_NG,2,"Splitting off ",rowSplit," rows and ",colSplit," columns");
    if optrec.recurseTop then
        optrec2 := ShallowCopy(optrec);
        optrec2.multiplierNeeded := true;        
        optrec2.cleanerNeeded := false;
        optrec2.remnantNeeded := false;
        optrec2.pivotsNeeded := true;        
        optrec2.failIfSingular := false;      
        res := MTX64_EchelizeInner(a1,optrec2);    
    else
        res := MTX64_SLEchelize(a1);
    fi;
    Info(InfoMTX64_NG,2,"Top part rank: ",res.rank);
    r1 := res.rank;    
    if r1 = 0 then 
        Info(InfoMTX64_NG, 1,"Zero top matrix, falling back");        
        res := MTX64_SLEchelize(mat);        
        Info(InfoMTX64_NG, 2,"Fallback rank ",res.rank);
        return res;        
    fi;
    
    # Pull out the pivot rows and columns from the top part and split up the matrix
    xrs := MTX64_PaddedBitString(res.rowSelect, n, 0); 
    xcs := MTX64_PaddedBitString(res.colSelect, m, 0);
    hilo := MTX64_RowSelect(xrs, mat); 
    a2 := MTX64_ColSelect(xcs,hilo[1])[2]; # r1 x (m-r1)
    a34 := MTX64_ColSelect(xcs,hilo[2]);   
    a3 := a34[1];                          # (n-r1) x r1
    a4 := a34[2];                          # (n-r1) x (m-r1)
    # Clean right
    a2 := res.multiplier*a2;               
    if optrec.multiplierNeeded then
        # Start populating the keeptrack
        keep1 := res.multiplier;           # r1 x r1
    fi;
    if optrec.cleanerNeeded or optrec.multiplierNeeded then
        keep2 := a3*res.multiplier;       # (n-r1) x r1
    fi;
    # clean down
    a4 := a4+a3*a2;
    
    # second recursive call
    if optrec.recurseBottom then
        optrec2 := ShallowCopy(optrec);
        optrec2.pivotsNeeded := optrec.pivotsNeeded or 
                           optrec.cleanerNeeded or 
                           optrec.multiplierNeeded or
                           optrec.remnantNeeded;
        res2 := MTX64_EchelizeInner(a4, optrec);
        if res2 = fail then
            # This can only happen if we have failIfSingular set
            return fail;
        fi;
    else
        res2 := MTX64_SLEchelize(a4);        
    fi;
    Info(InfoMTX64_NG,2,"Bottom part rank: ",res2.rank);
    ret := rec(rank := r1 + res2.rank);    
    
    if optrec.failIfSingular and ret.rank < n then
        return fail;
    fi;
    
       
    # if all we wanted was the rank, we can return now.
    if not (optrec.pivotsNeeded or 
            optrec.cleanerNeeded or 
            optrec.multiplierNeeded or
            optrec.remnantNeeded) then
        return ret;
    fi;
    
    # combine pivots 
    comb := MTX64_BSCombine(xcs, res2.colSelect);  
    pc := comb[1];             # rank/m
    rifc := comb[2];           # r2/rank
    comb := MTX64_BSCombine(xrs, res2.rowSelect);
    pr := comb[1];             # rank/n
    rifr := comb[2];           # r2/rank
    
    ret.rowSelect := pr;
    ret.colSelect := pc;    
    
    if not (optrec.cleanerNeeded or 
            optrec.multiplierNeeded or
            optrec.remnantNeeded) then
        return ret;
    fi;
    
    # calculate cleaner
    if optrec.cleanerNeeded or optrec.multiplierNeeded then
        keep2:= MTX64_BSColRifZ(rifr,keep2);           #  (n-r1) x rank
        k2ab := MTX64_RowSelect(res2.rowSelect,keep2); 
        k2a := k2ab[1];                                #  r2 x rank 
        MTX64_BSColPutS(rifr,k2a,One(f));    
        if optrec.cleanerNeeded then
            k2b := k2ab[2] + res2.cleaner*k2a;         # (n-rank) x rank  + (m-rank) x r2 * r2 x rank
            ret.cleaner := k2b;
            if not (optrec.multiplierNeeded or optrec.remnantNeeded) then
                return ret;
            fi;
        fi;
    fi;
    
    # Get organized for back-clean
    
    a2ab := MTX64_ColSelect(res2.colSelect,a2);
    a2a := a2ab[1];
    a2b := a2ab[2];
    if optrec.multiplierNeeded  then
       keep1 := MTX64_BSColRifZ(rifr,keep1);
       k2a := res2.multiplier*k2a;
       keep1 := keep1 + a2a*k2a;
       ret.multiplier  := MTX64_RowCombine(rifc, keep1, k2a);
   fi;
   if optrec.remnantNeeded then
        a2b := a2b+a2a*res2.remnant;
        ret.remnant := MTX64_RowCombine(rifc,a2b, res2.remnant);    
    fi;
    return ret;
end;

BindGlobal("MTX64_Echelize_DefaultOptions", rec(
        recurseTop := true,
                              recurseBottom := true,
                              splitMethod := 2,
                              rowSplitParam := 256,
                              colSplitParam := 256,
                              splitRatio := 1,
                              rowSplitMin := 2,
                              colSplitMin := 2,
                              multiplierNeeded := true,
                              cleanerNeeded := true,
                              remnantNeeded := true,
                              pivotsNeeded := true,
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
                                                       cleanerNeeded := false, remnantNeeded := false,
                                                 pivotsNeeded := false)).rank);
InstallMethod(InverseMutable, [IsMTX64Matrix], 1, 
        function(m)
    local res;
    res := MTX64_Echelize(m, rec(failIfSingular := true, cleanerNeeded := false, remnantNeeded := false,
                                            pivotsNeeded := false));
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
