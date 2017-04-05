/*    Meataxe-64    pcritPILc.c     */
/*    ==========    =========     */

/*    R. A. Parker      29 July 2015 */

#include <stdio.h>
#include <stdint.h>
#include "field.h"

/* these routines here because they depend on technology */

extern void pchal(FIELD * f)
{
    f->pcgen=1;
    f->pcsubgen=2;
}

/* end of pcrit1c.c  */
