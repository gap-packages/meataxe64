# Test for basic properties of MTX64 Field objects
#
gap> START_TEST("field.tst");
gap> f := MTX64_FiniteField(2);
<MTX64 GF(2)>
gap> FamilyObj(f);
<Family: "MTX64_FieldFamily">
gap> f9 := MTX64_FiniteField(3,2);
<MTX64 GF(3^2)>
gap> f9 := MTX64_FiniteField(3,2);
<MTX64 GF(3^2)>
gap> fe := MTX64_FiniteField(6);
Error, MTX64_FiniteField: field order must be a prime power < 2^64
gap> IsIdenticalObj(f, MTX64_FiniteField(2));
true
gap> f = f9;
false
gap> f = f;
true
gap> f < f9;
true
gap> f9 < MTX64_FiniteField(3);
false
gap> MTX64_FieldOrder(f);
2
gap> MTX64_FieldCharacteristic(f9);
3
gap> MTX64_FieldDegree(f);
1
gap> MTX64_FieldDegree(f9);
2
gap> MTX64_CREATE_FIELD("foo");
Error, MTX64_CreateField: argument must be a prime power < 2^64, not a list (s\
tring)
gap> MTX64_CREATE_FIELD(-1);
Error, MTX64_CreateField: argument must be a prime power < 2^64, not a integer
gap> STOP_TEST("field.tst");
