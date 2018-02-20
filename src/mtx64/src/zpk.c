/*
      zpk.c     meataxe-64 Peek at Matrix
      =====     R. A. Parker   13.10.2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "slab.h"
#include "io.h"

// uncomment the following line for internal data also
#define DEBUG 1
 
int main(int argc,  char **argv)
{
    uint64_t hdr[5];
    uint64_t fdef,nor,noc;
    uint64_t x,y;
    uint8_t *Thpa,*f8;
    int vals,i;
    FIELD * f;
    DSPACE ds;
    int hpmi;

    if(argc!=2)
    {
        printf("usage zpk <filename>\n");
        exit(1);
    }
    printf("Peek at file %s\n",argv[1]);
    EPeek(argv[1],hdr);
#ifdef DEBUG
    printf("Header %ld %ld %ld %ld %ld\n",
              hdr[0],hdr[1],hdr[2],hdr[3],hdr[4]);
#endif
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    if(hdr[0]==1)
    {
printf("Type 1 - flat matrix, field order %ld, %ld rows, %ld columns\n",
                                        fdef,  nor,     noc);

        f = malloc(FIELDLEN);
        FieldSet(fdef,f);
        f8=(uint8_t *)f;
printf("    Characteristic %ld, degree %ld\n",
                        f->charc,    f->pow);
printf("    mact string %c %c\n",f->mact[0],f->mact[1]);
#ifdef DEBUG
printf("   Field %ld entries per %ld bytes, Prime field %ld per %ld bytes\n",
               f->entbyte,    f->bytesper,        f->pentbyte, f->pbytesper);
printf("   paktyp=%d ppaktyp=%d\n",f->paktyp, f->ppaktyp);
printf(
"    addtyp=%d, multyp=%d, madtyp=%d, paddtyp=%d, pmultyp=%d, pmadtyp=%d\n",
  f->addtyp, f->multyp, f->madtyp, f->paddtyp, f->pmultyp, f->pmadtyp);
if(f->madtyp==11)
printf("    clpm-e=%016lx   clpm-p=%016lx    clpm-s=%ld\n",
           f->clpm[0], f->clpm[1], f->clpm[2]);
#endif
printf("    %d bytes used in FIELD structure\n",f->hwm);
//  Conway Polynomial
printf("    (Conway) polynomial used X");
        if(f->pow==1) printf(" = %ld\n",f->conp);
        else
        {
            printf("^%ld = %ld",f->pow,f->conp%f->charc);
            x=f->conp/f->charc;
            for(i=1;i<f->pow;i++)
            {
                y=x%f->charc;
                x=x/f->charc;
                if(y==0) continue;
                if(y!=1) printf(" + %ld.X",y);
                 else    printf(" + X");
                if(i!=1) printf("^%d",i);
            }
            printf("\n");
        }
        hpmi=0;
        if(f->pow==1) hpmi=1;
        if( (f->pow!=1)&&(f->linfscheme!=0) )hpmi=1;
        if(f->cauldron==0) hpmi=0;
        if(hpmi==1)
        {
printf("    HPMI available using A=%ld B=%ld C=%ld S=%ld G=%ld M=%ld\n",
            f->AfmtMagic, f->BfmtMagic, f->CfmtMagic,
            f->SeedMagic, f->GreaseMagic, f->BwaMagic);
            if(f->pow==1)
printf("    Direct use of cauldron %ld alcove %ld\n",f->cauldron, f->alcove);
                else
printf("    Linear forms use of cauldron %ld alcove %ld\n",
                                   f->cauldron, f->alcove);
#ifdef DEBUG
            printf("    czer=%0lx  bzer=%0lx, bfmtcauld=%ld\n",
                     f->czer,   f->bzer,   f->bfmtcauld);
            if(f->AfmtMagic==2)
            {
                vals=f->charc;
                if(f->charc<=13) vals=vals*vals;
                if(f->charc==5) vals=125;
                printf("    parms = charc %016lx,  shift S=%ld,  mask %016lx\n",
                                     f->parms[0],f->parms[1],f->parms[2]);
                printf("            2^S-p %016lx,  rowlen %ld,  slots %ld\n",
                                     f->parms[3],f->parms[4],f->parms[5]);
                printf("            slices %ld,  2^S mod p %016lx,  bias %016lx\n",
                                     f->parms[6],f->parms[7],f->parms[8]);
                Thpa=f8+f->Thpa;
                printf("    Afmt table");
                for(i=0;i<vals;i++)
                {
                    if(i%20==0) printf("\n  %3d ..",i);
                    printf(" %02x",Thpa[i]);
// This check should be done outside DEBUG
                    if( (Thpa[i]==0)&&(i!=0) )
                        printf("**** Error AS-table cannot make %d\n",i);
                }
                printf("\n");
            }
#endif
            printf("    alcovebytes=%ld recbox=%ld bbrickbytes=%ld bwasize=%ld\n",
                     f->alcovebytes, f->recbox, f->bbrickbytes, f->bwasize);
            printf("    cfmtcauld=%ld, dfmtcauld=%ld\n",
                     f->cfmtcauld,  f->dfmtcauld);
        }
        if(hpmi==0)
            printf("    HPMI not available\n");
        DSSet(f,noc,&ds);
printf("    Row occupies %ld bytes\n",ds.nob);
    }
    if(hdr[0]==2)
    {
printf("Type 2 - bitstring, %ld bits, of which %ld set\n",
                            nor,               noc);
    }
    if(hdr[0]==3)
    {
printf("Type 3 - map/permutation, domain %ld, image %ld\n",
                                          nor,      noc);
    }
    return 0;
}

/******  end of zpk.c    ******/
