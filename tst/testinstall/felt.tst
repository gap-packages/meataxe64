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
<2 : <MTX64 GF(5^3)>>
gap> z := MTX64_FiniteFieldElement(f,Z(125)^2);
<25 : <MTX64 GF(5^3)>>
gap> z7 := MTX64_FiniteFieldElement(f, Z(625));
Error, Element not in field
gap> z7 := MTX64_FiniteFieldElement(f, 0*Z(5));
<0 : <MTX64 GF(5^3)>>
gap> zz := Z(2,52);;
gap> z2 := zz^((2^52-1)/(2^26-1));;
gap> f := MTX64_FiniteField(2,26);;
gap> MTX64_FiniteFieldElement(f,z2);
<2 : <MTX64 GF(2^26)>>
gap> MTX64_FieldOfElement(y);
<MTX64 GF(5^3)>
gap> x*x = z;
true
gap> x = y;
false
gap> MTX64_ExtractFieldElement(x);
5
gap> x < y;
false
gap> Zero(x);
<0 : <MTX64 GF(5^3)>>
gap> One(z);
<1 : <MTX64 GF(5^3)>>
gap> y+z;
<27 : <MTX64 GF(5^3)>>
gap> x-z;
<30 : <MTX64 GF(5^3)>>
gap> -x;
<20 : <MTX64 GF(5^3)>>
gap> y^-1;
<3 : <MTX64 GF(5^3)>>
gap> x/z;
<79 : <MTX64 GF(5^3)>>
gap> FFEfromFELT(x);
Z(5^3)
gap> Zero(MTX64_FieldOfElement(y));
<0 : <MTX64 GF(5^3)>>
gap> One(MTX64_FieldOfElement(z));
<1 : <MTX64 GF(5^3)>>
gap> STOP_TEST("felt.tst");
