/*
         pmul.h  -   Slab-scale multiply Header
         ======      R. A. Parker 18.8.2017
*/

extern void PLMul(DSPACE *dsa, DSPACE *dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
     Dfmt * c);

/* end of pmul.h  */
