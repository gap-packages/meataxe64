/*
         linf.h  -   Linear forms module header
         ======      R. A. Parker     21.12.2016
*/

void linftab(FIELD * f);

void LLMul(DSPACE *dsa, DSPACE *dsbc, uint64_t nora, 
           const Dfmt *a,uint64_t astride, const Dfmt *b, uint64_t bstride,
           Dfmt *c);

/* end of linf.h  */
