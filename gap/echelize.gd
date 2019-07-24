#! @Chapter Gaussian Elimination Functions
#! @Section Basic Concepts and Main Functions
#! All Gaussian elimination operations in Meataxe64 are based on the following definitions.
#! <Display> <![CDATA[\left(\begin{array}{cc} M & 0\\ K & 1\end{array}\right)
#! \rho A \gamma = \left(\begin{array}{cc} -1 & R \\ 0 &
#! 0\end{array}\right)]]></Display> where <M>A</M> is the input matrix,
#! and <M>M</M>,<M>K</M>, <M>\rho</M>, <M>\gamma</M> and <M>R</M>
#! correspond to various components of the output. Specifically <List>
#! <Mark><M>M</M></Mark> <Item> Is an invertible matrix called the multiplier</Item>
#! <Mark><M>K</M></Mark> <Item> Is a matrix called the cleaner</Item>
#! <Mark><M>\rho</M></Mark> <Item>is the matrix corresponding to the row
#! select bitstring (left multiplicatiob by this matrix shuffles the
#! pivot rows to the top, preserving the order within the sets of
#! pivot and non-pivot rows</Item>
#! <Mark><M>\gamma</M></Mark> <Item>is the matrix corresponding to the column
#! select bitstring (right multiplicatiob by this matrix shuffles the
#! pivot columns to the left, preserving the order within the sets of
#! pivot and non-pivot columns</Item>
#! <Mark><M>R</M></Mark> <Item> Is a matrix called the remnant</Item></List>
#! Not all of these are necessarily always computed. 
#! 
#!  @Arguments mat
#!  @Returns a record with components <C>cleaner</C>, <C>colSelect</C>,
#! <C>multiplier</C>, <C>rank</C>, <C>remnant</C> and <C>rowSelect</C>
#! containing appropriate parts of the results. This uses the
#! recursive single-threaded "slab" implementation in the C code of MeatAxe64
DeclareGlobalFunction("MTX64_Echelize");

#!  @Arguments mat, optrec
#!  @Returns a record with some or all of the components <C>cleaner</C>, <C>colSelect</C>,
#! <C>multiplier</C>, <C>rank</C>, <C>remnant</C> and <C>rowSelect</C>
#! <A>optrec</A> may have components <C>multNeeded</C>,
#! <C>cleanerNeeded</C>, <C>remnantNeeded</C> (all default to
#! <C>true</C>. Setting any of these to false means the corresponding
#! part of the output may not be computed, which can save time. An
#! additional optional component <C>failIfSingular</C> will cause the
#! algorithm to return <C>fail</C> as soon as it detects that
#! <A>mat</A> is singular.  <P/> This uses a &GAP; implementation of
#! the recursive echelonisation, but still falls back to the C code
#! for multiplication and in the base case.
DeclareGlobalFunction("MTX64_GAPEchelize");
#! 
