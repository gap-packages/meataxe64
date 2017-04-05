/*
      zsv.c      meataxe-64 Select Vector program
      ======     R. A. Parker 30.08.14
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
    EFIL *e1,*e2;
    uint64 hdr[5];
    uint64 fdef,nor,noc,i,old,depth,fpow;
    FIELD * f;
    long vecno;
    DSPACE ds;
    Dfmt *m;
    Dfmt *v;
    Dfmt *vpt;
    FELT fel;

    LogCmd(argc,argv);
    if (argc != 4) 
    {
        LogString(80,"usage zsv <m> <vecno> <vector>");
        exit(14);
    }
    e1 = ERHdr(argv[1],hdr);
    fdef=hdr[1];
    nor=hdr[2];
    noc=hdr[3];
    f = malloc(FIELDLEN);
    FieldSet(fdef,f);
    DSSet(f,noc,&ds);
    m=malloc(ds.nob*nor);
    v=malloc(ds.nob);
    ERData(e1,ds.nob*nor,m);
    ERClose(e1);
// work out what vector we want
    vecno=atol(argv[2]);
    old=0;
    depth=0;
    fpow=1;
    while(vecno>=(old+fpow))
    {
        old+=fpow;
        fpow=fpow*fdef;
        depth++;
    }
    if(depth>=nor)
    {
        LogString(81,"Vector number outside range");
        exit(23);
    }
    vecno-=old;
// make that vector in v
    memset(v,0,ds.nob);
    for(i=0;i<depth;i++)
    {
        vpt=DPAdv(&ds,i,m);
        fel=vecno % fdef;
        DSMad(&ds,fel,1,vpt,v);
        vecno=vecno / fdef;
    }
    vpt=DPAdv(&ds,depth,m);
    DAdd(&ds,1,vpt,v,v);
    hdr[2]=1;
    e2 = EWHdr(argv[3],hdr);
    EWData(e2,ds.nob,v);
    EWClose(e2);
    free(f);
    free(m);
    free(v);
    return 0;
}

/*  end of zsv.c    */
