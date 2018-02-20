/*   proj15.c Projective grease level 1.5   */
/*            R. A. Parker 16.1.2018        */

// proj15 <field order> <seed>

#include <stdio.h>
#include <stdlib.h>
#include "field.h"

int main(int argc, char ** argv)
{
    uint64_t fdef;
    int seed;

    fdef=atoi(argv[1]);
    seed=atoi(argv[2]);
    srand(seed);
}

/* end of proj15.c  */
