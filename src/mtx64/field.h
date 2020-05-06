// Copyright (C) Richard Parker   2018
// Meataxe64 Nikolaus version
// field.h header file for operations in finite fields

extern uint8_t * AlignMalloc(size_t bytes);
extern void AlignFree(uint8_t * ptr);

#define FIELDLEN 4194304
#define NOMUL 16

typedef uint64_t FELT;
typedef uint8_t  Dfmt;

typedef struct
{
    uint64_t fdef;            // field order
    uint64_t charc;           // field characteristic
    uint64_t pow;             // field degree fdef = charc^pow
    char     mact[8];
    uint64_t conp;
    FELT   cp0;
    FELT   cp1;
/* 0-64bit 1-32bit 2-16bit 3-8bit 4-2inbyte 5-mod5 6-GF4 7-mod3 8-mod2 */
     int     paktyp;  /* 0-8 indicating how FELT packed in Dfmt */
     int     ppaktyp;  /* similar, but for ground field */
    uint64_t entbyte;
    uint64_t bytesper;
    uint64_t pentbyte;
    uint64_t pbytesper;
    uint32_t dbytevals;
    uint8_t  add8[65536];
    uint8_t  sub8[65536];
    uint8_t  mul8[65536];
    uint8_t  inv8[256];
    uint16_t red16[131072];
    uint16_t log16[65536];
    uint16_t alog16[196608];
    uint16_t zech16[196608];
    uint8_t late8[1792];
    uint8_t early8[1792];
// the spaclev batch
     int   spaclev;
     int   digit;
     int   nodigits;
    uint64_t digit2;
    uint64_t bar41;  /* spaclev power */
    uint16_t spaczero;
    uint16_t spacneg;
    uint16_t spac16[16384];
    uint16_t sqid16[65536];
// the "types" batch.
     int   addtyp;
     int   multyp;
     int   madtyp;
     int   paddtyp;
     int   pmultyp;
     int   pmadtyp;

/* ================================ */



    uint64_t qstep;  /* fdef/charc */

    uint64_t bar48;  /* 2^48/charc rounded up */
    uint64_t barpar[8];   // Barrett parameters for PExtract


    uint32_t qminus1;
    uint32_t p32;

    uint64_t clpm[3];

     int   atatime;

     int   hwm;     // high water mark - any new tables start here.

/*   HPMI private variables  */

    uint64_t AfmtMagic;   // 
    uint64_t BfmtMagic;   // 
    uint64_t CfmtMagic;
    uint64_t SeedMagic;
    uint64_t GreaseMagic;
    uint64_t BwaMagic;
    uint64_t czer;        // 64-bit value for zero in Cfmt
    uint64_t bzer;        // 64-bit value to initialize the BWA
    uint64_t bfmtcauld;   // bytes in a cauldron in Bfmt
     int   Thpv;
     int   Thpa;
     int   Thpb;
     int   Thpc;
    uint64_t p90;         // 2^90 mod p (only 64-bit primes)
    uint64_t parms[9];    // AS-code parms
    uint8_t  prog[40];    // AS-code addition chain
    uint64_t redfreq;      // how oftem to reduce floats

/*   HPMI public variables   */

    uint64_t abase;       // bytes of Afmt besides alcovebytes
    uint64_t boxlet;      // number of rows of A converted to A format together
    uint64_t alcovebytes; // bytes in a boxlet x alcove in Afmt inc. skip/term
    uint64_t cauldron;    // in columns
    uint64_t cfmtcauld;   // bytes in a cauldron in Cfmt
    uint64_t dfmtcauld;   // bytes in a cauldron in Dfmt
    uint64_t alcove;      // cols/rows in an alcove
    uint64_t recbox;      // target rows of C done by a brick
    uint64_t bbrickbytes; // number of bytes for brick in Bfmt
    uint64_t bwasize;     // size of the brick work area

/* other non-FIELD variables  */

     uint64_t megabytes;

     int   linfscheme;
     int   nomatlinf;
     int   pextype;
     int   pastype;

     int   Tlfx;
     int   Tlfa;

     int   Ttra;

}   FIELD;


// ==============================================================

typedef struct
{
  const FIELD * f;  /* the field in use */
  uint64_t noc;       /* Dimension of the space. */
  uint64_t nob;       /* Number of bytes for noc columns in Dfmt */
  int ground;       /* 0 = extension, 1=ground */
}   DSPACE;

extern void DSSet(const FIELD * f, uint64_t noc, DSPACE * ds);
extern void PSSet(const FIELD * f, uint64_t noc, DSPACE * ds);

/* four-functions field element stuff */

extern FELT FieldAdd(const FIELD * f, FELT a, FELT b);
extern FELT FieldNeg(const FIELD * f, FELT a);
extern FELT FieldSub(const FIELD * f, FELT a, FELT b);
extern FELT FieldMul(const FIELD * f, FELT a, FELT b);
extern FELT FieldInv(const FIELD * f, FELT a);
extern FELT FieldDiv(const FIELD * f, FELT a, FELT b);

extern void PExtract(const DSPACE * ds, const Dfmt *mq,
                   Dfmt *mp, uint64_t nor, uint64_t psiz); 
extern void PAssemble(const DSPACE * ds, const Dfmt *mp,
                   Dfmt *mq, uint64_t nor, uint64_t psiz); 

// ==============================================================

/* First to set the field */
/* FieldSet is in slab.h  */ 

extern void FieldASet (uint64_t fdef, FIELD * f);
extern int  FieldASet1(uint64_t fdef, FIELD * f, int flags);



/* The D-format things in field.c  */


extern FELT DUnpak(const DSPACE * ds, uint64_t col, const Dfmt * d);
extern void DPak(const DSPACE * ds, uint64_t col, Dfmt * d, FELT a);
extern void DAdd(const DSPACE * ds, uint64_t nor,
                  const Dfmt * d1, const Dfmt * d2, Dfmt * d);
extern void TAdd(const DSPACE * ds, uint64_t nor, const Dfmt *a, uint64_t astride,
                  const Dfmt *b, uint64_t bstride, Dfmt *c, uint64_t cstride);
extern void DSub(const DSPACE * ds, uint64_t nor,
                  const Dfmt * d1, const Dfmt * d2, Dfmt * d);
extern void TSub(const DSPACE * ds, uint64_t nor, const Dfmt *a, uint64_t astride,
                  const Dfmt *b, uint64_t bstride, Dfmt *c, uint64_t cstride);
extern void DSMad(const DSPACE * ds, FELT scalar, uint64_t nor,
                                     const Dfmt * d1, Dfmt * d2);
extern void DSMul(const DSPACE * ds, FELT a, uint64_t nor, Dfmt * d);
#define ZEROROW 0xffffffffffffffffull
extern uint64_t DNzl(const DSPACE * ds, const Dfmt * d);
extern void DCpy(const DSPACE * ds, const Dfmt * d1, uint64_t nor, Dfmt * d2);
extern void DCut(const DSPACE * ms, uint64_t nor, uint64_t col,
                  const Dfmt * m, const DSPACE * cbs, Dfmt *cb);
extern void DPaste(const DSPACE * cbs, const Dfmt * cb, uint64_t nor, 
                   uint64_t col, const DSPACE * ms, Dfmt * m);
extern Dfmt * DPAdv(const DSPACE * ds, uint64_t nor, const Dfmt * d);
extern Dfmt * DPInc(const DSPACE * ds, const Dfmt * d);


/* end of field.h  */
