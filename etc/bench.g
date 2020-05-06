LoadPackage("meataxe64");
ReadPackage("meataxe64","etc/timers.g");

sqtime := function(q,n) 
    local  f, m;
    f := MTX64_FiniteField(q);
    m := MTX64_RandomMat(f,n,n);
    return timer(function(ct)
        local i,x,mm;
        mm := m;
        for i in [1..ct] do
            x := mm*mm;
        od;
    end);
end;

rawtimes := function(q,maxpower)
    return List([1..maxpower], i->sqtime(q,2^i));
end;

rates := function(raw, exponent)
    return List([1..Length(raw)], 
                i-> Float(2^(exponent*i)/raw[i]));
end;

sqftime := function(q,n) 
    local  f, m;
    f := MTX64_FiniteField(q);
    m := MTX64_RandomMat(f,n,n);
    MTX64_WriteMatrix(m,"a");    
    return timer(function(ct)
        local i,x,mm;
        for i in [1..ct] do
            MTX64_fMultiply(".","a","a","b");
        od;
    end);
end;

rawftimes := function(q,maxpower)
    return List([1..maxpower], i->sqftime(q,2^i));
end;
            
