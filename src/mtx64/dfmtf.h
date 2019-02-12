/*
         dfmtf.h  -   Dfmt Functions
         =======      R. A. Parker 19.2.2018
*/

uint64_t DfEch(DSPACE * ds, Dfmt *a, uint64_t *rs, uint64_t *cs, 
             FELT * det, Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor);
void DFMul(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c);
void DFMad(DSPACE *dsa, DSPACE * dsbc, uint64_t nora,
     const Dfmt * a, uint64_t astride, const Dfmt * b, uint64_t bstride,
           Dfmt * c, uint64_t cstride);

/* end of dfmtf.h */
