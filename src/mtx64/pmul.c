/*
         pmul.c  -   Ground field slab-scale multiply
         ======      R. A. Parker 3.9.2017
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "field.h"
#include "hpmi.h"
#include "pmul.h"

void PLMul(const FIELD * f, const Dfmt * a, const Dfmt * b,
          Dfmt * c, uint64_t nora, uint64_t noca, uint64_t nocb)
{
    uint64_t alca,cauls;         // shape of the Bfmt
    uint64_t rowstodo, rowsleft, colstodo, colsleft;
    uint64_t box,boxes;
    uint64_t * ix;
    DSPACE dsa,dsbc;
    const Dfmt *pta,*ptb;
    Dfmt *ptc;
    Afmt *aa;
    Bfmt *bb;
    Cfmt *cc;
    uint8_t * bwa;
    uint64_t i,j,k;
    FELT e;

/* set up spaces */
    PSSet(f,noca,&dsa);
    PSSet(f,nocb,&dsbc);

/* check out the *do* *nothing* cases where C is zero bytes  */
    if(nocb==0) return;
    if(nora==0) return;
    if(noca==0)
    {
        memset(c,0,dsbc.nob*nora);
        return;
    }


    if(f->cauldron != 0)
    {
        // how many alcoves are there to do
        alca=(noca+f->alcove-1)/f->alcove;
        // how many cauldrons are there to do
        cauls=(nocb+f->cauldron-1)/f->cauldron;

/* Zeroize C */
        cc=AlignMalloc(f->cfmtcauld*cauls*nora);
        CZer(&dsbc,cc,nora);

/* Convert B to Bfmt */
        bb=malloc(f->bbrickbytes*alca*cauls);
        rowsleft=noca;
        for(j=0;j<alca;j++)
        {
            rowstodo=rowsleft;
            if(rowstodo>f->alcove) rowstodo=f->alcove;
            ptb=b+j*dsbc.nob*f->alcove;
            colsleft=nocb;
            for(k=0;k<cauls;k++)
            {
                colstodo=colsleft;
                if(colstodo>f->cauldron) colstodo=f->cauldron;
                DtoB(&dsbc,
                     ptb+k*f->dfmtcauld,
                     bb+(k*alca+j)*f->bbrickbytes,
                     rowstodo,
                     colstodo);
                colsleft-=colstodo;
            }
            rowsleft-=rowstodo;
        }

        boxes=(nora+f->recbox-1)/f->recbox;
        ix=malloc(alca*sizeof(uint64_t));
        aa=malloc(8+alca*(1+f->recbox*f->alcovebytes));
        bwa=AlignMalloc(f->bwasize);
        BwaInit(f,bwa);
        for(box=0;box<boxes;box++)
        {

/* convert box of rows to Afmt */
            rowstodo=nora-box*f->recbox;
            if(rowstodo>f->recbox) rowstodo=f->recbox;
            DtoA(&dsa,
                 ix,
                 a+box*f->recbox*dsa.nob,
                 aa,
                 rowstodo,
                 noca);

/*  Do the matrix multiplication  */
            for(k=0;k<cauls;k++)
            {
                for(j=0;j<alca;j++)
                {
                    BrickMad(f,
                             bwa,
                             aa+j*(1+rowstodo*f->alcovebytes),
                             bb+(k*alca+j)*f->bbrickbytes,
                             cc+(box*f->recbox+k*nora)*f->cfmtcauld
                            );
                }
            }
        }
        AlignFree(bwa);
        free(aa);
        free(bb);
        free(ix);
        CtoD(&dsbc,cc,c,nora);
        AlignFree(cc);
        return;
    }

/* if there is no HPMI, do it by steam (for now) */
/* Maybe this should include grease and stuff later */
/* but the correct answer is to write more HPMI primes!  */
    memset(c,0,dsbc.nob*nora);
    pta=a;
    ptc=c;
    for(i=0;i<nora;i++)
    {
        ptb=b;
        for(j=0;j<noca;j++)
        {
            e=DUnpak(&dsa,j,pta);
            DSMad(&dsbc,e,1,ptb,ptc);
            ptb+=dsbc.nob;
        }
        pta+=dsa.nob;
        ptc+=dsbc.nob;
    }
    return;
}

/* end of pmul.c  */
