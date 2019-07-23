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
#! Var="GlobalMersenneTwister"/> random source. This function is significantly
#! more efficient that filling in random entries individually, or using <Ref
#! Func="RandomMat"/> and then converting the result. 
#! @Arguments f, n, m
DeclareGlobalFunction( "MTX64_RandomMat" );
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
#! @Arguments bs, m 
#! @Returns a new Meataxe64 matrix containing only those rows of <A>m</A> whose
#! corresponding entry in <A>bs</A> is set. The length of <A>bs</A> must match
#! the number of rows of <A>m</A>
DeclareGlobalFunction( "MTX64_RowSelect");

#! @Section Standard Operations applicable to Meataxe64 objects
#! 
#! Methods are installed for many standard operations applied to Meataxe64
#! objects. In this section we briefly list the more important ones  with 
#! some notes where the behaviour may not be as expected.
#! 
#! <Ref Oper="&lt;"/> for Meataxe64 fields orders fields by their size. For
#! Meataxe64 finite field elements, it orders them according to the internal
#! numbering used by the C meataxe.
#!
#! Standard arithmetic operations such as <Ref Attr="Zero"/> and <Ref
#! Attr="*"/> are installed for Meataxe64 finite field elements, and where
#! relevant for the fields.
#!
#! A limited set of Collection operations such as <Ref Attr="Size"/>, <Ref
#! Attr="AsList"/>, <Ref Attr="AsSSortedList"/> and <Ref Attr="Random"/> are
#! provided for Meataxe64 finite fields, for convenience.
#! 
#! The list access operation <C>\[\]</C> can be used to do the same
#! thing as <Ref Func="MTX64_ExtractVector"/>, except that in this case the row
#! indexing is one based.
#! 
#! The list assignment operation <C>\[\]\:\=</C> can be used to do the same
#! thing as <Ref Func="MTX64_InsertVector"/>, except that in this case the row
#! indexing is one based.
#! 
#! <Ref Oper="ShallowCopy"/> applied to a Meataxe64 matrix produces a new
#! matrix which does not share its rows with the original, unlike standard
#! &GAP;  matrices.
#! 
#! Arithmetic operations are installed for operations among Meataxe64 matrices and
#! between matrices and finite field elements. There is no automatic coercion
#! between fields, and matrix dimensions must match correctly (unbound entries
#! are not treated as zero). This includes transposition.
#!
#! <Ref Oper="MatElm"/> and <Ref Oper="SetMatElm"/> methods are installed for
#! matrices which do the same as <Ref Func="MTX64_GetEntry"/> and
#! <Ref Func="MTX64_SetEntry"/> but using one-based indexing. This supports access
#! like <C>m[i,j]</C> for reading and writing.
#! 
#! Meataxe64 matrices are only equal or comparable with <Ref Oper="&lt;"/> if they
#! are defined over the same field and of the same shape. The ordering is a
#! linear ordering, but is not otherwise defined.
#! 
#! 
