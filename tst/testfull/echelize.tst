gap> START_TEST("echelize.tst");
gap> Read(Filename(DirectoriesPackageLibrary("meataxe64","tst"),"testutils.g"));
gap> for q in [2,3,4,5,7,8,13,16,256,257,
>       NextPrimeInt(2^16), 2^24, NextPrimeInt(2^32)] do
>       for n in [1,2,3,5,8,20,100,200,500,1000,2000] do
>        for m in [1,2,4,6,18,90,110,200, 400, 900,2000] do
>            if q <= 256 or n < 1000  then
>                if q <= 65536 or n < 200 then
>                   testEchelize(n,m,q, MTX64_Echelize_DefaultOptions);
>                fi;
>             fi; od; od; od;
gap> STOP_TEST("echelize.tst");
