gap> START_TEST("solving.tst");
gap> Read(Filename(DirectoriesPackageLibrary("meataxe64","tst"),"testutils.g"));
gap> for x in [[2,1],[2,300],[2,5000], [3, 600], [4,702],
>               [17, 100], [65536, 101]] do
>           testSolutionMat(x[2],x[1]);
>       od;
gap> STOP_TEST("solving.tst");
