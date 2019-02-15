#
gap> START_TEST("solving.tst");
gap> Read(Filename(DirectoriesPackageLibrary("meataxe64","tst"),"testutils.g"));
gap> for q in [2,3,4,5,7,8,13,16,256,257,
>       NextPrimeInt(2^16), 2^24, NextPrimeInt(2^32)] do
>       for n in [1,2,3,5,8,20,100,200,500,1000,2000] do
>           testSolutionMat(n,q);
>       od; od;
gap> STOP_TEST("solving.tst");
