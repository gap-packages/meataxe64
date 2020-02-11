MTX64_Sparsify := function(m, b, k) 
    local  n, f, seeds, i, shuffle, l, j, ech, pivs, nonpivs, 
           imagecoord, spivs, sbl, sbs, nspivs, map, mapbs, basis, x;
    n := MTX64_NumRows(m);
    f := MTX64_FieldOfMatrix(m);    
    seeds := [MTX64_RandomMat(f,b,n)];
    for i in [1..k] do
        Add(seeds, seeds[i]*m);
    od;
    shuffle := MTX64_NewMatrix(f,b*(k+1),n);
    l := 0;    
    for i in [1..b] do
        for j in [1..k] do
            MTX64_DCpy(seeds[j],shuffle,i-1, l,1);
            l := l+1;            
        od;
    od;
    MTX64_DCpy(seeds[k+1], shuffle, 0, l, b);
    ech := MTX64_Echelize(shuffle);
    if ech.rank <> n then
        Error("Not spanned. Rank ",ech.rank," out of ",n);
    fi;
    
    pivs := MTX64_PositionsBitString(ech.rowSelect); 
    nonpivs := Difference([1..(k+1)*b], pivs);
    if pivs[Length(pivs)] > b*k then
        return fail;
    fi;
    imagecoord := function(i)
        if i mod k <> 0 then
            return i+1;
        else
            return b*k+QuoInt((i-1),k)+1;
        fi;
    end;
    
    spivs := Filtered(pivs, i -> imagecoord(i) in pivs);
    sbl := BlistList(pivs,spivs);
    sbs := MTX64_BitStringBlist(sbl);
    nspivs := Difference(pivs,spivs);    
    map := List(spivs, p -> Position(pivs, imagecoord(p)));
    mapbs := MTX64_BitStringBlist(BlistList([1..n],map));    
    basis := shuffle{pivs};    
    x := -ech.cleaner{List(nspivs, p -> Position(nonpivs, imagecoord(p)))};
    return rec(base := basis, map := map, mapbs := mapbs, sparsebs := sbs, densepart := x);
end;

MTX64_SparseAct := function(vecs, sm)
    local  x, sim, dim;
    x := MTX64_ColSelect(sm.sparsebs, vecs);
    sim := MTX64_BSColRifZ(sm.mapbs, x[1]);
    dim := x[2]*sm.densepart;
    return sim+dim;
end;

MTX64_SparseCheck := {vecs, m, sm} ->
   vecs*sm.base*m = MTX64_SparseAct(vecs,sm)*sm.base;


    

    
