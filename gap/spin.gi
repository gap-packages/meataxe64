#
# Experiments towards some meataxe functionality
#


MTX64_SpinRelative := function(seeds, gens, isInvol, ech)
    local  n, f, sb, e, rs, nextseeds, done, i, r1, gen, vecs, ims, 
           newrank, newsb;
    n := MTX64_NumCols(seeds);    
    f := MTX64_FieldOfMatrix(seeds);    
    sb := MTX64_NewMatrix(f,n-ech.rank,n);
    e := ShallowCopy(ech);
    rs := MTX64_CleanExtend(e, seeds);
    rs := MTX64_RowSelect(rs, seeds);
    MTX64_DCpy(rs[1],sb,0, 0, e.rank);
    nextseeds := List(gens, x->0);
    repeat
        done := true;
        for i in [1..Length(gens)] do
            r1 := e.rank;                
            if nextseeds[i] < r1 then
                gen := gens[i];
                vecs := MTX64_Submatrix(sb, nextseeds[i]+1, r1-nextseeds[i],1,n);
                ims := vecs*gen;               
                nextseeds[i] := r1;                
                rs := MTX64_CleanExtend(e, ims);
                newrank := MTX64_WeightOfBitString(rs);                
                if newrank > 0 then
                    newsb := MTX64_RowSelect(rs,ims)[1];
                    done := false;        
                    MTX64_DPaste(newsb,r1,newrank,0,sb);
                    if isInvol[i] then
                        nextseeds[i] := e.rank;
                    fi;
                fi;
            fi;
        od;
    until done;
    return rec(sb := MTX64_Submatrix(sb,1,e.rank,1,n), ech := e);
end;
    

MTX64_Spin := function(seeds, gens, opts...)
    local  n, f, isInvol, e;
    n := MTX64_NumCols(seeds);    
    f := MTX64_FieldOfMatrix(seeds);    
    if Length(opts) > 1 then
        isInvol := opts[2];
    else
        isInvol := List(gens, x->false);
    fi;
    if not ForAll(gens, gen -> IsMTX64Matrix(gen) and MTX64_FieldOfMatrix(gen) = f and MTX64_NumCols(gen) = n and 
               MTX64_NumRows(gen) = n) then
        Error("Generators and seed incompatible");
    fi;
    e := MTX64_Echelize(MTX64_NewMatrix(f,0,n));
    Unbind(e.det); 
    Unbind(e.multiplier);
    Unbind(e.cleaner);    
    return MTX64_SpinRelative(seeds,gens, isInvol, e);
end;

MTX64_OnSubMat := function(basis, x, ech)
    local  im, cs;
    im := basis*x;
    cs := MTX64_ColSelect(ech.colSelect, im);
    return -cs[1]*ech.multiplier;    
end;

MTX64_OnQuoMat := function(basis, x, ech)
    local  rs, cs;
    rs := MTX64_RowSelect(ech.colSelect, x);
    cs := MTX64_ColSelect(ech.colSelect, rs[2]);
    return  cs[2]+cs[1]*ech.remnant;
end;

MTX64_OnSubQuo := function(basis, gens)
    local  ech;
    ech := MTX64_Echelize(basis);
    return rec(sub := List(gens, x-> MTX64_OnSubMat(basis, x, ech)),
               quo := List(gens, x-> MTX64_OnQuoMat(basis, x, ech)));
    
end;


MTX64_CPInner := function(mat)
    local  f, n, v, sp, facs, e;
    f := MTX64_FieldOfMatrix(mat);
    n := MTX64_NumRows(mat);    
    v := MTX64_NewMatrix(f,1,n);    
    v[1,1] := One(f);
    sp := MTX64_Spin(v,[mat]);
    if sp.ech.rank <> n then
        facs := MTX64_CPInner(MTX64_OnQuoMat(sp.sb, mat, sp.ech));
    else
        facs := [];
    fi;
    # cludge
    v := MTX64_Submatrix(sp.sb,sp.ech.rank,1,1,n)*mat;
    MTX64_DPaste(v,sp.ech.rank,1,0,sp.sb);    
    e := MTX64_Echelize(sp.sb);    
    Add(facs, e.cleaner);
    return facs;
end;

   
MTX64_CP := function(mat, ind)
    local  facs, vecs, one, f, pols;
    facs := MTX64_CPInner(mat);
    vecs := List(facs, x->x[1]);    
    one := One(GF(MTX64_FieldOrder(MTX64_FieldOfMatrix(mat))));
    f := FamilyObj(one);    
    pols := List(vecs, function(v)
        Add(v, one);        
        return UnivariatePolynomialByCoefficients(f, v, ind);
    end);
    return Product(pols);
end;

MTX64_Sparsify := function(mat, b)
    local  f, n, seeds, o, i, sp;
    f := MTX64_FieldOfMatrix(mat);
    n := MTX64_NumRows(mat);
    seeds := MTX64_NewMatrix(f, b, n);
    o := One(f);    
    for i in [1..b] do
        seeds[i,i] := o;
    od;
    sp := MTX64_Spin(seeds,[mat]);
    return MTX64_OnSubQuo(sp.sb,[mat]);
end;

    
      
        
    
    

    
    
    
    
    
        
        
    
    
