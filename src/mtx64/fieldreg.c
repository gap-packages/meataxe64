/*    Meataxe-64         fieldreg.c         */
/*    ==========         ==========         */
/*   regression program for the field module*/

/*    R. A. Parker      19.05.13        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "field.h"

// commment this line out for terse output
// #define PROGRESS 1

// comment this line out for a long test
#define QUICKFLAG 1

int errors;
uint64 fdef;

uint64 fields[] = {
  2,
  3,
  4,
  5,
  7,
  8,
  9,
  11,
  13,
  16,
  17,
  19,
  23,
  25,
  27,
  31,
  32,
  37,
  47,
  49,
  53,
  64,
  81,
  121,
  125,
  127,
  128,
  131,
  169,
  243,
  256,
  263,
  289,
  307,
  343,
  361,
  443,
  509,
  512,
  521,
  529,
  625,
  729,
  841,
  961,
  1021,
  1024,
  1331,
  1369,
  1681,
  2039,
  2048,
  2187,
  2197,
  2401,
  3125,
  3331,
  3721,
  4096,
  4913,
  5041,
  6007,
  6561,
  6859,
  6889,
  8192,
  10201,
  11881,
  12007,
  12167,
  14641,
  15625,
  16129,
  16384,
  16807,
  17161,
  19683,
  24389,
  26017,
  28561,
  29791,
  32768,
  39019,
  50653,
  59049,
  65521,
  65536,
  69169,
  78125,
  79507,
  83521,
  94249,
  111119,
  117649,
  130321,
  131072,
  144013,
  161051,
  177147,
  196249,
  259081,
  262144,
  271441,
  279841,
  299011,
  300763,
  371293,
  389017,
  390625,
  524288,
  531441,
  600011,
  707281,
  704969,
  823543,
  923521,
  1042441,
  1048576,
  1092727,
  1111151,
  1419857,
  1442897,
  1594323,
  1771561,
  1874161,
  1953125,
  2048383,
  2097152,
  2476099,
  2571353,
  4157521,
  4194304,
  4782969,
  4826809,
  4879681,
  5555567,
  5764801,
  6436343,
  8388608,
  9765625,
  11095561,
  11111117,
  14348907,
  16777216,
  18191447,
  19487171,
  20511149,
  22222223,
  24137569,
  25411681,
  28629151,
  28934443,
  33554432,
  36084049,
  38950081,
  40353607,
  43046721,
  47045881,
  48828125,
  62748517,
  67108864,
  86938307,
  129140163,
  131079601,
  131872229,
  134217728,
  141420761,
  144168049,
  148035889,
  214358881,
  222222227,
  244140625,
  260144641,
  268435456,
  282475249,
  387420489,
  410338673,
  418195493,
  536870912,
  594823321,
  676884289,
  815730721,
  887503681,
  893871739,
  1064332261,
  1073741824,
  1162261467,
  1220703125,
  1522482361,
  1977326743,
  2147483648U,
  2357947691U,
  3404825447U,
  3486784401U,
  3969126001U, 
  4293001441U,
  693386350578511591ULL,
  9223372036854775783ULL,
  18446744073709551557ULL,
  18446744030759878681ULL,
  18446598518342697919ULL,
  18429861372428076481ULL,
  18413785235633886649ULL,
  18142539992483535721ULL,
};

unsigned int total_fields = sizeof(fields)/sizeof(fdef);

static void ckint64(int e, uint64 was, uint64 shouldbe)
{
    if(was==shouldbe) return;
    printf("Error %d, field %lu, was %lx, should be %lx\n",
                  e,fdef,was,shouldbe);
    errors++;
}

static void ckint(int e, int was, int shouldbe)
{
    if(was==shouldbe) return;
    printf("Error %d, field %lu, was %d, should be %d\n",
                e,fdef,was,shouldbe);
    errors++;
}

int main(int argc, const char *const argv[])
{
    int res;
    FELT x1,x2,x3,x4,x5;
    FIELD *field;
    uint64 fdefsing;
    int quick_flag;
    uint64 k;

    fdefsing=0;
    if (argc > 2) {
      fprintf(stderr, "At most 1 argument may be specified\n");
      fprintf(stderr, "%s [-f|<field size>]\n", argv[0]);
      return 1;
    }
    /* First check for testing a specific field */
    if (argc==2)
    {
        fdef=strtoul(argv[1], NULL, 0);
        field = (FIELD *)malloc(FIELDLEN);
        res=FieldASet1(fdef,field,8);
        if(res<1)
        {
            printf("Field not viable . . . result %d\n",res);
            return 0;
        }
        printf(" fdef  %lu\n",field->fdef);
        printf("charc  %lu\n",field->charc);
        printf(" pow   %lu\n"  ,field->pow);
        printf(" conp  %lu\n",field->conp);
        printf("qstep  %lu\n",field->qstep);
        printf("paktyp %d\n"  ,field->paktyp);
        printf("entbyte %lu\n",field->entbyte);
        printf("bytesper %lu\n",field->bytesper);
        printf("dbytevals %d\n",field->dbytevals);
        printf("qminus1 %d\n",field->qminus1);
        printf("addtyp %d\n",field->addtyp);
        printf("multyp %d\n",field->multyp);
        printf("madtyp %d\n",field->madtyp);
        printf(" p32 %d\n",field->p32);
        fdefsing=fdef;
    }
    quick_flag = QUICKFLAG;
    errors=0;
    fdef=17;
    field = (FIELD *)malloc(FIELDLEN);
    fdef=0;
    res=FieldASet1(fdef,field,8);
    ckint(2,res,-1);
    fdef=1;
    res=FieldASet1(fdef,field,8);
    ckint(3,res,-1);
    free(field);

/* first main cycle - try all field orders up to a million  */
/* Or use those from the list */
    for(k=2;k<total_fields+2;k++)
    {
        unsigned int i, j;
        if(fdefsing==0) fdef = fields[k - 2];
        else fdef=fdefsing;
        if( (fdefsing!=0) && (k>2) ) break;
#ifdef PROGRESS
        printf("Handling field %lu\n", fdef);
#endif
        field = (FIELD *)malloc(FIELDLEN);
        res=FieldASet1(fdef,field,8);
        if(res<1)
        {
            printf("Field Set for %lu failed result %d\n",fdef,res);
            errors++;
            continue;
        }
/* otherwise take a look to see if field arithmetic works  */
        x1=0;
        x2=1;
        x3=1;
/* initial values of x1,x2,x3.  99 others tried too */
        for(i=0;i<100;i++) {
/* commutative law of addition  */  
          x4=FieldAdd(field,x1,x2);
          x5=FieldAdd(field,x2,x1);
          ckint64(1000+i,x4,x5);
/* x1+x2-x2 == x1  */
          x4=FieldSub(field,x4,x2);
          ckint64(1100+i,x1,x4);
/* commutative law for multiplication  */
          x4=FieldMul(field,x1,x2);
          x5=FieldMul(field,x2,x1);
          ckint64(1200+i,x4,x5);
          if(x1!=0) {
/* x1*x2/x1 == x2  */
            x5=FieldDiv(field,x4,x1);
            ckint64(1300+i,x5,x2);
/* x1.(x1^-1) == 1  */
            x4=FieldInv(field,x1);
            x5=FieldMul(field,x4,x1);
            ckint64(1400+i,x5,1);
          }
/* x1 + (-x1) == 0  */
          x4=FieldNeg(field,x1);
          x5=FieldAdd(field,x1,x4);
          ckint64(1500+i,0,x5);
/* distributive law of multiplication  */
          x4=FieldAdd(field,FieldMul(field,x1,x2),FieldMul(field,x1,x3));
          x5=FieldMul(field,x1,FieldAdd(field,x2,x3));
          ckint64(1600+i,x4,x5);
/* now update the test field elements */
          x1+=1+i;
          while(x1>=fdef)x1-=fdef;
          x2+=3+i;
          if(i%5==3)x2+=4;
          while(x2>=fdef)x2-=fdef;
          x3+=5+i;
          if(i%7==4) x3+=5;
          while(x3>=fdef)x3-=fdef;
        }
        /* Now try some vector addition using DAdd */
        /* We'll try all vector lengths up to 128,
         * thus guaranteeing to get at least 2 uint64s involved
         * We try inserting values, adding, then checking that the add
         * gives what the field arithmetic says
         */
        for (i = 1; i <= 128; i++) {  
          /* Create a DSPACE with i columns */
          DSPACE ds;
          FELT a, b, c;
          Dfmt *d1, *d2, *d3;
          uint64 bytes;
          if( (quick_flag==1) && 
              ( i!=1) &&
              ( i!=2) &&
              ( i!=13) &&
              ( i!=31) &&
              ( i!=32) &&
              ( i!=63) &&
              ( i!=69) &&
              ( i!=128) ) continue;
          DSSet(field, i, &ds);
          bytes = ds.nob;
          d1 = malloc(bytes * 2); /* We provide 2 rows for cut and paste */
          d2 = malloc(bytes * 2);
          /* First test, clear out and add, all should be zero */
          memset(d1, 0, bytes);
          memset(d2, 0, bytes);
          DAdd(&ds, 1, d1, d2, d2);
          res = memcmp(d1, d2, bytes);
          ckint(199, res, 0);
          /* Insert values into vectors */
          for (a = 0, j = 0; j < i; a++, j++) {
            a %= fdef;
            DPak(&ds, j, d1, a);
          }
          /* Simple add, check d1 ends up same as d2 */
          DAdd(&ds, 1, d1, d2, d2);
          res = memcmp(d1, d2, bytes);
          ckint(200+fdef, res, 0);
          /* Now check that DUnpak gets the right value out of d2 */
          for (a = 0, j = 0; j < i; a++, j++) {
            a %= fdef;
            b = DUnpak(&ds, j, d2);
            ckint64(2000+fdef, b, a);
            assert(a == b);
          }
          /* Now try non-zero values in d2, offsetting */
          for (j = 0; j < i; j++) {
            /* j is the offest */
            unsigned int k;
            memset(d2, 0, bytes);
            for (b = j % fdef, k = 0; k < i; b++, k++) {
              b %= fdef;
              DPak(&ds, k, d2, b);
            }
            DAdd(&ds, 1, d1, d2, d2);
            /* Now check that all the right results have been made */
            for (k = 0, a = 0, b = j %fdef; k < i; k++, a++, b++) {
              FELT d;
              a %= fdef;
              b %= fdef;
              c = FieldAdd(field, a, b); /* What we should get */
              d = DUnpak(&ds, k, d2);
              ckint64(20000+fdef, c, d);
              assert(c == d);
            }
          }
          /* Now try d1 *= scalar, for a reasonable number of scalars */
          /* Now try a reasonable number of scalars */
          for (b = 0; b < fdef && b < 100; b++) {
            /* Set up d1 */
            memset(d1, 0, bytes);
            for (a = 0, j = 0; j < i; j++, a++) {
              a %= fdef;
              DPak(&ds, j, d1, a);
            }
            DSMul(&ds, b, 1, d1);
            /* Check the values in d1 */
            for (a = 0, j = 0; j < i; j++, a++) {
              FELT d;
              a %= fdef;
              c = FieldMul(field, a, b);
              d = DUnpak(&ds, j, d1);
              ckint64(40000, c, d);
              assert(c == d);
            }
          }
          /* Now try d2 += d1, with d2 starting zero */
          memset(d1, 0, bytes);
          /* Set up d1 */
          for (a = 0, j = 0; j < i; j++, a++) {
            a %= fdef;
            DPak(&ds, j, d1, a);
          }
          /* Now try a reasonable number of scalars */
          for (b = 0; b < fdef && b < 100; b++) {
            FELT e;
            memset(d2, 0, bytes);
            DSMad(&ds, b, 1, d1, d2);
            /* Check the values in d2 */
            for (a = 0, j = 0; j < i; j++, a++) {
              FELT d;
              a %= fdef;
              c = FieldMul(field, a, b);
              d = DUnpak(&ds, j, d2);
              ckint64(40000, c, d);
              assert(c == d);
            }
            /* Now a non zero d2 */
            memset(d2, 0, bytes);
            for (a = fdef / 2, j = 0; j < i; j++, a++) {
              /*printf("j = %u, a = %llu, fdef = %llu, a mod fdef = %llu\n", j, a, fdef, a % fdef);*/
              a %= fdef;
              DPak(&ds, j, d2, a);
            }
            DSMad(&ds, b, 1, d1, d2);
            /* Check the values in d2 */
            for (a = 0, j = 0, e = fdef / 2; j < i; j++, a++, e++) {
              FELT d;
              a %= fdef;
              c = FieldMul(field, a, b);
              c = FieldAdd(field, c, e % fdef);
              d = DUnpak(&ds, j, d2);
              ckint64(40000, c, d);
              assert(c == d);
            }
          }
          /* Now test DNzl */
          for (j = 0; j <= i; j++) {
            /* Testing first non zero element at column j
             * j == i means no element at all, ie all zero
             */
            uint64 l;
            memset(d1, 0, bytes);
            if (j < i) {
              unsigned int k;
              /* Try each field element as the first, up to no more than 100 */
              for (a = 0; a < fdef && a < 100; a ++) {
                a %= fdef;
                DPak(&ds, j, d1, a);
              }
              b = a;
              /* Pack rest of columns after j */
              for (k = j + 1; k < i; k++, b++) {
                b %= fdef;
                DPak(&ds, k, d1, b);
              }
              l = DNzl(&ds, d1);
              ckint64(60000 + fdef, l, j);
              assert(l == j);
            } else {
              l = DNzl(&ds, d1);
              ckint64(80000 + fdef, l, ZEROROW);
              assert(l == ZEROROW);
            }
          }
          /* Test DPAdv and DPInc */
          for (j = 0; j < 128; j++) {
            d3 = DPAdv(&ds, j, d1);
            assert(d3 == (Dfmt *)(((char *)d1) + j * bytes));
          }
          d3 = DPInc(&ds, d1);
          assert(d3 == (Dfmt *)(((char *)d1) + bytes));
          /* Test DCut */
          /* Plan, for each possible starting column, for each possible length
           * cut d1 into d2 then check the result.
           * We only test for two rows, although DCut can do any matrix shape
           */
          /* First set up d1 */
          memset(d1, 0, bytes * 2);
          for (j = 0, a = 0; j < i; j++, a++) {
            Dfmt *src_row = DPInc(&ds, d1);
            b = a + 1;
            a %= fdef;
            b %= fdef;
            DPak(&ds, j, d1, a);
            DPak(&ds, j, src_row, b);
          }
          free(d2); /* Standard d2 isn't big enough in general */
          d2 = malloc(bytes * 128 * 2); /* Allow up to 128 columns */
          /* Do this for all widths up to 128 */
          for (j = 0; j < i; j++) {
            /* j is starting column */
            unsigned int k;
            for (k = 1; k <= 128; k++) {
              /* k is copy length, which may exceed available input */
              unsigned int l;
              unsigned int kplus;
              DSPACE dscb;
              DSSet(field, k, &dscb);
              memset(d2, 0, bytes * 128 * 2);
              kplus=k;
              if(field->paktyp==8) kplus=((k+7)/8)*8;
              if(field->paktyp==7) kplus=((k+4)/5)*5;
              if(field->paktyp==6) kplus=((k+3)/4)*4;
              if(field->paktyp==5) kplus=((k+2)/3)*3;
              if(field->paktyp==4) kplus=((k+1)/2)*2;
              DCut(&ds, 2, j, d1, &dscb, d2); /* Cut 2 rows */
              /* Now check d2 has expected content */
              for (l = 0, a = j; l < kplus; l++, a++) {
                Dfmt *dest_row = DPInc(&dscb, d2);
                FELT c = (a + 1) % fdef;
                FELT d;
                (void)c;
                a %= fdef;
                b = DUnpak(&dscb, l, d2);
                d = DUnpak(&dscb, l, dest_row);
                /* If j + l < i then value from d1 */
                /* Otherwise value should be zero */
                if ( (j + l < i) && ( l < k) ) {
                  ckint64(100000, b, a);
                  ckint64(100000, d, c);
                } else {
                  ckint64(110000, b, 0);
                  ckint64(110000, d, 0);
                }
              }
            }
          }
          /* Test DPaste */
          /* Plan, for each possible starting column, for each possible length
           * paste d1 into d2 then check the result.
           * We only test for two rows, although DPaste can do any matrix shape
           */
          /* Now paste k columns of d2 into d1 starting at column j */
          for (j = 0; j < i; j++) {
            /* j is starting column */
            unsigned int k;
            Dfmt *src_row = DPInc(&ds, d1);
            /* Do this for all widths up to 128 */
            for (k = 1; k <= 128; k++) {
              /* k is copy length, which may exceed available input/output */
              unsigned int l;
              DSPACE dscb; /* The clipboard */
              Dfmt *dest_row;
              DSSet(field, k, &dscb);
              dest_row = DPInc(&dscb, d2);
              /* Paste from cb, d2, 2 rows, starting at column j, to target */
              memset(d1, 0, bytes * 2);
              for (l = 0, a = 0; l < i; l++, a++) {
                FELT b = a + 1;
                a %= fdef;
                DPak(&ds, l, d1, a);
                DPak(&ds, l, src_row, b % fdef);
              }
              /* Need to initialise all of d2, as when it gets longer
               * what was row 1 becomes part of row 0
               * but has the wrong values
               */
              /* Clear d2 */
              memset(d2, 0, bytes * 128 * 2);
              for (l = 0; l < k; l++) {
                DPak(&dscb, l, d2, l % fdef);
                DPak(&dscb, l, dest_row, (l+1) % fdef);
              }
              /* Paste from cb, d2, 2 rows, starting at column j, to target */
#if 0
              printf("Pasting %d cols of clipboard from col %d of matrix, into matrix of %d columns\n", k, j, i);
              printf("Pre paste matrix = 0x%llx, clipboard = 0x%llx\n", *(uint64 *)d1, *(uint64 *)d2);
#endif
              DPaste(&dscb, d2, 2, j, &ds, d1);
              /* Now check d1 has expected content */
              /* Up to j it should be as before */
              for (l = 0, a = 0; l < j; l++, a++) {
                b = DUnpak(&ds, l, d1);
                ckint64(100000, b, a % fdef);
                if (a % fdef != b) {
                  printf ("Failure row 0, column = %d, matrix = 0x%lx\n", l, *(uint64 *)d1);
                }
                assert(a % fdef == b);
                b = DUnpak(&ds, l, src_row);
                ckint64(100000, b, (a+1) % fdef);
                if ((a+1) % fdef != b) {
                  printf ("Failure row 1, column = %d, matrix = 0x%lx\n", l, *(uint64 *)d1);
                }
                assert((a+1) % fdef == b);
              }
              /* After that, for up to min(k,i-j), it should be as d2 */
              for (l = 0, a = 0; l < k && l+j < i; l++, a++) {
                b = DUnpak(&ds, l+j, d1);
                ckint64(100000, b, a % fdef);
                if (a % fdef != b) {
                  printf ("Failure row 0, column = %d, matrix = 0x%lx\n", l+j, *(uint64 *)d1);
                }
                assert(a % fdef == b);
                b = DUnpak(&ds, l+j, src_row);
                ckint64(100000, b, (a+1) % fdef);
                if ((a+1) % fdef != b) {
                  printf ("Failure row 1, column = %d, matrix = 0x%lx\n", l+j, *(uint64 *)d1);
                }
                assert((a+1) % fdef == b);
              }
              /* After that, it should be as d1 again */
              for (l = j+k, a = j+k; l < i; l++, a++) {
                b = DUnpak(&ds, l, d1);
                ckint64(100000, b, a % fdef);
                if (a % fdef != b) {
                  printf ("Failure column = %d, matrix = 0x%lx\n", l, *(uint64 *)d1);
                }
                assert(a % fdef == b);
                b = DUnpak(&ds, l, src_row);
                ckint64(100000, b, (a+1) % fdef);
                if ((a+1) % fdef != b) {
                  printf ("Failure column = %d, matrix = 0x%lx\n", l, *(uint64 *)d1);
                }
                assert((a+1) % fdef == b);
              }
            }
          }
          free(d2);
          free(d1);
          /* Test DCpy */
          /* We'll only do this for a few fields, it's not worth more */
          if (fdef > 256) {
            continue;
          }
          /* Try all up to 64 rows. Needs a different source and dest */
          for (j = 1; j <= 64; j++) {
            unsigned int k, l;
            Dfmt *src, *dest;
            DSSet(field, i, &ds);
            bytes = ds.nob;
            src = malloc(bytes * j);
            dest = malloc(bytes * j);
            /* Initialise dest to zero */
            memset(dest, 0, bytes * j);
            /* Initialise src to zero, even thopugh we're going to write it all */
            memset(src, 0, bytes * j);
            /* Initialise src */
            for (k = 0; k < j; k++) {
              /* Rows */
              Dfmt *src_row = DPAdv(&ds, k, src);
              for (l = 0; l < i; l++) {
                /* Columns */
                FELT a = (k * l) % fdef;
                DPak(&ds, l, src_row, a);
              }
            }
            /* Copy j rows from src to dest */
            DCpy(&ds, src, j, dest);
            /* Now check the answer */
            for (k = 0; k < j; k++) {
              /* Rows */
              Dfmt *dest_row = DPAdv(&ds, k, dest);
              for (l = 0; l < i; l++) {
                /* Columns */
                FELT b = (k * l) % fdef;
                FELT a = DUnpak(&ds, l, dest_row);
                ckint64(110000, a, b);
                if (a != b) {
                  printf("row %d, col %d\n", k, l);
                }
                assert(a == b);
              }
            }
            free(src);
            free(dest);
          }
        }
        free(field);
    }

    printf("Field regression completed - %d errors\n",errors);
    return 0;
}

/*    end of fieldreg.c       */
