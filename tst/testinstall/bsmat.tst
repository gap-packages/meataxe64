# 
# Test for functions that combine bit strings and matrices.
# most of these are tested in the echelize code, so this is mainly tests of error
# cases and similar needed for coverage
#
gap> START_TEST("bitstring.tst");
gap> f := MTX64_FiniteField(49);;
gap> m := MTX64_RandomMat(f, 100, 100);;
gap> bs := MTX64_EmptyBitString(99);;
gap> MTX64_ColSelect(bs, m);
Error, mismatched row length
gap> MTX64_RowSelectShifted(bs, m, 2);
Error, mismatched matrix length: matrix 100, bitstring + shift 101
gap> bs := MTX64_EmptyBitString(199);;
gap> for i in [3,5..199] do
> MTX64_SetEntryOfBitString(bs, i-1);
> od;
gap> MTX64_RowCombine(bs, m, m);
Error, MTX64_RowCombine: matrices and bitstring don't match
gap> MTX64_SetEntryOfBitString(bs, 0);;
gap> MTX64_RowCombine(bs, m, m);
Error, MTX64_RowCombine: matrices and bitstring don't match
gap> m := MTX64_RandomMat(f, 100, 73);;
gap> MTX64_BSColRifZ(bs, m);
Error, mismatched row length
gap> MTX64_BSColPutS(bs, m, One(f));
Error, mismatched row length
gap> STOP_TEST("bitstring.tst");
