# Test for basic properties of MTX64 Matrix objects
#
gap> START_TEST("matrix.tst");
gap> f := MTX64_FiniteField(61);
<MTX64 GF(61)>
gap> m := MTX64_NewMatrix(f,8,12);
< matrix 8x12 : <MTX64 GF(61)>>
gap> MTX64_Matrix_NumRows(m);
8
gap> MTX64_Matrix_NumCols(m);
12
gap> MTX64_GetEntry(m,0,0);
<0 : <MTX64 GF(61)>>
gap> MTX64_GetEntry(m,7,11);
<0 : <MTX64 GF(61)>>
gap> x := MTX64_FiniteFieldElement(f,1);
<1 : <MTX64 GF(61)>>
gap> MTX64_SetEntry(m,7,11,x);    
gap> Display(m);
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  1
gap> MTX64_SetEntry(m,0,11,x+x);
gap> Display(m);
  .  .  .  .  .  .  .  .  .  .  .  2
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  1
gap> MTX64_SetEntry(m,7,0,x+x+x);
gap> Display(m);
  .  .  .  .  .  .  .  .  .  .  .  2
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  .  .  .  .  .  .  .  .  .  .  .  .
  3  .  .  .  .  .  .  .  .  .  .  1
gap> MTX64_GetEntry(m,8,11);     
Error, Meataxe64: index out of range
gap> MTX64_SetEntry(m,8,11,x);
Error, Meataxe64: index out of range
gap> m := MTX64_Matrix(Z(7)*[[1,2,3],[4,5,6],[0,1,2]]);
< matrix 3x3 : <MTX64 GF(7)>>
gap> Display(m);
 3 6 2
 5 1 4
 . 3 6
gap> m2 := MTX64_NewMatrix(MTX64_FiniteField(7),6,3);
< matrix 6x3 : <MTX64 GF(7)>>
gap> MTX64_DCpy(m, m2, 1, 1);
gap> Display(m2);
 5 1 4
 . . .
 . . .
 . . .
 . . .
 . . .
gap> MTX64_DCpy(m, m2, 4, 1);
Error, Meataxe64: row range too large for matrix: 5 3
gap> MTX64_DCpy(m, m2, 1, 4);
Error, Meataxe64: row range too large for matrix: 5 3
gap> m3 := MTX64_NewMatrix(MTX64_FiniteField(7),6,4);
< matrix 6x4 : <MTX64 GF(7)>>
gap> MTX64_DCpy(m, m3, 1, 2);                        
Error, Meataxe64 matrices not same width
gap> m4 := MTX64_NewMatrix(MTX64_FiniteField(11),6,3);
< matrix 6x3 : <MTX64 GF(11)>>
gap> MTX64_DCpy(m, m4, 1, 2);                         
Error, Meataxe64 matrices not over the same field
gap> m2 := MTX64_NewMatrix(MTX64_FiniteField(7),6,3);
< matrix 6x3 : <MTX64 GF(7)>>
gap> MTX64_DCut(m,1,2,1,m2);
gap> Display(m2);
 1 4 .
 3 6 .
 . . .
 . . .
 . . .
 . . .
gap> MTX64_DCut(m,1,2,1,m2);
gap> MTX64_DCut(m,0,7,0,m2);
Error, Meataxe64: row range too large for matrix: 7 3
gap> MTX64_DCut(m,7,7,0,m2);
Error, Meataxe64: row range too large for matrix: 14 3
gap> MTX64_DCut(m,7,7,0,m4);
Error, Meataxe64 matrices not over the same field
gap> for d in [1,2,3,4,5,16,31,32,33,100,201] do
> for q in [2,3,4,5,7,8,13,16,256,257,
>       NextPrimeInt(2^16), 2^24, NextPrimeInt(2^32)] do
>     testRoundtrip(d,q); od; od;
gap> STOP_TEST("matrix.tst");

