#ifndef MTX64_BITSTRING_H
#define MTX64_BITSTRING_H

#include "meataxe64.h"
#include "mtx64/bitstring.h"

/* Functions that deal with the organisation of bitstring data in a bag */

static inline uint64_t *DataOfBitStringObject(Obj bs) {
  return (uint64_t *)(ADDR_OBJ(bs) + 1);
}

static inline UInt Size_Bits_BitString(UInt len) {
  return 8 * ((len + 63) / 64);
}

static inline UInt Size_Data_BitString(UInt len) {
  return 2 * sizeof(uint64_t) + Size_Bits_BitString(len);
}

static inline UInt Size_Bag_BitString(UInt len) {
  return sizeof(Obj) + Size_Data_BitString(len);
}


extern Obj MTX64_MakeBitString(UInt len);

extern UInt IS_MTX64_BitString(Obj bs);

static inline void CHECK_MTX64_BitString(Obj bs) {
  if (!IS_MTX64_BitString(bs))
    ErrorMayQuit("Invalid argument, expecting a meataxe64 bitstring", 0, 0);
}


extern StructInitInfo InitBitString;
#endif
