#
# meataxe64: meataxe64
#
# Declarations
#

#! @Description
#!   Insert documentation for you function here
DeclareGlobalFunction( "meataxe64_Example" );

DeclareCategory( "IsMTX64FiniteField",
                 IsField and IsFinite and IsAttributeStoringRep );
DeclareCategory( "IsMTX64DSpace",
                 IsObject );

DeclareOperation( "MTX64_FiniteField", [IsPosInt]);
DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);

DeclareCategory( "IsMTX64FiniteFieldElement",
                 IsFFE );
DeclareOperation( "MTX64_FiniteFieldElement",
                  [IsMTX64FiniteField, IsPosInt] );
DeclareGlobalFunction( "MTX64_FieldOfElement" );

