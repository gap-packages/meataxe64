/*    Meataxe-64         conreg.c           */
/*    ==========         ========           */
/*   Test the Conway Polynomial computation */

/*    R. A. Parker      4.05.13             */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"

int errors;
uint64 fdef;

int main(int argc, char ** argv)
{
    FILE * f;
    int p,deg,i,j,res,junk;
    FIELD *field;
    uint64 prim,conp,k;
    int pol[70];
    f = fopen(argv[1],"rb");
    if(f==NULL)
    {
        printf("Conway polynomial file not found\n");
        exit(2);
    }
    junk=0;
    while(1)
    {
        junk+=fscanf(f,"%d",&p);
        if(p==-1) break;
        junk+=fscanf(f,"%d",&deg);
        for(i=0;i<=deg;i++) junk+=fscanf(f,"%d",&pol[i]);
        prim=p;
        fdef=p;
        for(j=1;j<deg;j++) fdef=fdef*prim;
        field = (FIELD *)malloc(FIELDLEN);
        res=FieldASet1(fdef,field,8);
        conp=0;
        for(j=0;j<deg;j++)
        {
            i=(p-pol[deg-j-1])%p;
            k=i;
            conp=conp*prim+k;
        }
        if( (res<1) || (conp != field->conp) )
        {
            printf(" case %luull: ",fdef);
            printf(" f->conp  = %luull; break;\n",conp);
            errors++;
        }
        free(field);
    }   
    if(junk==44) printf("\n");
    printf("Conway Polynomial test completed - %d errors\n",errors);
    return 0;
}

/*    end of conreg.c       */
