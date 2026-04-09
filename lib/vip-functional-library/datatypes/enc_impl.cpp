#include "enc_lib.h"

using namespace enc_lib;

enc_char null_char  __attribute__((__section__("ciphertexts"))) = enc_char('\0');
enc_double::operator enc_long() {     return enc_long((long)(this->decrypt_double()));   }
enc_float::operator enc_double() {    return enc_double((double)(this->decrypt_float()));   }
enc_uint32::operator enc_ulong() {    return enc_ulong((unsigned long)(this->decrypt_uint32()));   };
enc_uint32::operator enc_int()   {    return enc_int((int)(this->decrypt_uint32()));   };
enc_int::operator enc_double()   {    return enc_double((double)this->decrypt_int());}; 
enc_int::operator enc_float()   {    return enc_float((float)this->decrypt_int());}; 
enc_uint32::operator enc_double()   {    return enc_double((double)this->decrypt_uint32());}; 
enc_int::operator enc_uint32()   {    return enc_uint32((unsigned int)this->decrypt_int());};
enc_int::operator enc_uint64()   {    return enc_uint64((uint64_t)this->decrypt_int());};
