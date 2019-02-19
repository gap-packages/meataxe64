#
# meataxe64: meataxe64
#
# Declarations
#

#! @Description
#!   Insert documentation for you function here

DeclareCategory( "IsMTX64FiniteField",
                 IsObject );

DeclareCategory( "IsMTX64BitString",
                 IsObject );

DeclareCategory( "IsMTX64FiniteFieldElement",
        IsScalar and IsCommutativeElement );

DeclareCategory( "IsMTX64Matrix",
                 IsScalar );

DeclareOperation( "MTX64_FiniteField", [IsPosInt]);

DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);

DeclareOperation( "MTX64_FiniteFieldElement",
                  [IsMTX64FiniteField, IsInt] );

DeclareOperation( "MTX64_FiniteFieldElement",
                  [IsMTX64FiniteField, IsFFE] );

DeclareGlobalFunction( "MTX64_FieldOfElement" );

DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl] );
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, 
        IsField and IsFFECollection and IsFinite] );
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, IsPosInt] );
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, 
        IsPosInt, IsInt, IsInt] );

DeclareOperation( "MTX64_ExtractMatrix", [IsMTX64Matrix]);


