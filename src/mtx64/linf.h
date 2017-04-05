/*
         linf.h  -   Linear forms module header
         ======      R. A. Parker     21.12.2016
*/

void linftab(FIELD * f);

void LLMul(const FIELD *f,const Dfmt *a,const Dfmt *b,Dfmt *c,
           uint64_t nora,uint64_t noca, uint64_t nocb);
void CExtract(DSPACE * dq, const Dfmt *mq, uint64_t nor,
                 Dfmt *mp);

/* end of linf.h  */
