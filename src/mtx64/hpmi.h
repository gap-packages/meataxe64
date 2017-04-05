/*
         hpmi.h  -   High Performance Meataxe Interface Header
         ======      R. A. Parker     5.7.2015
*/

typedef struct
{
    const FIELD * f;
// The part populated by the slab module
    uint64 nora;
    uint64 noca;
    uint64 noba;
    uint64 nocb;
    uint64 nobbc;       // Dfmt bytes per row
    uint64 nz0;         // rows A/C
    uint64 nz1;         // alcoves per Afmt block
    uint64 bytesnz1;    // bytes for nz1 alcoves in Dfmt
    uint64 nz2;         // cauldrons
    uint64 nz3;         // Afmt blocks 
    uint64 nz4;         // nz0's in the whole matrix
// then the part populated by AllocWA
    uint64 alen;      // bytes per row of Afmt
    uint8 * a;        // pointer to the Afmt itself if bytes
    uint64 * a64;      // pointer to the Afmt itself if uint64_t
    uint64 * ix;      // pointer to the working index array 
    uint8 * bwa;       // Brick Work Area = L1 cache
// then the part populated by BSeed/DSeed
    int sparsity;     // 0 brick is zero   1 brick is nonzero
}   HPMI;

//  DtoB will be needed for other characteristics

extern void hpmitab(FIELD * f);
extern void AllocWA(HPMI * hp);
extern void FreeWA(HPMI * hp);
extern void DtoA(HPMI * hp, const Dfmt * a, uint64 z4, uint64 z3);
extern void DtoB(HPMI * hp, const Dfmt * a, uint8 * b);
extern void DtoC(HPMI * hp, const Dfmt * a, uint8 * c);
extern void CZer(HPMI * hp, uint8 * c);
extern void CtoD(HPMI * hp, uint8 * cc, Dfmt * c);
extern void BSeed(HPMI * hp, const uint8 * bb, uint64 z3, uint64 z2, uint64 z1);
extern void DSeed(HPMI * hp, const Dfmt * b, uint64 z3, uint64 z2, uint64 z1);
extern void BGrease(HPMI * hp);        // compute the other grease vectors
extern void BrickMad(HPMI * hp, uint8 * c, uint64 z1);       // do the multiplication

/* end of hpmi.h  */
