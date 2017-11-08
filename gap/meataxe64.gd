#
# meataxe64: meataxe64
#
# Declarations
#

#! @Description
#!   Insert documentation for you function here

DeclareCategory( "IsMTX64FiniteField",
                 IsObject );

DeclareCategory( "IsMTX64FiniteFieldElement",
                 IsObject );

DeclareCategory( "IsMTX64Matrix",
                 IsObject );

DeclareOperation( "MTX64_FiniteField", [IsPosInt]);

DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);

DeclareOperation( "MTX64_FiniteFieldElement",
                  [IsMTX64FiniteField, IsInt] );

DeclareGlobalFunction( "MTX64_FieldOfElement" );

