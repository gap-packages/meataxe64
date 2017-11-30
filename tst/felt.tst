# Test for basic properties of MTX64 Field elements
#
gap> START_TEST("felt.tst");
gap> f := MTX64_FiniteField(125);
<MTX64 GF(5^3)>
gap> x := MTX64_FiniteFieldElement(f,5);
<5 : <MTX64 GF(5^3)>>
gap> FieldOfMTX64FELT(x);
<MTX64 GF(5^3)>
gap> y := MTX64_FiniteFieldElement(f,Z(5));
<28 : <MTX64 GF(5^3)>>
gap> z := MTX64_FiniteFieldElement(f,Z(125)^2);
<5 : <MTX64 GF(5^3)>>
gap> MTX64_FieldOfElement(y);
<MTX64 GF(5^3)>
gap> x*x = z;
false
gap> x = y;
false
gap> MTX64_ExtractFieldElement(x);
5
gap> x < y;
true
gap> Zero(x);
<0 : <MTX64 GF(5^3)>>
gap> One(z);
<1 : <MTX64 GF(5^3)>>
gap> y+z;
<33 : <MTX64 GF(5^3)>>
gap> x-z;
<10 : <MTX64 GF(5^3)>>
gap> -x;
<20 : <MTX64 GF(5^3)>>
gap> y^-1;
<15 : <MTX64 GF(5^3)>>
gap> x/z;
<1 : <MTX64 GF(5^3)>>
gap> FFEfromFELT(x);
Z(5^3)
gap> Zero(MTX64_FieldOfElement(y));
<0 : <MTX64 GF(5^3)>>
gap> One(MTX64_FieldOfElement(z));
<1 : <MTX64 GF(5^3)>>
gap> STOP_TEST("felt.tst");
