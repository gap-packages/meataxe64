/*
         io.h  -   Input/Output Routines Header
         ====      R. A. Parker      21.2.2017
*/


typedef struct
{
    uint64_t P1;
    uint64_t P2;
    uint64_t P3;
    uint64_t P4;
    uint64_t Q1;
    uint64_t Q2;
    uint64_t Q3;
    uint64_t Q4;
    uint8_t * bk;    //64 bytes long
    FILE * f;
    uint64_t bck[2];
    int null;
    char * fn;     // filename
    int nex;
}   EFIL;

extern void EPeek(const char * fname, uint64_t * header);
extern EFIL * ERHdr(const char * fname, uint64_t * header);
extern EFIL * EWHdr(const char * fname, const uint64_t * header);
extern void ERData(EFIL * e, size_t bytes, uint8_t * d);
extern void EWData(EFIL * e, size_t bytes, const uint8_t * d);
extern void ERClose(EFIL * e);
extern int  ERClose1(EFIL * e, int flag);
extern void EWClose(EFIL * e);
extern int  EWClose1(EFIL * e, int flag);
extern void LogCmd(int argc, char ** argv);
extern void LogString(int type, const char *string);
extern void CLogCmd(int argc, const char *argv[]);

/* end of io.h  */
