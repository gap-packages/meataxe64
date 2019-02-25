# Test for basic properties of MTX64 Bitstrings.
# The functions that use them in combination with matrices are tested later
#
gap> START_TEST("bitstring.tst");
gap> bs := MTX64_EmptyBitString(10);
< MTX64 bitstring 0/10>
gap> Display(bs);
[0,0,0,0,0,0,0,0,0,0]
gap> MTX64_LengthOfBitString(bs);
10
gap> MTX64_WeightOfBitString(bs);
0
gap> MTX64_GetEntryOfBitString(bs,3);
0
gap> MTX64_SetEntryOfBitString(bs,3);
gap> MTX64_GetEntryOfBitString(bs,3);
1
gap> MTX64_SetEntryOfBitString(bs,7);
gap> MTX64_SetEntryOfBitString(bs,9);
gap> MTX64_GetEntryOfBitString(bs,11);
Error, MTX64_GetEntryOfBitString: position not an integer or out of range
gap> MTX64_SetEntryOfBitString(bs,11);
Error, MTX64_SetEntryOfBitString: position not an integer or out of range
gap> bs1 := ShallowCopy(bs);
< MTX64 bitstring 3/10>
gap> MTX64_SetEntryOfBitString(bs,0);
gap> Display(bs);
[1,0,0,1,0,0,0,1,0,1]
gap> Display(bs1);
[0,0,0,1,0,0,0,1,0,1]
gap> MTX64_WeightOfBitString(bs);
4
gap> MTX64_SetEntryOfBitString(bs,0);
gap> MTX64_WeightOfBitString(bs);
4
gap> bs[5];
0
gap> bs[12];
Error, Index too big
gap> bs1[3] := 1;
1
gap> bs[12] := 1;
Error, Index too big
gap> bs1[3] := 0;
Error, meataxe64 bitstring entries can ONLY be set to 1
gap> bs1[3] := 2;
Error, meataxe64 bitstring entries can ONLY be set to 1
gap> bs = bs1;
false
gap> bs1 < bs;
false
gap> bsx := MTX64_EmptyBitString(100);;
gap> MTX64_SetEntryOfBitString(bsx, 23);
gap> MTX64_SetEntryOfBitString(bsx, 72);
gap> MTX64_SetEntryOfBitString(bsx, 99);
gap> bsx2 := MTX64_ComplementBitString(bsx);;
gap> bs2 := MTX64_ComplementBitString(bs);
< MTX64 bitstring 6/10>
gap> Display(bsx2);
[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1\
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1\
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0]
gap> MTX64_PositionsBitString(bs2);
[ 2, 3, 5, 6, 7, 9 ]
gap> sub := MTX64_EmptyBitString(6);
< MTX64 bitstring 0/6>
gap> MTX64_PositionsBitString(sub);
[  ]
gap> MTX64_SetEntryOfBitString(sub,1);
gap> MTX64_BSCombine(bs1, sub);
[ < MTX64 bitstring 5/10>, < MTX64 bitstring 4/5> ]
gap> MTX64_BSCombine(bs2, sub);
Error, MTX64_BSCombine: bitstrings incompatible
gap> bl := BlistList([1..1000],Primes);;
gap> bs := MTX64_BitStringBlist(bl);
< MTX64 bitstring 168/1000>
gap> bl = MTX64_BlistBitString(bs);
true
gap> b1 := MTX64_EmptyBitString(10);;
gap> b2:= MTX64_EmptyBitString(10);;
gap> MTX64_BSShiftOr(b1, 3, b2);
Error, BSShiftOr: destination bitstring not long enough
gap> MTX64_BitStringBlist([true]);
< MTX64 bitstring 1/1>
gap> MTX64_BitStringBlist([17]);
< MTX64 bitstring 0/1>
gap> STOP_TEST("bitstring.tst");
