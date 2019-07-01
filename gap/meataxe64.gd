#
# meataxe64: meataxe64
#
# Declarations
#

#!
#! @Description
#!  The category of MeatAxe64 finite fields. There is one such field for 
#!  every prime power up to  <M>2^{64}</M> inclusive. There should be only one &GAP;
#!  object, up to identity, which represents a field of any given order.
#!  They are unrelated to other finite field objects in 
#!  &GAP; and are not fields in the &GAP; sense
DeclareCategory( "IsMTX64FiniteField",
        IsObject );

#! @Description
#!  The category of MeatAxe64 bitstrings. Used heavily in Gaussian elimination
#!  to indicate locations of pivot rows and columns. 
DeclareCategory( "IsMTX64BitString",
        IsObject );

#! @Description
#!  The category of MeatAxe64 finite field elements. They are not equal to
#!  other finite field elements in &GAP; and  there is no automatic inclusion
#!  of subfields.
DeclareCategory( "IsMTX64FiniteFieldElement",
        IsScalar and IsCommutativeElement );

#! @Description
#!  The category of MeatAxe64 matrices. They are not matrices in the &GAP;
#!  sense (not even matrix objects) and are not lists of lists. There are no vectors,
#!  if needed they are represented by matrices of one row. MeatAxe64 matrices 
#!  know their dimensions and field. 
DeclareCategory( "IsMTX64Matrix",
        IsScalar );

#! @Arguments q
#! @Returns a MeatAxe64 finite field
#! @Description
#!   retrieves the MeatAxe64 finite field of order <A>q</A> which must be
#!   be a prime power less than or equal to <M>2^{64}</M>, creating it if needed.
DeclareOperation( "MTX64_FiniteField", [IsPosInt]);

#! @Arguments p, d
#! @Returns a MeatAxe64 finite field
#! @Description
#!   retrieves the MeatAxe64 finite field of order <M>p^d</M> 
#!   which must be a prime power less than or equal to <M>2^{64}</M>, creating it if needed.
DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);

#! @Arguments f, x
#! @Returns a MeatAxe64 finite field element
#! @Description
#!   creates the MeatAxe64 finite field element numbered <A>x</A> in the 
#!   field <A>f</A>. The numbering of elements is defined in the MeatAxe64 
#!   C library.
DeclareOperation( "MTX64_FiniteFieldElement",        
        [IsMTX64FiniteField, IsInt] );

#! @Arguments fld, ffe
#! @Returns a MeatAxe64 finite field element
#! @Description
#!   creates the MeatAxe64 finite field element corresponding to the &GAP;
#!   finite field elemnent <A>ffe</A> in the field <A>fld</A>. 
DeclareOperation( "MTX64_FiniteFieldElement",
        [IsMTX64FiniteField, IsFFE] );

#! @Arguments felt
#! @Returns the MeatAxe64 finite field in which <A>felt</A> is defined
DeclareGlobalFunction( "MTX64_FieldOfElement" );

#! @BeginGroup MTX64_Matrix
#! @Description constructs a Meataxe64 matrix from a &GAP;
#!  matrix, plus optional specification of the field and dimensions
#!  of the matrix. Specification of the dimensions is useful 
#!  in the case where the number of rows is zero. 
#! @Arguments m
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl] );
#! @Arguments m, fld
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, 
        IsField and IsFFECollection and IsFinite] );
#! @Arguments m, q
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, IsPosInt] );
#! @Arguments m, q, nor, noc
DeclareOperation( "MTX64_Matrix", [IsMatrix and IsFFECollColl, 
        IsPosInt, IsInt, IsInt] );
#! @EndGroup

#! @Arguments m
#! @Returns a &GAP; matrix (compressed where appropriate)
#!   equal to the MeatAxe64 matrix <A>m</A>
DeclareOperation( "MTX64_ExtractMatrix", [IsMTX64Matrix]);

#! @Arguments m
#! @Returns the MeatAxe64 finite field over which <A>m</A> is defined
DeclareGlobalFunction( "MTX64_FieldOfMatrix" );

