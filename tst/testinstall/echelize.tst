gap> START_TEST("echelize.tst");
gap> Read(Filename(DirectoriesPackageLibrary("meataxe64","tst"),"testutils.g"));
gap> f := MTX64_FiniteField(3);
<MTX64 GF(3)>
gap> repeat m := MTX64_RandomMat(f,1000,1000); until RankMat(m) = 1000;
gap> m1 := ShallowCopy(m);
< matrix 1000x1000 : <MTX64 GF(3)>>
gap> m1[2] := m1[1];
< mutable compressed vector length 1000 over GF(3) >
gap> RankMat(m1);
999
gap> m1^-1;
fail
gap> m1 := ShallowCopy(m);
< matrix 1000x1000 : <MTX64 GF(3)>>
gap> m1[1000] := m1[1];
< mutable compressed vector length 1000 over GF(3) >
gap> RankMat(m1);
999
gap> m1^-1;               
fail
gap> for x in [[2,1000,2000],[3,2000,1000],[5,100,100],
>        [9,450,451],[13,100,110],[17, 250,200], [181, 500,501],
>        [257,300,310], [65537, 310, 300], [2^24, 20, 30],
>        [NextPrimeInt(2^32), 30,20], [11,1,1],[23,0,1],[16,1,0]] do
>         testEchelize(x[2],x[3],x[1], MTX64_Echelize_DefaultOptions);
>    od;
gap> STOP_TEST("echelize.tst");
