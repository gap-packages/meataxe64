/*
         gauss.h  -   Low-level Gaussian functions
         =======      R. A. Parker 9.5.2019
*/

typedef struct
{
    const FIELD * f;
    uint64_t maxrows;
    uint64_t maxcols;
    int phase;
    uint64_t nor;
    uint64_t noc;
    uint64_t nobm;
    uint64_t nobt;
    uint64_t nobrealt;
    DSPACE * dsm;
    DSPACE * dst;
    Dfmt *mtx;
    Dfmt *trf;
    int16_t * piv;
    int16_t * sorted;
    uint64_t rank;
    Dfmt *batch;
    Dfmt *battrf;
    uint64_t batpiv;
    FELT det;
    int detsign;
}   GAUSS;


GAUSS * GaussCreate(const FIELD * f);
void GaussDestroy(GAUSS * gs);
void GaussStart(GAUSS * gs, uint64_t nor, uint64_t noc, Dfmt * m);
void GaussBack(GAUSS * gs);
int  GaussNewRow(GAUSS * gs, int row);
void GaussCleanC(GAUSS * gs, int row);
void GaussCleanA(GAUSS * gs, int row);
void GaussReduce(GAUSS * gs);
void GaussMultiplier(GAUSS * gs, Dfmt * mul);
void GaussCleaner(GAUSS * gs, Dfmt * cln);
void GaussRemnant(GAUSS * gs, Dfmt * rem);

uint64_t BCEch(GAUSS *gs, DSPACE * ds, Dfmt *a, 
               uint64_t *rs, uint64_t *cs, FELT * det,
               Dfmt *m, Dfmt *c, Dfmt *r, uint64_t nor, int flags);

/* end of gauss.h */
