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
gap> 
gap> x := MTX64_FiniteFieldElement(f,1);
<1 : <MTX64 GF(61)>>
gap> MTX64_SetEntry(m,7,11,x);          
gap> MTX64_SetEntry(m,0,11,x+x);
gap> MTX64_SetEntry(m,7,0,x+x+x);
gap> MTX64_GetEntry(m,8,11);     
Error, Meataxe64: index out of range
gap> MTX64_SetEntry(m,8,11,x);
Error, Meataxe64: index out of range
gap> 
gap> STOP_TEST("matrix.tst");
