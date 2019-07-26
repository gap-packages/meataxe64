# Test for functiosn interface to disk based parallel functions
#
gap> START_TEST("functions.tst");
gap> f := MTX64_FiniteField(2);
<MTX64 GF(2)>
gap> m1 := MTX64_RandomMat(f,20000,22000);
< matrix 20000x22000 : <MTX64 GF(2)>>
gap> m2 := MTX64_RandomMat(f,22000,20000);
< matrix 22000x20000 : <MTX64 GF(2)>>
gap> d := DirectoryTemporary();;
gap> fna := Filename(d,"a");;
gap> fnb := Filename(d,"b");;
gap> fnc := Filename(d,"c");;
gap> fnx := Filename(d,"x");;
gap> fny := Filename(d,"y");;
gap> fnz := Filename(d,"z");;
gap> tmpdir := Filename(d, ".");;
gap> MTX64_WriteMatrix(m1, fna);
true
gap> MTX64_WriteMatrix(m2, fnb);
true
gap> MTX64_fMultiply(tmpdir, fna, fnb, fnc);
gap> m3 := MTX64_ReadMatrix(fnc);
< matrix 20000x20000 : <MTX64 GF(2)>>
gap> m3 = m1*m2;
true
gap> MTX64_fMultiply(tmpdir, fnb, fna, fnc);
gap> m3 := MTX64_ReadMatrix(fnc);
< matrix 22000x22000 : <MTX64 GF(2)>>
gap> m3 = m2*m1;
true
gap> MTX64_fMultiplyAdd(tmpdir, fnb, fna, fnc, fnx);
gap> m4 := MTX64_ReadMatrix(fnx);
< matrix 22000x22000 : <MTX64 GF(2)>>
gap> IsZero(m4);
true
gap> MTX64_fMultiplyAdd(tmpdir, fnb, fna, fnx, fnc);
gap> m4 := MTX64_ReadMatrix(fnc);
< matrix 22000x22000 : <MTX64 GF(2)>>
gap> m3 = m4;
true
gap> MTX64_fTranspose(tmpdir,fna,fnc);
gap> m3 := MTX64_ReadMatrix(fnc);
< matrix 22000x20000 : <MTX64 GF(2)>>
gap> m3 = TransposedMat(m1);
true
gap> ReadPackage("meataxe64","tst/testutils.g");
true
gap> f := MTX64_FiniteField(9);
<MTX64 GF(3^2)>
gap> m1 := MakeEchTestMatrix(GF(9), 5000, 2000);
< matrix 5000x5000 : <MTX64 GF(3^2)>>
gap> MTX64_WriteMatrix(m1, fna);
true
gap> rk := MTX64_fProduceNREF(tmpdir, fna, fnc, fnx);
1600
gap> cs := MTX64_ReadBitString(fnc);
< MTX64 bitstring 1600/5000>
gap> r := MTX64_ReadMatrix(fnx);
< matrix 1600x3400 : <MTX64 GF(3^2)>>
gap> MTX64_NumRows(r) = rk;
true
gap> MTX64_NumCols(r) = MTX64_NumCols(m1)-rk;
true
gap> MTX64_LengthOfBitString(cs) = MTX64_NumCols(m1);
true
gap> MTX64_WeightOfBitString(cs) = rk;
true
gap> RankMat(m1) = rk;
true
gap> RankMat(MTX64_ColSelect(cs,m1)[1]) = rk;
true
gap> f := MTX64_FiniteField(11);
<MTX64 GF(11)>
gap> m1 := MakeEchTestMatrix(GF(11), 5000, 2000);
< matrix 5000x5000 : <MTX64 GF(11)>>
gap> MTX64_WriteMatrix(m1, fna);
true
gap> rk := MTX64_fEchelize(tmpdir, fna, fnc, fnb, fnz, fny, fnx);
1600
gap> cs := MTX64_ReadBitString(fnc);
< MTX64 bitstring 1600/5000>
gap> rs := MTX64_ReadBitString(fnb);
< MTX64 bitstring 1600/5000>
gap> r := MTX64_ReadMatrix(fnx);
< matrix 1600x3400 : <MTX64 GF(11)>>
gap> m := MTX64_ReadMatrix(fnz);
< matrix 1600x1600 : <MTX64 GF(11)>>
gap> k := MTX64_ReadMatrix(fny);
< matrix 3400x1600 : <MTX64 GF(11)>>
gap> MTX64_CheckEchelize(m1, rec(rank := rk, multiplier := m,
> cleaner := k, remnant := r, colSelect := cs, rowSelect := rs),
> rec(failIfSingular := false, multiplierNeeded := true,
> cleanerNeeded := true, remnantNeeded := true));
gap> STOP_TEST("functions.tst");
