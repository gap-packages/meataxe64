#
gap> extract_roundtrip := function(n, q)
>    local m, mm;
>    m := RandomMat(n, n, GF(q));
>    mm := MTX64_ExtractMatrix(MTX64_Matrix(m));
>    if m <> mm then
>        Error("Mismatch between matrix and roundtrip matrix");
>    fi;
>    return true;
>    end;;
gap> for i in [1..10] do extract_roundtrip(Random([2..100]), Random(Primes) ^ Random([1..3])); od;;

#
gap> echelize := function(m, n, q)
>       local fld, mat, r;
>       fld := MTX64_FiniteField(q);
>       mat := MTX64_RandomMat(fld, m, n); 
>       r := MTX64_Echelize(mat);
>    end;;
gap> for i in [1..1000] do echelize(Random([2..100]), Random([2..100]), Random(Primes)); od;

#
gap> solutionmat := function(n, q)
>       local fld, mat, gapmat, v, gapv, sol, gapsol;
>       fld := MTX64_FiniteField(q);
>       mat := MTX64_RandomMat(fld, n, n);
>       v   := MTX64_RandomMat(fld, 1, n); 
>       sol := MTX64_SolutionsMat(mat, v)[2];
>       if (MTX64_Matrix_NumRows(sol) = 1) and
>          (sol * mat <> -v) then
>           Error("Not a solution");
>       fi;
>       sol    := MTX64_ExtractMatrix(sol);
>       gapmat := MTX64_ExtractMatrix(mat);
>       gapv   := MTX64_ExtractMatrix(v)[1];
>       gapsol := SolutionMat(gapmat, gapv);
>       if not ((sol = [] and gapsol = fail) or IsZero(sol[1] + gapsol)) then
>           Error("Solutions do not match");
>       fi;
>       return [sol, gapsol, sol + gapsol]; 
>    end;;
gap> for i in [1..1000] do solutionmat(Random([2..100]), Random(Primes)); od;
