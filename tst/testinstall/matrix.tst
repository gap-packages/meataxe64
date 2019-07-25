# Test for basic properties of MTX64 Matrix objects
#
gap> START_TEST("matrix.tst");
gap> Read(Filename(DirectoriesPackageLibrary("meataxe64","tst"),"testutils.g"));
gap> f := MTX64_FiniteField(61);
<MTX64 GF(61)>
gap> m := MTX64_NewMatrix(f,8,12);
< matrix 8x12 : <MTX64 GF(61)>>
gap> MTX64_NumRows(m);
8
gap> MTX64_NumCols(m);
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
> for q in [2,3,4,5,7,8,13,16,256,257,512,
>       NextPrimeInt(2^16), 2^24, NextPrimeInt(2^32)] do
>     testRoundtrip(d,q); od; od;
gap> repeat m := MTX64_RandomMat(MTX64_FiniteField(13^2),100,100); until RankMat(m) = 100;
gap> mi := Inverse(m);;
gap> mism := InverseSameMutability(m);;
gap> mim := InverseMutable(m);;
gap> mi = mism;
true
gap> mi = mim;
true
gap> IsMutable(mim);
true
gap> IsMutable(mism);
true
gap> IsMutable(mi);
false
gap> IsOne(mi*m);
true
gap> IsOne(m*mi);
true
gap> o := One(m);
< matrix 100x100 : <MTX64 GF(13^2)>>
gap> IsMutable(o);
false
gap> IsOne(o);
true
gap> om := OneMutable(m);
< matrix 100x100 : <MTX64 GF(13^2)>>
gap> m2 := MTX64_NewMatrix(MTX64_FiniteField(11),3,2);;
gap> OneMutable(m2);
Error, Not square
gap> IsOne(om);
true
gap> IsOne(MTX64_Matrix(Z(23)^0*[[1,1],[1,1]],23));
false
gap> IsMutable(om);
true
gap> osm := OneSameMutability(m);
< matrix 100x100 : <MTX64 GF(13^2)>>
gap> IsOne(osm);
true
gap> IsMutable(osm);
true
gap> om = osm and o = om;
true
gap> IsMutable(OneSameMutability(mi));
false
gap> mi2 := InverseSameMutability(mi);;
gap> IsMutable(mi2);
false
gap> mi2 = m;
true
gap> m := MTX64_RandomMat(MTX64_FiniteField(17),100,101);
< matrix 100x101 : <MTX64 GF(17)>>
gap> mi := AdditiveInverse(m);;
gap> mism := AdditiveInverseSameMutability(m);;
gap> mim := AdditiveInverseMutable(m);;
gap> mi = mism;
true
gap> mi = mim;
true
gap> IsMutable(mim);
true
gap> IsMutable(mism);
true
gap> IsMutable(mi);
false
gap> IsZero(mi+m);
true
gap> IsZero(m+mi);
true
gap> mi2 := AdditiveInverseSameMutability(mi);;
gap> IsMutable(mi2);
false
gap> mi2 = m;
true
gap> IsZero(m-mi);
false
gap> m := MTX64_RandomMat(MTX64_FiniteField(512),73,52);
< matrix 73x52 : <MTX64 GF(2^9)>>
gap> mi := AdditiveInverse(m);;
gap> mism := AdditiveInverseSameMutability(m);;
gap> mim := AdditiveInverseMutable(m);;
gap> mi = mism;
true
gap> mi = mim;
true
gap> IsMutable(mim);
true
gap> IsMutable(mism);
true
gap> IsMutable(mi);
false
gap> IsZero(mi+m);
true
gap> IsZero(m+mi);
true
gap> mi2 := AdditiveInverseSameMutability(mi);;
gap> IsMutable(mi2);
false
gap> mi2 = m;
true
gap> mi = m;
true
gap> IsZero(mi-mi);
true
gap> zm := ZeroMutable(m);
< matrix 73x52 : <MTX64 GF(2^9)>>
gap> IsMutable(zm);
true
gap> IsZero(zm);
true
gap> IsOne(MTX64_Matrix(Z(2)*[[1,0]], 2));
false
gap> MTX64_Matrix([],3);
Error, no method found! For debugging hints type ?Recovery from NoMethodFound
Error, no 2nd choice method found for `MTX64_Matrix' on 2 arguments
gap> MTX64_Matrix([[]],GF(3));
< matrix 1x0 : <MTX64 GF(3)>>
gap> MTX64_Matrix([[],[]],4,2,0);
< matrix 2x0 : <MTX64 GF(2^2)>>
gap> MTX64_Matrix([[],1],9);
Error, no method found! For debugging hints type ?Recovery from NoMethodFound
Error, no 2nd choice method found for `MTX64_Matrix' on 2 arguments
gap> m1 := MTX64_RandomMat(MTX64_FiniteField(17,2), 10,15);;
gap> m2 := MTX64_RandomMat(MTX64_FiniteField(17,2), 11,15);;
gap> m3 := MTX64_RandomMat(MTX64_FiniteField(17,2), 10,16);;
gap> m4 := MTX64_RandomMat(MTX64_FiniteField(17,3), 10,15);;
gap> m1 < m2;
Error, No ordering for matrices of different shapes
gap> m1 < m3;
Error, No ordering for matrices of different shapes
gap> m1 < m4;
Error, no method found! For debugging hints type ?Recovery from NoMethodFound
Error, no 1st choice method found for `<' on 2 arguments
gap> m1 < m1;
false
gap> Zero(m1) < m1;
true
gap> m1 := MTX64_Matrix(Z(7)*[[1,2,3,4],[5,6,7,8]]);
< matrix 2x4 : <MTX64 GF(7)>>
gap> sm1 := MTX64_Submatrix(m1, 1, 1, 1, 4);
< matrix 1x4 : <MTX64 GF(7)>>
gap> Display(sm1);
 3 6 2 5
gap> sm2 := MTX64_Submatrix(m1, 1, 2, 2, 2);
< matrix 2x2 : <MTX64 GF(7)>>
gap> Display(sm2);
 6 2
 4 .
gap> m1 := MTX64_Matrix(Z(257)*[[1,2,3,4],[5,6,7,8]]);;
gap> MTX64_InsertVecFFE(m1, [Z(257)],0);
Error, row length mismatch
gap> m1 := MTX64_Matrix(Z(121)*[[1,2,3,4],[5,6,7,8]],121);;
gap> MTX64_InsertVecFFE(m1, Z(11)*[1,3,2,4],0);
fail
gap> m1 := MTX64_NewMatrix(MTX64_FiniteField(257), 3, 0);;
gap> MTX64_ExtractVecFFE(m1,0);
[  ]
gap> MTX64_InsertVecFFE(m1,[],1);
gap> m1 := MTX64_Matrix(Z(2^2)^0*[[1,1],[0,1]],4);;
gap> x := MTX64_FiniteFieldElement(MTX64_FiniteField(4),2);;
gap> MTX64_DSMad(2, x, m1, m1);
gap> Display(m1);
z = Z(4)
 z^2 z^2
   . z^2
gap> m := MTX64_NewMatrix(MTX64_FiniteField(4),10,10);;
gap> v := Z(2)*[1..10];; ConvertToVectorRep(v,2);;
gap> MTX64_InsertVector(m,v,1);
gap> Display(m);
 . . . . . . . . . .
 1 . 1 . 1 . 1 . 1 .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
gap> MTX64_ExtractVecGF2(m, 1);
Error, field mismatch
gap> m := MTX64_NewMatrix(MTX64_FiniteField(2),10,10);;
gap> MTX64_InsertVecGF2(m, "foo", 1);
Error, MTX64_InsertVecGF2: vector should be a compressed GF2 vector
gap> MTX64_InsertVecGF2(m, Z(2)*[1..10], 1);
Error, MTX64_InsertVecGF2: vector should be a compressed GF2 vector
gap> MTX64_InsertVecGF2(m, Z(4)*[1..10], 1);
Error, MTX64_InsertVecGF2: vector should be a compressed GF2 vector
gap> v := Z(4)*[1..10];; ConvertToVectorRep(v,4);;
gap> MTX64_InsertVecGF2(m, v, 1);
Error, MTX64_InsertVecGF2: vector should be a compressed GF2 vector
gap> v := Z(2)*[1..8];; ConvertToVectorRep(v,2);;
gap> MTX64_InsertVecGF2(m, v, 1);
Error, row length mismatch
gap> m := MTX64_NewMatrix(MTX64_FiniteField(2),10,0);;
gap> v := [Z(2)];; ConvertToVectorRep(v,2);; v := v{[]};;
gap> MTX64_InsertVecGF2(m, v, 1);
gap> MTX64_ExtractVecGF2(m, 1);
<a GF2 vector of length 0>
gap> m := MTX64_NewMatrix(MTX64_FiniteField(9),10,10);;
gap> v := Z(3)*[1..10];; ConvertToVectorRep(v,3);;
gap> MTX64_InsertVector(m,v,1);
gap> Display(m);
 . . . . . . . . . .
 2 1 . 2 1 . 2 1 . 2
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
 . . . . . . . . . .
gap> v := Z(3)*[1..9];; ConvertToVectorRep(v,9);;
gap> MTX64_InsertVec8Bit(m,v,1);
Error, row length mismatch
gap> MTX64_InsertVec8Bit(m, "foo", 1);
Error, MTX64_InsertVec8Bit: bad vector format
gap> MTX64_InsertVec8Bit(m, Z(2)*[1..10], 1);
Error, MTX64_InsertVec8Bit: bad vector format
gap> MTX64_InsertVec8Bit(m, Z(4)*[1..10], 1);
Error, MTX64_InsertVec8Bit: bad vector format
gap> v := Z(4)*[1..10];; ConvertToVectorRep(v,4);;
gap> MTX64_InsertVec8Bit(m, v, 1);
fail
gap> v := Z(2)*[1..10];; ConvertToVectorRep(v,2);;
gap> MTX64_InsertVec8Bit(m, v, 1);
Error, MTX64_InsertVec8Bit: bad vector format
gap> m := MTX64_NewMatrix(MTX64_FiniteField(7),10,0);;
gap> v := [Z(7)];; ConvertToVectorRep(v,7);; v := v{[]};;
gap> MTX64_InsertVec8Bit(m, v, 1);
gap> m := MTX64_NewMatrix(MTX64_FiniteField(7^3),10,10);;
gap> MTX64_ExtractVec8Bit(m, 1);
Error, field mismatch
gap> v := Z(2)*[1..11];; ConvertToVectorRep(v,2);;
gap> MTX64_InsertVector(m, v, 2);
Error, MTX64_InsertVector: row length mismatch
gap> Reset(GlobalMersenneTwister, 33);;
gap> m := MTX64_RandomMat(MTX64_FiniteField(NextPrimeInt(2^63)), 10,10);
< matrix 10x10 : <MTX64 GF(9223372036854775837)>>
gap> d := DirectoryTemporary();;
gap> MTX64_WriteMatrix(m, Filename(d,"a"));
true
gap> m2 := MTX64_ReadMatrix(Filename(d,"a"));
< matrix 10x10 : <MTX64 GF(9223372036854775837)>>
gap> m2 = m;
true
gap> m3 := MTX64_ReadMatrix( 17);
Error, MTX64_ReadMatrix: filename must be a string
gap> MTX64_WriteMatrix( m2, 11);
Error, MTX64_WriteMatrix: filename must be a string
gap> MTX64_HashMatrix(m);
-327586356389823045
gap> MTX64_RANDOM_MAT( MTX64_FiniteField(3), 10, 10, 10);
Error, MTX64_RANDOM_MAT: <mtsource> must be a string (not the integer 10)
gap> MTX64_RANDOM_MAT( MTX64_FiniteField(3), 10, 10, "123");
Error, MTX64_RANDOM_MAT: <mtstr> must be a string with at least 2500 character\
s
gap> f := MTX64_FiniteField(19);;
gap> m1 := MTX64_RandomMat( f, 100, 100);;
gap> m2 := MTX64_RandomMat( f, 101, 100);;
gap> m1*m2;
Error, SLMultiply: matrices are incompatible shapes
gap> m2*m1;
< matrix 101x100 : <MTX64 GF(19)>>
gap> m1*m1 = MTX64_SLMultiplyStrassenSquare(m1, m1, 1);
true
gap> m2*m1 = MTX64_SLMultiplyStrassenNonSquare(m2, m1, 1);
true
gap> MTX64_SLMultiplyStrassenNonSquare(m1, m2, 1);
Error, Matrices Incompatible
gap> MTX64_SLMultiplyStrassenSquare(m2, m1, 1);
Error, Matrices must be square
gap> STOP_TEST("matrix.tst");
