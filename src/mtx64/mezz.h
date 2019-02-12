/*    Meataxe-64    mezz.h     */
/*    ==========    ======     */
/*    R. A. Parker     22.6.2017 */

extern void mmul(const char *m1, int s1, const char *m2, int s2,
                 const char *m3, int s3);
extern uint64_t mpef(const char *m1, int s1, const char *b2, int s2,
                     const char *m3, int s3);
extern uint64_t mech(const char *m1, int s1, const char *b2, int s2, 
                const char *b3, int s3, const char *m4, int s4,
                const char *m5, int s5, const char *m6, int s6);

/* end of mezz.h  */
