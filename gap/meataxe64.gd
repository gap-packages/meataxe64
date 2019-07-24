#
# meataxe64: meataxe64
#
# Declarations
#

#! @Chapter datatypes
#! @ChapterTitle Basic Meataxe64 DataTypes
#! The Meataxe64 system has its own representations of finite fields, their
#! elements, and matrices over them, as well as a bitstring used to indicate a
#! selection of rows or columns from a matrix. Note that there is no vector
#! type, a vector is simply a matrix with one row. Note further that working
#! with such vectors is relatively inefficient, and should be avoided where possible.
#! 
#! The Meataxe64 &GAP; package provides &GAP; objects which "wrap" these
#! Meataxe64 objects, which are defined in this chapter.

#! @Section Defining Categories
 
#!
#! @Description
#!  The category of MeatAxe64 finite fields. There is one such field for 
#!  every prime power up to  <M>2^{64}</M> inclusive. There should be only one &GAP;
#!  object, up to identity, which represents a field of any given order.
#!  They are unrelated to other finite field objects in 
#!  &GAP; and are not fields (or even collections) in the &GAP; sense
DeclareCategory( "IsMTX64FiniteField",
        IsObject );

#! @Description
#!  The category of MeatAxe64 finite field elements. They are not equal to
#!  other finite field elements in &GAP; and  there is no automatic inclusion
#!  of subfields.
DeclareCategory( "IsMTX64FiniteFieldElement",
        IsScalar and IsCommutativeElement );

#! @Description
#!  The category of MeatAxe64 matrices. They are not matrices in the &GAP;
#!  sense (not even matrix objects) and are not lists of lists. MeatAxe64 matrices 
#!  know their dimensions and defining field. 
DeclareCategory( "IsMTX64Matrix",
        IsScalar );
#! 
#! @Description
#!  The category of MeatAxe64 bitstrings. Used heavily in Gaussian elimination
#!  to indicate locations of pivot rows and columns. 
DeclareCategory( "IsMTX64BitString",
        IsObject );

#! @Section Basic Construction and Access Operations
#! @BeginGroup MTX64_FiniteField
#! @Description
#!   retrieves a MeatAxe64 finite field of given order, which must be
#!   be a prime power less than or equal to <M>2^{64}</M>, creating it if needed.
#!   If the order is not a prime power an error is generated.
#! @Arguments q
#! @Returns a MeatAxe64 finite field of order <A>q</A>
DeclareOperation( "MTX64_FiniteField", [IsPosInt]);

#! @Arguments p, d
#! @Returns a MeatAxe64 finite field of order <M>p^d</M>
DeclareOperation( "MTX64_FiniteField", [IsPosInt, IsPosInt]);
#! @EndGroup

#! <ManSection>
#!  <Func Name="MTX64_FieldOrder" Arg="f"/>
#!  <Func Name="MTX64_FieldCharacteristic" Arg="f"/>
#!  <Func Name="MTX64_FieldDegree" Arg="f"/>
#! <Description>
#! These functions provide access to basic information about a MeatAxe64 field.
#! </Description></ManSection>
#! 
 

#! @BeginGroup MTX64_FiniteFieldElement
#! @Description creates a MeatAxe64 finite field element in a given field
#! <A>f</A>. The element may be specified by number: the numbering of elements runs from 0 to <M>q-1</M> and is defined
#! in the MeatAxe64 C library. It is guaranteed that element 0 is the zero of
#! the field and element 1 is the one of the field. Alternatively the element
#! may be specified by giving the corresponding &GAP;
#! finite field element. 
#! @Arguments f, x 
DeclareOperation( "MTX64_FiniteFieldElement",        
        [IsMTX64FiniteField, IsInt] );
#! 
#! @Arguments fld, ffe
DeclareOperation( "MTX64_FiniteFieldElement",
        [IsMTX64FiniteField, IsFFE] );
#! @EndGroup

#! @Arguments felt
#! @Returns the MeatAxe64 finite field in which <A>felt</A> is defined
DeclareGlobalFunction( "MTX64_FieldOfElement" );
#! 
#! <ManSection>
#!  <Func Name="MTX64_NewMatrix" Arg="f, nor, noc"/>
#! <Description>
#! Returns a new mutable zero matrix over the field <A>f</A> with <A>nor</A> rows
#! and <A>noc</A> columns.</Description></ManSection>

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
#! @Description Constructs a new Meataxe64 matrix with <A>n</A> rows and
#! <A>m</A> columns over the field <A>f</A>.The entries are uniformly
#! pseudo-randomly chosen from <A>f</A> using the <Ref
#! Var="GlobalMersenneTwister" BookName="ref"/> random source. This function is significantly
#! more efficient that filling in random entries individually, or using <Ref
#! Func="RandomMat" BookName="ref"/> and then converting the result. 
#! @Arguments f, n, m
DeclareGlobalFunction( "MTX64_RandomMat" );
#! 
#! 
#! <ManSection>
#!  <Func Name="MTX64_NumCols" Arg="m"/>
#!  <Func Name="MTX64_NumRows" Arg="m"/> <Description>
#! These function return the dimensions of the matrix. Unlike in the C API
#! every  matrix knows how many rows it has. Unlike in &GAP;
#! even a matrix with no rows knows how many columns it has.</Description></ManSection>
#! 
#! <ManSection>
#!  <Func Name="MTX64_GetEntry" Arg="m, i, j"/>
#!  <Func Name="MTX64_SetEntry" Arg="m, i, j, x"/> <Description>
#! <C>MTX64_GetEntry</C> and <C>MTX64_SetEntry</C> provide
#! access to individual entries of matrices.  <A>i</A> and <A>j</A> are zero based
#! row and column indices respectively and <A>x</A> is Meataxe64 finite field
#! element defined over the same field as <A>m</A>.</Description></ManSection>

#! @Arguments m
#! @Returns the MeatAxe64 finite field over which <A>m</A> is defined
DeclareGlobalFunction( "MTX64_FieldOfMatrix" );

#! 
#! @Arguments m, v, row
#! @Description Copies the &GAP; vector <A>v</A> as row <A>row</A> of the mutable Meataxe64
#! matrix <A>m</A>. Row numbering is zero-based. An error will be given if the
#! vector and matrix are not compatible. For &GAP;'s built-in comrpessed
#! vectors over small fields this will be much more efficient than inserting
#! the entries one by one.
DeclareGlobalFunction( "MTX64_InsertVector" );

#! @Arguments m, row
#! @Description Returns a &GAP; vector equal to row <A>row</A> (zero-based) of
#! the Meataxe64 matrix <A>m</A>. When the field of <A>m</A> is small enough,
#! the vector will be compressed.
DeclareGlobalFunction( "MTX64_ExtractVector" );
#! 
#! 
#! @Arguments m
#! @Returns a &GAP; matrix (compressed where appropriate)
#!   equal to the MeatAxe64 matrix <A>m</A>
DeclareOperation( "MTX64_ExtractMatrix", [IsMTX64Matrix]);

#! @Arguments m, startrow, numrows, startcol, numcols
#! @Description returns a copy of the submatrix of <A>m</A> specified by the indices.
#! <A>startrow</A> and <A>startcol</A> are one based.
DeclareGlobalFunction( "MTX64_Submatrix" );
 
#!
#! <ManSection> 
#! <Func Name="MTX64_EmptyBitString" Arg="len"/>
#!  <Description>Creates a new bitstring of length <A>len</A> with no set
#! bits</Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_GetEntryOfBitstring" Arg="bitstring, position"/>
#!  <Description> <C>MTX64_GetEntryOfBitstring</C> returns 0 or 1 according to
#! whether the <A>position</A> entry of bitstring <A>bitstring</A> is unset or set.
#!  </Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_SetEntryOfBitstring" Arg="bitstring, position"/>
#!  <Description> <C>MTX64_SetEntryOfBitstring</C> sets the 
#!  <A>position</A> entry of bitstring <A>bitstring</A> (to 1). There is no way
#! to set an entry to zero in the underlying Meataxe64 C library.
#!  </Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_BitStringBlist" Arg="blist"/>
#! <Func Name="MTX64_BlistBitstring" Arg="bitstring"/>
#!  <Description>Convert between the MeatAxe64 bitstrings and GAP's Boolean lists</Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_LengthOfBitString" Arg="bitstring"/>
#! <Func Name="MTX64_WeightOfBitstring" Arg="bitstring"/>
#!  <Description>The length of a bitstring is the number of bits in it, the
#! weight is the number of one bits.</Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_PositionsBitString" Arg="bitstring"/>
#!  <Description>Returns a &GAP; list containing the indices of the set bits of
#! <A>bitstring</A> 
#! </Description> 
#! </ManSection>
#! 

#! @Section Additional Matrix Functions
#! 
#! <ManSection> 
#! <Func Name="MTX64_CompareMatrices" Arg="m1, m2"/>
#!  <Description> This function underlies the comparison of matrices used by
#! <C>&lt;</C> and <C>=</C>. For compatible matrices <A>m1</A> and <A>m2</A> it
#! returns a negative, zero or positive integer according to whether <A>m1</A>
#! is less than, equal to, or greater than <A>m2</A>.
#! </Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_ReadMatrix" Arg="fn"/>
#! <Func Name="MTX64_WriteMatrix" Arg="m, fn"/>
#!  <Description> These functions allow reading and writing of matrices to disk
#! in the Meataxe64 file format, which is defined in the C library documentation
#! </Description> 
#! </ManSection>
#! 
#! <ManSection> 
#! <Func Name="MTX64_HashMatrix" Arg="m"/>
#!  <Description> This function implements an efficient hash function for a
#! MeatAxe64 matrix. All entries of the matrix contribute to the hash.
#! </Description> 
#! </ManSection>
#! 
#!
#! @Section Additional Bitstring Functions
#! <ManSection> 
#! <Func Name="MTX64_CompareBitStrings" Arg="b1, b2"/>
#!  <Description> This function underlies the comparison of bitstrings used by
#! <C>&lt;</C> and <C>=</C>. For bitstrings <A>b1</A> and <A>b2</A> it
#! returns a negative, zero or positive integer according to whether <A>b1</A>
#! is less than, equal to, or greater than <A>b2</A>.
#! </Description> 
#! </ManSection>

#! 
#! @Arguments bs, m 
#! @Returns a new Meataxe64 matrix containing only those rows of <A>m</A> whose
#! corresponding entry in <A>bs</A> is set. The length of <A>bs</A> must match
#! the number of rows of <A>m</A>
DeclareGlobalFunction( "MTX64_RowSelect");
#! 
#! <ManSection> 
#! <Func Name="MTX64_ColSelect" Arg="bitstring, matrix"/>
#!  <Description> <C>MTX64_ColSelect</C> returns a matrix composed of the
#!  columns of <A>matrix</A> corresponding to the set bits in <A>bitstring</A>
#! whose length should match the number columns of <A>matrix</A>. The return
#! value this has as many columns as the weight of <A>bitstring</A> and as many
#! rows as <A>matrix</A>
#!  </Description> 
#! </ManSection>
