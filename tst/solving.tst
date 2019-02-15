#
gap> START_TEST("solving.tst");

#
gap> solutionmat := function(n, q)
>       local fld, mat, gapmat, v, gapv, sol, gapsol;
>       fld := MTX64_FiniteField(q);
>       mat := MTX64_RandomMat(fld, n, n);
>       v   := MTX64_RandomMat(fld, 1, n); 
>       sol := MTX64_SolutionsMat(mat, v)[2];
>       if (MTX64_Matrix_NumRows(sol) = 1) and
>          (sol * mat <> v) then
>           Error("Not a solution");
>       fi;
>    end;;
gap> for q in [2,3,4,5,7,8,13,16,256,257,
>       NextPrimeInt(2^16), 2^24, NextPrimeInt(2^32)] do
>       for n in [1,2,3,5,8,20,100,200,500,1000,2000] do
>                   AppendTo("foo", Runtime(), " S ",q," ",n,"\n");
>           solutionmat(n,q);
>       od; od;
gap> STOP_TEST("solving.tst");
