Bench1 := function(f, arg...)
    local  t, m, n, memory, wallTime;
    t := Runtime();
    n := NanosecondsSinceEpoch();
    CallFuncList(f, arg);
    return rec(cpuTime := Runtime()-t,
               wallTime := NanosecondsSinceEpoch() -n);
end;

   
BenchTill := function(lim, f, arg...)
    local  results, timelim,  walllim;
    results := [];
    timelim :=  IsBound(lim.cpuTime);
    walllim := IsBound(lim.wallTime);
    if not (timelim or walllim) then
        Error("No limits given");
    fi;
    repeat
        Add(results, CallFuncList(Bench1, Concatenation([f], arg)));
    until (timelim and Sum(results, x->x.cpuTime) > lim.cpuTime) or
          (walllim and Sum(results, x->x.wallTime) > lim.wallTime);
    return results;
end;

MulTests := function(q,n)
    local  t, f, m1, m2, f1, f2, f3, res1, resp;
    t := TmpDirectory();   
    f := MTX64_FiniteField(q);    
    m1 := MTX64_RandomMat(f,n,n);
    m2 := MTX64_RandomMat(f,n,n);
    f1 := TmpName();
    f2 := TmpName();
    f3 := TmpName();
    MTX64_WriteMatrix(m1,f1);
    MTX64_WriteMatrix(m2,f2);
    res1 := BenchTill(rec(wallTime := 10*10^9),\*,m1,m2);
    resp := BenchTill(rec(wallTime := 10*10^9),MTX64_fMultiply,t,f1,f2,f3);
    return rec(single := res1, parallel := resp);
end;

fields := [2,3,4,5,7,8,13,17,31, 61, 81, 128, 181];
dims :=  [2000,5000,10000,20000,50000,100000,200000];

AllMulTests := function(lim)
    local  data, q, n, ndata;
    data := [];    
    for q in fields do
        for n in dims do
            ndata := MulTests(q,n);
            Add(data,[q,n,ndata]);            
            Print(q," ",n,"\n");            
            if ndata.single[1].wallTime > lim*10^9 then
                break;
            fi;
        od;
    od;
    return data;
end;

Analyse := function(reclist)
    local  anal2, res, n;
    anal2 := function(reclist, name)
        local  data, sum, count, median, mean, var, min, max;
        data := List(reclist, x->x.(name));
        Sort(data);     
        sum := Sum(data);
        count := Length(data);        
        return rec(count := count,
                   median := data[QuoInt(count+1,2)],
                   mean := Float(sum/count),
                   stddev := Sqrt((Sum(data, x->x*x) - Float(sum^2/count))/count),
                   min := data[1],
                   max := data[count]);
    end;
    res := rec();    
    for n in NamesOfComponents(reclist[1]) do
        res.(n) := anal2(reclist,n);
    od;
    return res;
end;

       
Distill := data->List(data, x-> [x[1],x[2],rec(single := Analyse(x[3].single),
                   parallel := Analyse(x[3].parallel))]);

    
       
     
        
    
            
            
    
