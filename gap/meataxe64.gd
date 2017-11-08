#
# meataxe64: meataxe64
#
# Declarations
#

#! @Description
#!   Insert documentation for you function here

DeclareCategory( "IsMTX64Object",
                 IsObject );

DeclareOperation( "MTX64_FiniteField", [IsPosInt]);
DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);

DeclareOperation( "MTX64_FiniteFieldElement",
                  [IsMTX64FiniteField, IsPosInt] );
DeclareGlobalFunction( "MTX64_FieldOfElement" );

