/*   gcodes.h Grease Codes Header   */
/*   R. A. Parker 31.10.2016       */

extern uint16_t * findcode(int type, int p);
extern int mkatab(int type, int p, uint8_t * tab);
extern int addchain(int type, int p, uint8_t * adc);

/* end of gcodes.h  */
