#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

static uint64_t mrbases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

typedef unsigned __int128 uint128_t;

static uint64_t pcpmad(uint64_t m, uint64_t a, uint64_t b, uint64_t c) {
    uint128_t x =  (uint128_t)a*(uint128_t)b + c;
    uint64_t q,r;
    asm( "divq %4"
         : "=a" (q), "=d" (r)
         : "0" ((uint64_t)x), "1" ((uint64_t)(x >>64)), "r" (m));
    return r;
}

static uint64_t ppow2 (uint64_t a , uint64_t b, uint64_t m )
{
    uint64_t c;
    uint64_t one,zero;
    one=1;
    zero=0;
    c=one;
/* invariant   answer = c * (a^b) */
    while (b != zero)
    {
        //        printf("BAA %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", a, b, c);
        if((b&one) == one) c = pcpmad(m,a,c,0);
        b >>= one;
        a = pcpmad(m,a,a,0);
    }
    return c;
}


uint64_t primtest(uint64_t n) {
    uint64_t n1 = n-1;
    uint64_t d  = n1;
    uint64_t r  = 0;
    while ((d & 1) == 0) {
        r++;
        d >>= 1;
    }
    //    printf("AAA %" PRIu64 " %" PRIu64 "\n", r, d);
    for (int i = 0; i < sizeof(mrbases)/sizeof(mrbases[0]); i++) {
        uint64_t a = mrbases[i];
        if (a == n)
            return 1;
        uint64_t x = ppow2(a, d, n);
        //        printf("AAB %" PRIu64 " %" PRIu64 "\n", a, x);
        if (x != 1 && x != n1) {
            uint64_t comp = 1;
            for (int j = 1; j < r; j++) {
                x = pcpmad(n, x, x, 0);
                if (x == n1) {
                    comp = 0;
                    break;
                }
            }
            if (comp)
                return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    uint64_t x;
    sscanf(argv[1], "%" PRIu64, &x);
    uint64_t ct;
    sscanf(argv[2], "%" PRIu64, &ct);

    uint64_t y = 0;
    for (uint64_t i = 0; i < ct; i++) {
        y+=primtest(x);
    }
    
    printf("%" PRIu64 "\n", y);
}
