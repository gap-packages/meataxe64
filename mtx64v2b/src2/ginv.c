/*
      ginv.c     Grease Code investigate
      =======    R. A. Parker   28.3.2018
                 only characteristic 2 so far
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "slab.h"
 
int main(int argc,  char **argv)
{
    EFIL *ef;
    FIELD *f;
    uint64_t hdr[5],fdef,nor,noc,cl;
    int i1,i2,i3,i4,j,vecs;
    uint8_t * m;
    DSPACE ds;
    Dfmt *cd,*v1;

    if (argc != 2)
    {
        LogString(80,"usage ginv code");
        exit(21);
    }
    ef = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor =hdr[2];
    noc =hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc,&ds);
    cd=malloc(ds.nob*nor);
    ERData(ef,nor*ds.nob,cd);
    ERClose(ef);
    v1=malloc(ds.nob);
    vecs=1;
    for(i1=0;i1<noc;i1++) vecs=vecs*f->fdef;
    printf("Code field %ld dimension %ld, words %ld, vecs %d\n",
                      f->fdef,ds.noc,nor,vecs);
    m=malloc(vecs);
    for(j=0;j<vecs;j++) m[j]=0;
    for(i1=0;i1<nor;i1++)
    {
        for(i2=0;i2<=i1;i2++)
        {
            memcpy(v1,cd+i1*ds.nob,ds.nob);
            DAdd(&ds,1,cd+i2*ds.nob,v1,v1);
            j=0;
            for(i3=0;i3<noc;i3++)
                j=f->fdef*j+DUnpak(&ds,i3,v1);
            m[j]++;
        }
    }
    for(j=0;j<vecs;j++)
    {
        if(m[j]!=0) continue;
        printf("Can't make %d\n",j);
    }
    for(i1=2;i1<nor;i1++)
    {
        for(i2=1;i2<i1;i2++)
        {
            for(i3=0;i3<i2;i3++)
            {
                memcpy(v1,cd+i1*ds.nob,ds.nob);
                DAdd(&ds,1,cd+i2*ds.nob,v1,v1);
                DAdd(&ds,1,cd+i3*ds.nob,v1,v1);
                cl=DNzl(&ds,v1);
                if(cl==ZEROROW)
                {
                    printf("%d = %d + %d\n",i1,i2,i3);
                    continue;
                }
                for(i4=0;i4<i3;i4++)
                {
                    if(!memcmp(v1,cd+i4*ds.nob,ds.nob))
                        printf("  %d = %d + %d + %d\n",i1,i2,i3,i4);
                }
            }
        }
    }
    printf("Grease Code Investigation Completed\n");
    free(cd);
    free(v1);
    free(f);
    free(m);
    return 0;
}

/******  end of ginv.c    ******/
