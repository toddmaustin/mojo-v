//
// TrustForge privacy enhanced execution configuration
//

#include "../common/utils.h"
#include <valarray>
#include <vector>

#if defined(VIP_NA_MODE)

#define VIP_INIT
#define VIP_ENCCHAR     char
#define VIP_ENCUCHAR    unsigned char
#define VIP_ENCINT      int
#define VIP_ENCUINT     unsigned int
#define VIP_ENCUINT64   uint64_t
#define VIP_ENCLONG     long
#define VIP_ENCULONG    unsigned long
#define VIP_ENCBOOL     bool
#define VIP_ENCFLOAT    float
#define VIP_ENCDOUBLE   double
#define VIP_VEC_ENCDOUBLE   std::valarray<VIP_ENCDOUBLE>
#define VIP_VEC_ENCINT      std::valarray<VIP_ENCINT>
#define VIP_DEC(X)      (X)
#define VIP_DEC_VEC(X)  (X)
#define VIP_EMITCT(F,X,C,S,D) ({ (void)(X); if (((C) = (C) + 1) >= (S)) goto D; })

extern inline void vip_init_vector(VIP_VEC_ENCINT* dest, std::vector<int>* source) {
    std::copy(source->begin(), source->end(), std::begin(*dest));
}
inline VIP_VEC_ENCINT   operator<<   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(rhs); }
inline VIP_VEC_ENCINT   operator>>   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(-rhs); }

#elif defined(VIP_DO_MODE)

#define VIP_INIT
#define VIP_ENCCHAR     char
#define VIP_ENCUCHAR    unsigned char
#define VIP_ENCINT      int
#define VIP_ENCUINT     unsigned int
#define VIP_ENCUINT64   uint64_t
#define VIP_ENCLONG     long
#define VIP_ENCULONG    unsigned long
#define VIP_ENCBOOL     bool
#define VIP_ENCFLOAT    float
#define VIP_ENCDOUBLE   double
#define VIP_VEC_ENCDOUBLE   std::valarray<VIP_ENCDOUBLE>
#define VIP_VEC_ENCINT      std::valarray<VIP_ENCINT>
#define VIP_DEC(X)      (X)
#define VIP_DEC_VEC(X)  (X)
#define VIP_CMOV(P,A,B) ((P) ? (A) : (B))
#define VIP_EMITCT(F,X,C,S,D) ({ (void)(X); if (((C) = (C) + 1) >= (S)) goto D; })

extern inline void vip_init_vector(VIP_VEC_ENCINT* dest, std::vector<int>* source) {
    std::copy(source->begin(), source->end(), std::begin(*dest));
}
inline VIP_VEC_ENCINT   operator<<   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(rhs); }
inline VIP_VEC_ENCINT   operator>>   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(-rhs); }

#elif defined(VIP_ENC_MODE)

#include "tf.h"

#define VIP_INIT        tfInit(NULL, false)

#define VIP_ENCCHAR     tfChar
#define VIP_ENCUCHAR    tfUChar
#define VIP_ENCINT      tfInt32
#define VIP_ENCUINT     tfUInt32
#define VIP_ENCLONG     tfLong64
#define VIP_ENCUINT64   tfULong64
#define VIP_ENCULONG    tfULong64
#define VIP_ENCBOOL     tfBool
#define VIP_ENCFLOAT    tfFloat
#define VIP_ENCDOUBLE   tfDouble
#define VIP_VEC_ENCDOUBLE   std::valarray<VIP_ENCDOUBLE>
#define VIP_VEC_ENCINT      std::valarray<VIP_ENCINT>
#define VIP_DEC(X)      ((X).asval())
#define VIP_DEC_VEC(X)      (vip_dec_vector(X))
#define VIP_CMOV(P,A,B) tfCMOV((P), (A), (B))
#define VIP_EMITCT(F,X,C,S,D) ({ if (((C) = vip_emitct((F),(X).data.x.bits128,(C),(S))) >= (S)) goto D; })

extern inline uint64_t
vip_emitct(FILE *outfile, uint128_t ct, uint64_t count, uint64_t samples)
{
  if (count < samples)
  {
    fprintf(outfile, "%10u\n", uint32_t(ct & 0xffffffff));
    count++;
  }
  if (count < samples)
  {
    fprintf(outfile, "%10u\n", uint32_t((ct >> 32) & 0xffffffff));
    count++;
  }
  if (count < samples)
  {
    fprintf(outfile, "%10u\n", uint32_t((ct >> 64) & 0xffffffff));
    count++;
  }
  if (count < samples)
  {
    fprintf(outfile, "%10u\n", uint32_t((ct >> 96) & 0xffffffff));
    count++;
  }

  return count;
}

extern inline void vip_init_vector(VIP_VEC_ENCINT* dest, std::vector<int>* source) {
    dest->resize(source->size());
    for (unsigned int i=0; i<source->size(); i++) {
        (*dest)[i] = (*source)[i];
    }
}
extern inline std::valarray<int> vip_dec_vector(VIP_VEC_ENCINT x){
    std::valarray<int> p(x.size());
    for(unsigned int i=0; i<x.size(); i++){
        p[i] = VIP_DEC(x[i]);
    }
    return p;
}
inline VIP_VEC_ENCINT   operator<<   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(rhs); }
inline VIP_VEC_ENCINT   operator>>   (VIP_VEC_ENCINT lhs, int rhs)   { return lhs.cshift(-rhs); }


// TrustForge require DO mode code
#undef  VIP_DO_MODE
#define VIP_DO_MODE

#endif

