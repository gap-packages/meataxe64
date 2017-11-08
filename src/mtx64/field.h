/*
         field.h  -   Field Routines Header
         =======      R. A. Parker  mtx64t version 30.6.2015
*/

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long uint64;

extern uint8 * AlignMalloc(size_t bytes);
extern void AlignFree(uint8 * ptr);

#define FIELDLEN 4194304
#define NOMUL 16

typedef uint64 FELT;
typedef uint8  Dfmt;

typedef struct
{
    uint64_t fdef;            // field order
    uint64_t charc;           // field characteristic
    uint64_t pow;             // field degree fdef = charc^pow

/* ================================ */

    uint64 conp;
    FELT   cp0;
    FELT   cp1;
    uint64 qstep;  /* fdef/charc */
    uint64 bar41;  /* spaclev power */
    uint64_t bar48;  /* 2^48/charc rounded up */
    uint64 sqpower;
    uint64 sqpower2;


    uint64 entbyte;
    uint64 bytesper;
    uint32 dbytevals;
    uint32 qminus1;
    uint32 p32;
/* 0-64bit 1-32bit 2-16bit 3-8bit 4-2inbyte 5-mod5 6-GF4 7-mod3 8-mod2 */
     int   paktyp;  /* 0-8 indicating how FELT packed in Dfmt */
     int   ppaktyp;  /* similar, but for ground field */
    uint64 pentbyte;
    uint64 pbytesper;
    uint16 spaczero;
    uint16 spacneg;
    uint64_t clpm[3];

     int   addtyp;
     int   multyp;
     int   madtyp;
     int   paddtyp;
     int   pmultyp;
     int   pmadtyp;
     int   spaclev;
     int   bong;

     int   Tlog16;
     int   Talog16;
     int   Tspac16;
     int   Tsqid16;
     int   Tred16;
     int   Tzech16;
     int   Trdlg16;

     int   Tlate8;
     int   Tearly8;
     int   Tinv8;
     int   Tmul8;
     int   Tadd8;
     int   Tsub8;

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
    uint64_t parms[9];    // AS-code parms
    uint8_t  prog[32];    // AS-code addition chain

/*   HPMI public variables   */

    uint64_t alcovebytes; // bytes in an alcove in Afmt inc. skip/term
    uint64_t cauldron;    // in columns
    uint64_t cfmtcauld;   // bytes in a cauldron in Cfmt
    uint64_t dfmtcauld;   // bytes in a cauldron in Dfmt
    uint64_t alcove;      // cols/rows in an alcove
    uint64_t recbox;      // target rows of C done by a brick
    uint64_t bbrickbytes; // number of bytes for brick in Bfmt
    uint64_t bwasize;     // size of the brick work area

/* other non-FIELD variables  */

     int   linfscheme;
     int   pextype;
     int   pastype;

     int   Tlfx;
     int   Tlfa;

     int   Ttra;



}   FIELD;

typedef struct
{
  const FIELD * f;  /* the field in use */
  uint64 noc;       /* Dimension of the space. */
  uint64 nob;       /* Number of bytes for a row of Dfmt in memory. */
  int ground;       /* 0 = extension, 1=ground */
}   DSPACE;

/* First to set the field */
/* FieldSet is in slab.h  */ 

extern void FieldASet (uint64_t fdef, FIELD * f);
extern int  FieldASet1(uint64_t fdef, FIELD * f, int flags);


/* four-functions field element stuff */

extern FELT FieldAdd(const FIELD * f, FELT a, FELT b);
extern FELT FieldNeg(const FIELD * f, FELT a);
extern FELT FieldSub(const FIELD * f, FELT a, FELT b);
extern FELT FieldMul(const FIELD * f, FELT a, FELT b);
extern FELT FieldInv(const FIELD * f, FELT a);
extern FELT FieldDiv(const FIELD * f, FELT a, FELT b);

/* The D-format things in field.c  */

extern void DSSet(const FIELD * f, uint64 noc, DSPACE * ds);
extern void PSSet(const FIELD * f, uint64 noc, DSPACE * ds);
extern FELT DUnpak(const DSPACE * ds, uint64 col, const Dfmt * d);
extern void DPak(const DSPACE * ds, uint64 col, Dfmt * d, FELT a);
extern void DAdd(const DSPACE * ds, uint64 nor,
                  const Dfmt * d1, const Dfmt * d2, Dfmt * d);
extern void DSub(const DSPACE * ds, uint64 nor,
                  const Dfmt * d1, const Dfmt * d2, Dfmt * d);
extern void DSMad(const DSPACE * ds, FELT scalar, uint64 nor,
                                     const Dfmt * d1, Dfmt * d2);
extern void DSMul(const DSPACE * ds, FELT a, uint64 nor, Dfmt * d);
#define ZEROROW 0xffffffffffffffffull
extern uint64 DNzl(const DSPACE * ds, const Dfmt * d);
extern void DCpy(const DSPACE * ds, const Dfmt * d1, uint64 nor, Dfmt * d2);
extern void DCut(const DSPACE * ms, uint64 nor, uint64 col,
                  const Dfmt * m, const DSPACE * cbs, Dfmt *cb);
extern void DPaste(const DSPACE * cbs, const Dfmt * cb, uint64 nor, 
                   uint64 col, const DSPACE * ms, Dfmt * m);
extern Dfmt * DPAdv(const DSPACE * ds, uint64 nor, const Dfmt * d);
extern Dfmt * DPInc(const DSPACE * ds, const Dfmt * d);
extern void PExtract(const DSPACE * ds, const Dfmt *mq,
                   Dfmt *mp, uint64_t nor, uint64_t psiz); 
extern void PAssemble(const DSPACE * ds, const Dfmt *mp,
                   Dfmt *mq, uint64_t nor, uint64_t psiz); 

/* end of field.h  */
