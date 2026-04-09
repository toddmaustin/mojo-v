#ifndef ENC_LIB_H
#define ENC_LIB_H

#include <stdint.h>
#include <iostream>
#include <math.h>
#include "../interface/interface.h"
namespace enc_lib
{

struct enc_long; 
struct enc_ulong;
struct enc_uint32;
struct enc_uint64;
struct enc_float; 
struct enc_double; 


// Primitive C++ Data Types  
/************************************************************************************
/*  · Boolean
        Keyword: bool
        A logical value representing true or false. 

/*  · Character
        Keyword: char
        Character values in the range [-128, 127] or [0, 255]. Represented by 1 byte
        of memory on an unaltered machine. 

/*  · Integer
        Keyword: int
        Integer values in the range [-2,147,483,648 : 2,147,483,647]. Represented by 
        4 bytes of memory on an unaltered machine.

/*  · Floating Point
        Keyword: float
        Single precision floating point values or decimal values. Represented by 
        4 bytes of memory on an unaltered machine.

/*  · Double Floating Point
        Keyword: double
        Double precision floating point values or decimal values. Represented by
        8 bytes of memory on an unaltered machine.

/*  · Valueless or Void: NOT SUPPORTED 
        Keyword: void
        A valueless entity.

/*  - Wide Character: NOT SUPPORTED 
        Keyword: wchar_t
        A character data type that is larger in size. Represented by 2 or 4 bytes
        on an unaltered machine. 
/************************************************************************************/



// ENCRYPTED Primitive C++ Data Types  
/************************************************************************************/

/*****************************************************************************/
/*  · Encrypted Boolean
        Keyword: enc_bool
        An 8 byte (64-bit) ciphertext containing: 
        A 1 byte boolean value followed by 7 bytes (56 bits) of random salt
*/
    typedef struct enc_bool{
       /*******************************/
        bit128_t ciphertext; // 1B bool, 7B padding = 8B (64-bit value)
                             // 1B bool, 15B padding = 16B (128-bit value)
       /*******************************/
       /*** Constructors & Equality ***/
        enc_bool()                { bool value = 0; ciphertext = encrypt_8_128(((uint8_t*) &value)); }
        enc_bool(bool value)      { ciphertext = encrypt_8_128(((uint8_t*) &value)); }
        enc_bool(bit128_t value)   { ciphertext = value; }
        inline enc_bool operator= (int i1) __attribute__((always_inline)) { ciphertext = encrypt_8_128(((uint8_t*) &i1)); return *this; } 

        private: 
        bool decrypt_bool(){
                // More complex decryption function that implements pointer casts rather than static casts 
                uint8_t value = decrypt_128_8(this->ciphertext);
                return *((bool*) &value);
        }
        public:
       /****************************/
       /*** One-Input Operations ***/
        inline friend enc_bool operator! (enc_bool b1) __attribute__((always_inline)) { return enc_bool(!b1.decrypt_bool()); }        

       /****************************/
       /*** Two-Input Operations ***/
        // // Logic, Enc-Enc Op          
        inline friend enc_bool operator&& (enc_bool b1, enc_bool b2) __attribute__((always_inline)) {return enc_bool(b1.decrypt_bool() && b2.decrypt_bool()); }
        inline friend enc_bool operator|| (enc_bool b1, enc_bool b2) __attribute__((always_inline)) {return enc_bool(b1.decrypt_bool() || b2.decrypt_bool()); }
        // // Logic, Enc-Plaintext Op
        // // Logic, Plaintext-Enc Op

       /**************/
       /*** Other ***/
        bool GET_DECRYPTED_VALUE(){ //std::cerr << "ENC_BOOL: EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";
         return this->decrypt_bool(); }
        inline friend enc_bool   CMOV(enc_bool cond, enc_bool v_true, enc_bool v_false) __attribute__((always_inline)) {
                uint16_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.decrypt_bool()), "r" ((uint16_t)v_true.decrypt_bool()), "r" ((uint16_t)v_false.decrypt_bool())
                    : "cc"
                );
                return enc_bool(result);
        }
        inline friend enc_bool   CMOV(enc_bool cond, bool v_true, enc_bool v_false) __attribute__((always_inline)) {
                uint16_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.decrypt_bool()), "r" ((uint16_t)v_true), "r" ((uint16_t)v_false.decrypt_bool())
                    : "cc"
                );
                return enc_bool(result);
        }
    } enc_bool;
/*****************************************************************************/





/*****************************************************************************/
/*  · Encrypted Integer
        Keyword: enc_int
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte signed integer value (double the size of regular integers) followed 
        by 8 bytes (64 bits) of random salt
*/
    typedef struct enc_int{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_int()                 { int value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_int(int64_t value)    { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_int(bit128_t value)   { ciphertext = value; }
        inline enc_int operator= (int i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        inline enc_int operator= (enc_int i1) __attribute__((always_inline)) { int x = i1.decrypt_int(); ciphertext = encrypt_64_128(((uint64_t*) &(x))); return *this; } //trying fix for pointer cast

        // private: 
        int decrypt_int(){
                // More complex decryption function that implements pointer casts rather than static casts 
                uint64_t value = decrypt_128_64(this->ciphertext);
                return *((int*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_int operator++() __attribute__((always_inline))  { 
                int value = this->decrypt_int();
                enc_int temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_int operator++(int) __attribute__((always_inline))  { 
                int value = this->decrypt_int();
                enc_int temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
        inline enc_int operator-() __attribute__((always_inline))  { 
                double value = this->decrypt_int();
                enc_int temp(-value); // Pre-Negate
                return temp; 
        }
       /****************************/
       /*** One-Input Operations ***/
        inline enc_int operator+=(enc_int rhs)    __attribute__((always_inline))  { int temp = this->decrypt_int() + rhs.decrypt_int(); this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_int operator+=(const int& rhs) __attribute__((always_inline))  { int temp = this->decrypt_int() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_int operator-=(const int& rhs) __attribute__((always_inline))  { int temp = this->decrypt_int() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_int       operator+   (enc_int lhs, enc_int rhs)       __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() + rhs.decrypt_int()); }
        inline friend enc_int       operator-   (enc_int lhs, enc_int rhs)       __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() - rhs.decrypt_int()); }
        inline friend enc_int       operator*   (enc_int lhs, enc_int rhs)       __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() * rhs.decrypt_int()); }   
        inline friend enc_int       operator/   (enc_int lhs, enc_int rhs)       __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() / rhs.decrypt_int()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_int       operator+   (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() + rhs); }
        inline friend enc_int       operator-   (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() - rhs); }
        inline friend enc_int       operator*   (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() * rhs); }
        inline friend enc_int       operator/   (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() / rhs); }
        inline friend enc_int       operator%   (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() % rhs); }
        inline friend enc_int       operator<<  (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() << rhs); }
        inline friend enc_int       operator>>  (enc_int lhs, int rhs)           __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() >> rhs); }

        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_int      operator+   (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs + rhs.decrypt_int()); }
        inline friend enc_int      operator-   (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs - rhs.decrypt_int()); }
        inline friend enc_int      operator*   (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs * rhs.decrypt_int()); }
        inline friend enc_int      operator/   (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs / rhs.decrypt_int()); }
        inline friend enc_int      operator%   (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs % rhs.decrypt_int()); }
        inline friend enc_int      operator<<  (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs << rhs.decrypt_int()); }
        inline friend enc_int      operator>>  (int lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_int(lhs >> rhs.decrypt_int()); }

        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_int()) == (rhs.decrypt_int())));   }
        inline friend enc_bool     operator!=  (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_int()) != (rhs.decrypt_int())));   }
        inline friend enc_bool     operator<   (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_int()) < (rhs.decrypt_int())));   }
        inline friend enc_bool     operator<=  (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_int()) > (rhs.decrypt_int())));  }
        inline friend enc_bool     operator>   (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_int()) < (lhs.decrypt_int()));     }
        inline friend enc_bool     operator>=  (enc_int lhs, enc_int rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_int()) < (rhs.decrypt_int())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_int()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_int()) == rhs));   }
        inline friend enc_bool     operator<   (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_int()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_int()) > rhs));    }
        inline friend enc_bool     operator>   (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_int()));       }
        inline friend enc_bool     operator>=  (enc_int lhs, int rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_int()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_int()));     }
        inline friend enc_bool     operator!=  (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_int())));  }
        inline friend enc_bool     operator<   (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_int())));    }
        inline friend enc_bool     operator<=  (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_int())));   }
        inline friend enc_bool     operator>   (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_int()) < lhs);      }
        inline friend enc_bool     operator>=  (int lhs, enc_int rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_int())));   }
       /*************/
        inline friend enc_int      operator&   (enc_int lhs, enc_int rhs)      __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() & rhs.decrypt_int()); }
        inline friend enc_int      operator|   (enc_int lhs, enc_int rhs)      __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() | rhs.decrypt_int()); }  
        inline friend enc_int      operator^   (enc_int lhs, enc_int rhs)      __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() ^ rhs.decrypt_int()); }
        inline friend enc_int      operator>>  (enc_int lhs, enc_int rhs)      __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() >> rhs.decrypt_int()); }
        inline friend enc_int      operator<<  (enc_int lhs, enc_int rhs)      __attribute__((always_inline))  { return enc_int(lhs.decrypt_int() << rhs.decrypt_int()); } 
       /*** Math ***/
        // inline friend enc_int      sqrt   (enc_int x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_int(sqrt(x.decrypt_int())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_int      pow    (enc_int x, int y)                     __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_int(pow(x.decrypt_int(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_int      acos   (enc_int x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_int(acos(x.decrypt_int())); }   //TODO Should implement d.o. acos function
        // inline friend enc_int      cos    (enc_int x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_int(cos(x.decrypt_int())); }    //TODO Should implement d.o. cos function
        // inline friend enc_int      fabs   (enc_int x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_int(fabs(x.decrypt_int())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        int GET_DECRYPTED_VALUE(){ //std::cerr << "ENC_INT: EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";
         return this->decrypt_int(); }


        inline friend enc_int       CMOV(enc_bool cond, enc_int v_true, enc_int v_false) __attribute__((always_inline)) {
                int result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_int()), "r" (v_false.decrypt_int())
                    : "cc"
                );
                return enc_int(result);
        }
        inline friend enc_int       CMOV(enc_bool cond, int v_true, enc_int v_false) __attribute__((always_inline)) {
                int result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true), "r" (v_false.decrypt_int())
                    : "cc"
                );
                return enc_int(result);
        }     
        operator enc_double(); 
        operator enc_float(); 
        operator enc_uint32();
        operator enc_uint64();
    } enc_int;
/*****************************************************************************/




/*****************************************************************************/
/*  · Encrypted Character
        Keyword: enc_char
        An 8 byte (64-bit) ciphertext containing: 
        A 1 byte char value followed by 7 bytes (56 bits) of random salt
*/
    typedef struct enc_char{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_char()                 { char value = 0; ciphertext = encrypt_8_128(((uint8_t*) &value)); }
        enc_char(char value)       { ciphertext = encrypt_8_128(((uint8_t*) &value)); }
        enc_char(bit128_t value)   { ciphertext = value; }
        inline enc_char operator= (int i1) __attribute__((always_inline)) { ciphertext = encrypt_8_128(((uint8_t*) &i1)); return *this; } 

        private: 
        char decrypt_char(){
                // More complex decryption function that implements pointer casts rather than static casts 
                uint8_t value = decrypt_128_8(this->ciphertext);
                return *((char*) &value);
        }
        public:
        // Arithmetic
         inline friend enc_int  operator+   (enc_char& lhs, enc_char& rhs)   __attribute__((always_inline)) { return enc_int(lhs.decrypt_char() + rhs.decrypt_char()); }
         inline friend enc_int  operator-   (enc_char& lhs, enc_char& rhs)   __attribute__((always_inline)) { return enc_int(lhs.decrypt_char() - rhs.decrypt_char()); }
        
        // Logical Operators
        inline friend enc_bool     operator==  (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool(lhs.decrypt_char() == rhs.decrypt_char()); }
        inline friend enc_bool     operator!=  (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool(!(lhs.decrypt_char() == rhs.decrypt_char())); }
        inline friend enc_bool     operator<   (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool((lhs.decrypt_char() < rhs.decrypt_char())); }
        inline friend enc_bool     operator<=  (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool(!(lhs.decrypt_char() > rhs.decrypt_char())); }
        inline friend enc_bool     operator>   (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool((rhs.decrypt_char() > lhs.decrypt_char())); }
        inline friend enc_bool     operator>=  (enc_char& lhs, enc_char& rhs) __attribute__((always_inline)) { return enc_bool(!(lhs.decrypt_char() < rhs.decrypt_char())); }

        inline friend enc_bool   operator==  (enc_char lhs, char rhs)       __attribute__((always_inline)) { return enc_bool(lhs.decrypt_char() == rhs); }

        /**************/
       /*** Other ***/
        char GET_DECRYPTED_VALUE(){ 
                //std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n"; 
                return this->decrypt_char(); }
        inline friend enc_char   CMOV(enc_bool cond, enc_char v_true, enc_char v_false) __attribute__((always_inline)) {
                uint16_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;" 
                    "cmovnz %2, %0;" 
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" ((uint16_t)v_true.decrypt_char()), "r" ((uint16_t)v_false.decrypt_char())
                    : "cc"
                );
                return enc_char(result);
        }
        inline friend enc_char   CMOV(enc_bool cond, enc_char v_true, char v_false) __attribute__((always_inline)) {
                uint16_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  
                    "cmovnz %2, %0;" 
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" ((uint16_t)v_true.decrypt_char()), "r" ((uint16_t)v_false)
                    : "cc"
                );
                return enc_char(result);
        }
        inline friend enc_char   CMOV(enc_bool cond, char v_true, enc_char v_false) __attribute__((always_inline)) {
                uint16_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;" 
                    "cmovnz %2, %0;" 
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" ((uint16_t)v_true), "r" ((uint16_t)v_false.decrypt_char())
                    : "cc"
                );
                return enc_char(result);
        }
        inline friend enc_char*   CMOV(enc_bool cond, enc_char* v_true, enc_char* v_false) __attribute__((always_inline)) {
                enc_char* result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;" 
                    "cmovnz %2, %0;" 
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true), "r" (v_false)
                    : "cc"
                );
                return result;
        }
        // inline friend enc_char*   CMOV(enc_bool cond, enc_char* v_true, enc_char* v_false) __attribute__((always_inline)) {
        //         log("WARNING:: USE OF UNVERIFIED FUNCTION\n");
        //         enc_char* result;
        //         __asm__ __volatile__ (
        //             "test   %1, %1;"
        //             "cmovz  %3, %0;" 
        //             "cmovnz %2, %0;" 
        //             : "=r" (result)
        //             : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true), "r" (v_false)
        //             : "cc"
        //         );
        //         return result;
        // }      
        /*** Type Casting ***/
        operator enc_int        () {     return enc_int((int)(this->decrypt_char()));   }
        // operator bool           () const{  log("EXTERNAL CAST TO BOOL\n");           abort(); }
        // operator char           () const{  log("EXTERNAL CAST TO CHAR\n");           abort(); }
        // operator int            () const{  log("EXTERNAL CAST TO INT\n");            abort(); }
        // operator unsigned int   () const{  log("EXTERNAL CAST TO UNSIGNED INT\n");   abort(); }
        // operator long           () const{  log("EXTERNAL CAST TO LONG\n");           abort(); }
        // operator unsigned long  () const{  log("EXTERNAL CAST TO UNSIGNED LONG\n");  abort(); } 
        // operator double         () const{  log("EXTERNAL CAST TO DOUBLE\n");         abort(); }
        // operator float          () const{  log("EXTERNAL CAST TO FLOAT\n");          abort(); }
        /*************/  
    } enc_char;
/*****************************************************************************/

/*****************************************************************************/
/*  · Encrypted Unsigned Integer
        Keyword: enc_uint64
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte unsigned integer value (double the size of regular integers) followed 
        by 8 bytes (64 bits) of random salt
*/    
    typedef struct enc_uint64{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_uint64()                    { uint64_t value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_uint64(uint64_t value)      { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_uint64(bit128_t value)     { ciphertext = value; }
        inline enc_uint64 operator= (uint64_t i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        private: 
        uint64_t decrypt_uint64(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from uint to uint64_t drops decimal values, producing an incorrect result)
                uint64_t value = decrypt_128_64(this->ciphertext);
                return *((uint*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_uint64 operator++() __attribute__((always_inline))  { 
                uint64_t value = this->decrypt_uint64();
                enc_uint64 temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_uint64 operator++(int) __attribute__((always_inline))  { 
                uint64_t value = this->decrypt_uint64();
                enc_uint64 temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
       /****************************/
       /*** One-Input Operations ***/
        inline enc_uint64 operator+=(const uint64_t& rhs) __attribute__((always_inline))  { uint64_t temp = this->decrypt_uint64() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_uint64 operator-=(const uint64_t& rhs) __attribute__((always_inline))  { uint64_t temp = this->decrypt_uint64() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_uint64 operator&=(const int& rhs)      __attribute__((always_inline))  { uint64_t temp = this->decrypt_uint64() & rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }

       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_uint64   operator+   (enc_uint64 lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() + rhs.decrypt_uint64()); }
        inline friend enc_uint64   operator-   (enc_uint64 lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() - rhs.decrypt_uint64()); }
        inline friend enc_uint64   operator*   (enc_uint64 lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() * rhs.decrypt_uint64()); }   
        inline friend enc_uint64   operator/   (enc_uint64 lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() / rhs.decrypt_uint64()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_uint64   operator+   (enc_uint64 lhs, uint64_t rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() + rhs); }
        inline friend enc_uint64   operator-   (enc_uint64 lhs, uint64_t rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() - rhs); }
        inline friend enc_uint64   operator*   (enc_uint64 lhs, uint64_t rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() * rhs); }
        inline friend enc_uint64   operator/   (enc_uint64 lhs, uint64_t rhs)           __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() / rhs); }
        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_uint64   operator+   (uint64_t lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs + rhs.decrypt_uint64()); }
        inline friend enc_uint64   operator-   (uint64_t lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs - rhs.decrypt_uint64()); }
        inline friend enc_uint64   operator*   (uint64_t lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs * rhs.decrypt_uint64()); }
        inline friend enc_uint64   operator/   (uint64_t lhs, enc_uint64 rhs)           __attribute__((always_inline))  { return enc_uint64(lhs / rhs.decrypt_uint64()); }
        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint64()) == (rhs.decrypt_uint64())));   }
        inline friend enc_bool     operator!=  (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint64()) != (rhs.decrypt_uint64())));   }
        inline friend enc_bool     operator<   (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint64()) < (rhs.decrypt_uint64())));   }
        inline friend enc_bool     operator<=  (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint64()) > (rhs.decrypt_uint64())));  }
        inline friend enc_bool     operator>   (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_uint64()) < (lhs.decrypt_uint64()));     }
        inline friend enc_bool     operator>=  (enc_uint64 lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint64()) < (rhs.decrypt_uint64())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_uint64()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint64()) == rhs));   }
        inline friend enc_bool     operator<   (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint64()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint64()) > rhs));    }
        inline friend enc_bool     operator>   (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_uint64()));       }
        inline friend enc_bool     operator>=  (enc_uint64 lhs, uint64_t rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint64()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_uint64()));     }
        inline friend enc_bool     operator!=  (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_uint64())));  }
        inline friend enc_bool     operator<   (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_uint64())));    }
        inline friend enc_bool     operator<=  (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_uint64())));   }
        inline friend enc_bool     operator>   (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_uint64()) < lhs);      }
        inline friend enc_bool     operator>=  (uint64_t lhs, enc_uint64 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_uint64())));   }
     // Bitwise, Enc-Enc Op
        inline friend enc_uint64      operator&   (enc_uint64 lhs, enc_uint64 rhs)      __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() & rhs.decrypt_uint64()); }
        inline friend enc_uint64      operator|   (enc_uint64 lhs, enc_uint64 rhs)      __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() | rhs.decrypt_uint64()); }  
        inline friend enc_uint64      operator^   (enc_uint64 lhs, enc_uint64 rhs)      __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() ^ rhs.decrypt_uint64()); }
        inline friend enc_uint64      operator>>  (enc_uint64 lhs, enc_uint64 rhs)      __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() >> rhs.decrypt_uint64()); }
        inline friend enc_uint64      operator<<  (enc_uint64 lhs, enc_uint64 rhs)      __attribute__((always_inline))  { return enc_uint64(lhs.decrypt_uint64() << rhs.decrypt_uint64()); } 
       /*************/
       /*** Math ***/
        // inline friend enc_uint64     sqrt   (enc_uint64 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint64(sqrt(x.decrypt_uint64())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_uint64     pow    (enc_uint64 x, uint64_t y)                  __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint64(pow(x.decrypt_uint64(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_uint64     acos   (enc_uint64 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint64(acos(x.decrypt_uint64())); }   //TODO Should implement d.o. acos function
        // inline friend enc_uint64     cos    (enc_uint64 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint64(cos(x.decrypt_uint64())); }    //TODO Should implement d.o. cos function
        // inline friend enc_uint64     fabs   (enc_uint64 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint64(fabs(x.decrypt_uint64())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        uint GET_DECRYPTED_VALUE(){ //std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n"; 
        return this->decrypt_uint64(); }

        inline friend enc_uint64       CMOV(enc_bool cond, enc_uint64 v_true, enc_uint64 v_false) __attribute__((always_inline)) {
                uint64_t result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_uint64()), "r" (v_false.decrypt_uint64())
                    : "cc"
                );
                return enc_uint64(result);
        }    
        operator enc_int() {return enc_int((int)(this->decrypt_uint64()));}
    } enc_uint64;
/*****************************************************************************/

/*****************************************************************************/
/*  · Encrypted Unsigned Integer
        Keyword: enc_uint32
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte unsigned integer value (double the size of regular integers) followed 
        by 8 bytes (64 bits) of random salt
*/    
    typedef struct enc_uint32{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_uint32()                    { unsigned int value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_uint32(unsigned int value)  { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_uint32(bit128_t value)      { ciphertext = value; }
        inline enc_uint32 operator= (unsigned int i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        private: 
        unsigned int decrypt_uint32(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from uint to unsigned int drops decimal values, producing an incorrect result)
                unsigned int value = decrypt_128_64(this->ciphertext);
                return *((uint*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_uint32 operator++() __attribute__((always_inline))  { 
                unsigned int value = this->decrypt_uint32();
                enc_uint32 temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_uint32 operator++(int) __attribute__((always_inline))  { 
                unsigned int value = this->decrypt_uint32();
                enc_uint32 temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
       /****************************/
       /*** One-Input Operations ***/
        inline enc_uint32 operator+=(const unsigned int& rhs) __attribute__((always_inline))  { unsigned int temp = this->decrypt_uint32() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_uint32 operator-=(const unsigned int& rhs) __attribute__((always_inline))  { unsigned int temp = this->decrypt_uint32() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }

        inline enc_uint32 operator&=(const unsigned int& rhs) __attribute__((always_inline))  { unsigned int temp = this->decrypt_uint32() & rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }

       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_uint32   operator+   (enc_uint32 lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() + rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator-   (enc_uint32 lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() - rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator*   (enc_uint32 lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() * rhs.decrypt_uint32()); }   
        inline friend enc_uint32   operator/   (enc_uint32 lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() / rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator%   (enc_uint32 lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() % rhs.decrypt_uint32()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_uint32   operator+   (enc_uint32 lhs, unsigned int rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() + rhs); }
        inline friend enc_uint32   operator-   (enc_uint32 lhs, unsigned int rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() - rhs); }
        inline friend enc_uint32   operator*   (enc_uint32 lhs, unsigned int rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() * rhs); }
        inline friend enc_uint32   operator/   (enc_uint32 lhs, unsigned int rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() / rhs); }
        inline friend enc_uint32   operator%   (enc_uint32 lhs, unsigned int rhs)           __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() % rhs); }
        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_uint32   operator+   (unsigned int lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs + rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator-   (unsigned int lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs - rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator*   (unsigned int lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs * rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator/   (unsigned int lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs / rhs.decrypt_uint32()); }
        inline friend enc_uint32   operator%   (unsigned int lhs, enc_uint32 rhs)           __attribute__((always_inline))  { return enc_uint32(lhs % rhs.decrypt_uint32()); }
        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint32()) == (rhs.decrypt_uint32())));   }
        inline friend enc_bool     operator!=  (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint32()) != (rhs.decrypt_uint32())));   }
        inline friend enc_bool     operator<   (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint32()) < (rhs.decrypt_uint32())));   }
        inline friend enc_bool     operator<=  (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint32()) > (rhs.decrypt_uint32())));  }
        inline friend enc_bool     operator>   (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_uint32()) < (lhs.decrypt_uint32()));     }
        inline friend enc_bool     operator>=  (enc_uint32 lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint32()) < (rhs.decrypt_uint32())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_uint32()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint32()) == rhs));   }
        inline friend enc_bool     operator<   (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_uint32()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint32()) > rhs));    }
        inline friend enc_bool     operator>   (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_uint32()));       }
        inline friend enc_bool     operator>=  (enc_uint32 lhs, unsigned int rhs)         __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_uint32()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_uint32()));     }
        inline friend enc_bool     operator!=  (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_uint32())));  }
        inline friend enc_bool     operator<   (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_uint32())));    }
        inline friend enc_bool     operator<=  (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_uint32())));   }
        inline friend enc_bool     operator>   (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_uint32()) < lhs);      }
        inline friend enc_bool     operator>=  (unsigned int lhs, enc_uint32 rhs)         __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_uint32())));   }
     // Bitwise, Enc-Enc Op
        inline friend enc_uint32      operator&   (enc_uint32 lhs, enc_uint32 rhs)      __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() & rhs.decrypt_uint32()); }
        inline friend enc_uint32      operator|   (enc_uint32 lhs, enc_uint32 rhs)      __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() | rhs.decrypt_uint32()); }  
        inline friend enc_uint32      operator^   (enc_uint32 lhs, enc_uint32 rhs)      __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() ^ rhs.decrypt_uint32()); }
        inline friend enc_uint32      operator>>  (enc_uint32 lhs, enc_uint32 rhs)      __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() >> rhs.decrypt_uint32()); }
        inline friend enc_uint32      operator<<  (enc_uint32 lhs, enc_uint32 rhs)      __attribute__((always_inline))  { return enc_uint32(lhs.decrypt_uint32() << rhs.decrypt_uint32()); } 
       /*************/
       /*** Math ***/
        // inline friend enc_uint32     sqrt   (enc_uint32 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint32(sqrt(x.decrypt_uint32())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_uint32     pow    (enc_uint32 x, unsigned int y)                  __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint32(pow(x.decrypt_uint32(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_uint32     acos   (enc_uint32 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint32(acos(x.decrypt_uint32())); }   //TODO Should implement d.o. acos function
        // inline friend enc_uint32     cos    (enc_uint32 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint32(cos(x.decrypt_uint32())); }    //TODO Should implement d.o. cos function
        // inline friend enc_uint32     fabs   (enc_uint32 x)                              __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_uint32(fabs(x.decrypt_uint32())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        uint GET_DECRYPTED_VALUE(){ //std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n"; 
        return this->decrypt_uint32(); }

        inline friend enc_uint32       CMOV(enc_bool cond, enc_uint32 v_true, enc_uint32 v_false) __attribute__((always_inline)) {
                unsigned int result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_uint32()), "r" (v_false.decrypt_uint32())
                    : "cc"
                );
                return enc_uint32(result);
        }    

        operator enc_ulong();
        operator enc_int();
        operator enc_double();

    } enc_uint32;
/*****************************************************************************/




/*****************************************************************************/
/*  · Encrypted Floating Point
        Keyword: enc_float
        A 16 byte (128-bit) ciphertext containing: 
        A 4 byte floating point value followed by 12 bytes (96 bits) of random salt
*/
    typedef struct enc_float{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_float()                  { float value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_float(float value)      { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_float(bit128_t value)    { ciphertext = value; }
        inline enc_float operator= (float i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        // private: 
        float decrypt_float(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from float to uint64_t drops decimal values, producing an incorrect result)
                uint64_t value = decrypt_128_64(this->ciphertext);
                return *((float*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_float operator++() __attribute__((always_inline))  { 
                float value = this->decrypt_float();
                enc_float temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_float operator++(int) __attribute__((always_inline))  { 
                float value = this->decrypt_float();
                enc_float temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
        inline enc_float operator-() __attribute__((always_inline))  { 
                double value = this->decrypt_float();
                enc_float temp(-value); // Pre-Negate
                return temp; 
        }
       /****************************/
       /*** One-Input Operations ***/
        inline enc_float operator+=(const float& rhs) __attribute__((always_inline))  { float temp = this->decrypt_float() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_float operator+=(enc_float rhs) __attribute__((always_inline))     { float temp = this->decrypt_float() + rhs.decrypt_float(); this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_float operator+=(const int& rhs) __attribute__((always_inline))    { float temp = this->decrypt_float() + (float)rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_float operator/=(const int& rhs) __attribute__((always_inline))    { float temp = this->decrypt_float() / (float)rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }

        inline enc_float operator-=(const float& rhs) __attribute__((always_inline))  { float temp = this->decrypt_float() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_float   operator+   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() + rhs.decrypt_float()); }
        inline friend enc_float   operator-   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() - rhs.decrypt_float()); }
        inline friend enc_float   operator*   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() * rhs.decrypt_float()); }   
        inline friend enc_float   operator/   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() / rhs.decrypt_float()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_float   operator+   (enc_float lhs, float rhs)            __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() + rhs); }
        inline friend enc_float   operator-   (enc_float lhs, float rhs)            __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() - rhs); }
        inline friend enc_float   operator*   (enc_float lhs, float rhs)            __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() * rhs); }
        inline friend enc_float   operator/   (enc_float lhs, float rhs)            __attribute__((always_inline))  { return enc_float(lhs.decrypt_float() / rhs); }
        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_float   operator+   (float lhs, enc_float rhs)            __attribute__((always_inline))  { return enc_float(lhs + rhs.decrypt_float()); }
        inline friend enc_float   operator-   (float lhs, enc_float rhs)            __attribute__((always_inline))  { return enc_float(lhs - rhs.decrypt_float()); }
        inline friend enc_float   operator*   (float lhs, enc_float rhs)            __attribute__((always_inline))  { return enc_float(lhs * rhs.decrypt_float()); }
        inline friend enc_float   operator/   (float lhs, enc_float rhs)            __attribute__((always_inline))  { return enc_float(lhs / rhs.decrypt_float()); }
        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_float()) == (rhs.decrypt_float())));   }
        inline friend enc_bool     operator!=  (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_float()) != (rhs.decrypt_float())));   }
        inline friend enc_bool     operator<   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_float()) < (rhs.decrypt_float())));   }
        inline friend enc_bool     operator<=  (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_float()) > (rhs.decrypt_float())));  }
        inline friend enc_bool     operator>   (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_float()) < (lhs.decrypt_float()));     }
        inline friend enc_bool     operator>=  (enc_float lhs, enc_float rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_float()) < (rhs.decrypt_float())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_float()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_float()) == rhs));   }
        inline friend enc_bool     operator<   (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_float()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_float()) > rhs));    }
        inline friend enc_bool     operator>   (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_float()));       }
        inline friend enc_bool     operator>=  (enc_float lhs, float rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_float()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_float()));     }
        inline friend enc_bool     operator!=  (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_float())));  }
        inline friend enc_bool     operator<   (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_float())));    }
        inline friend enc_bool     operator<=  (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_float())));   }
        inline friend enc_bool     operator>   (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_float()) < lhs);      }
        inline friend enc_bool     operator>=  (float lhs, enc_float rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_float())));   }
       /*************/
       /*** Math ***/
        // inline friend enc_float   sqrt   (enc_float x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_float(sqrt(x.decrypt_float())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_float   pow    (enc_float x, float y)                   __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_float(pow(x.decrypt_float(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_float   acos   (enc_float x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_float(acos(x.decrypt_float())); }   //TODO Should implement d.o. acos function
        // inline friend enc_float   cos    (enc_float x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_float(cos(x.decrypt_float())); }    //TODO Should implement d.o. cos function
        // inline friend enc_float   fabs   (enc_float x)                            __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_float(fabs(x.decrypt_float())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        float GET_DECRYPTED_VALUE(){ //std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";
         return this->decrypt_float(); }
        inline friend enc_float       CMOV(enc_bool cond, enc_float v_true, enc_float v_false) __attribute__((always_inline)) {
                float result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_float()), "r" (v_false.decrypt_float())
                    : "cc"
                );
                return enc_float(result);
        }

        operator enc_double();

    } enc_float;
/*****************************************************************************/




/*****************************************************************************/
/*  · Encrypted Double
        Keyword: enc_double
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte double precision floating point (aka "double") value followed by 
        8 bytes (64 bits) of random salt
*/
    typedef struct enc_double{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_double()                    { double value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_double(double value)        { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_double(bit128_t value)      { ciphertext = value; }
        inline enc_double operator= (double i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        public: 
        double decrypt_double(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from double to uint64_t drops decimal values, producing an incorrect result)
                // printf("Before: %f\n", (double) this->ciphertext.getLowerValue_64b());
                uint64_t value = decrypt_128_64(this->ciphertext);
                // printf("After: %f\n\n", *((double*) &value));
                return *((double*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_double operator++() __attribute__((always_inline))  { 
                double value = this->decrypt_double();
                enc_double temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_double operator++(int) __attribute__((always_inline))  { 
                double value = this->decrypt_double();
                enc_double temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
        inline enc_double operator-() __attribute__((always_inline))  { 
                double value = this->decrypt_double();
                enc_double temp(-value); // Pre-Negate
                return temp; 
        }
       /****************************/
       /*** One-Input Operations ***/
        inline enc_double operator+=(const double& rhs) __attribute__((always_inline))  { double temp = this->decrypt_double() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_double operator-=(const double& rhs) __attribute__((always_inline))  { double temp = this->decrypt_double() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_double operator+=(enc_float& rhs)      __attribute__((always_inline))  { double temp = this->decrypt_double() + rhs.decrypt_float(); this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }

       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_double   operator+   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() + rhs.decrypt_double()); }
        inline friend enc_double   operator-   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() - rhs.decrypt_double()); }
        inline friend enc_double   operator*   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() * rhs.decrypt_double()); }   
        inline friend enc_double   operator/   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() / rhs.decrypt_double()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_double   operator+   (enc_double lhs, double rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() + rhs); }
        inline friend enc_double   operator-   (enc_double lhs, double rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() - rhs); }
        inline friend enc_double   operator*   (enc_double lhs, double rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() * rhs); }
        inline friend enc_double   operator/   (enc_double lhs, double rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() / rhs); }
        inline friend enc_double   operator+   (enc_double lhs, enc_int rhs)           __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() + rhs.decrypt_int()); }
        inline friend enc_double   operator+   (enc_double lhs, enc_float rhs)         __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() + rhs.decrypt_float());} 
        inline friend enc_double   operator-   (enc_double lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() - rhs.decrypt_int()); }
        inline friend enc_double   operator*   (enc_double lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() * rhs.decrypt_int()); }
        inline friend enc_double   operator/   (enc_double lhs, enc_int rhs)            __attribute__((always_inline))  { return enc_double(lhs.decrypt_double() / rhs.decrypt_int()); }
        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_double   operator+   (double lhs, enc_double rhs)            __attribute__((always_inline))  { return enc_double(lhs + rhs.decrypt_double()); }
        inline friend enc_double   operator-   (double lhs, enc_double rhs)            __attribute__((always_inline))  { return enc_double(lhs - rhs.decrypt_double()); }
        inline friend enc_double   operator*   (double lhs, enc_double rhs)            __attribute__((always_inline))  { return enc_double(lhs * rhs.decrypt_double()); }
        inline friend enc_double   operator/   (double lhs, enc_double rhs)            __attribute__((always_inline))  { return enc_double(lhs / rhs.decrypt_double()); }
        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_double()) == (rhs.decrypt_double())));   }
        inline friend enc_bool     operator!=  (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_double()) != (rhs.decrypt_double())));   }
        inline friend enc_bool     operator<   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_double()) < (rhs.decrypt_double())));   }
        inline friend enc_bool     operator<=  (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_double()) > (rhs.decrypt_double())));  }
        inline friend enc_bool     operator>   (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_double()) < (lhs.decrypt_double()));     }
        inline friend enc_bool     operator>=  (enc_double lhs, enc_double rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_double()) < (rhs.decrypt_double())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_double()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_double()) == rhs));   }
        inline friend enc_bool     operator<   (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_double()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_double()) > rhs));    }
        inline friend enc_bool     operator>   (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_double()));       }
        inline friend enc_bool     operator>=  (enc_double lhs, double rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_double()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_double()));     }
        inline friend enc_bool     operator!=  (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_double())));  }
        inline friend enc_bool     operator<   (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_double())));    }
        inline friend enc_bool     operator<=  (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_double())));   }
        inline friend enc_bool     operator>   (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_double()) < lhs);      }
        inline friend enc_bool     operator>=  (double lhs, enc_double rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_double())));   }
       /*************/
//        /*** Math ***/
//         inline friend enc_double   sqrt   (enc_double x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_double(sqrt(x.decrypt_double())); }   //TODO Should implement d.o. sqrt function
//         inline friend enc_double   pow    (enc_double x, double y)                     __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_double(pow(x.decrypt_double(), y)); } //TODO Should implement d.o. pow function
//         inline friend enc_double   acos   (enc_double x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_double(acos(x.decrypt_double())); }   //TODO Should implement d.o. acos function
//         inline friend enc_double   cos    (enc_double x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_double(cos(x.decrypt_double())); }    //TODO Should implement d.o. cos function
//         inline friend enc_double   fabs   (enc_double x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_double(fabs(x.decrypt_double())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        double GET_DECRYPTED_VALUE(){ /*std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";*/ return this->decrypt_double(); }
        inline friend enc_double       CMOV(enc_bool cond, enc_double v_true, enc_double v_false) __attribute__((always_inline)) {
                double result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_double()), "r" (v_false.decrypt_double())
                    : "cc"
                );
                return enc_double(result);
        }
        /*** Type Casting ***/
        operator enc_long();
        operator enc_int()   {     return enc_int((int)(this->decrypt_double()));   }
        operator enc_float() {     return enc_float((double)(this->decrypt_double()));   }
        // operator enc_long          ();
        // operator bool           () const{  log("EXTERNAL CAST TO BOOL\n");           abort(); }
        // operator char           () const{  log("EXTERNAL CAST TO CHAR\n");           abort(); }
        // operator int            () const{  log("EXTERNAL CAST TO INT\n");            abort(); }
        // operator unsigned int   () const{  log("EXTERNAL CAST TO UNSIGNED INT\n");   abort(); }
        // operator long           () const{  log("EXTERNAL CAST TO LONG\n");           abort(); }
        // operator unsigned long  () const{  log("EXTERNAL CAST TO UNSIGNED LONG\n");  abort(); } 
        // operator double         () const{  log("EXTERNAL CAST TO DOUBLE\n");         abort(); }
        // operator float          () const{  log("EXTERNAL CAST TO FLOAT\n");          abort(); }
        /*************/
    } enc_double;
/*****************************************************************************/




/*****************************************************************************/
/*  · Encrypted Long
        Keyword: enc_long
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte double precision floating point (aka "double") value followed by 
        8 bytes (64 bits) of random salt
*/
    typedef struct enc_long{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_long()                    { long value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_long(long value)        { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_long(bit128_t value)      { ciphertext = value; }
        inline enc_long operator= (long i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        private: 
        long decrypt_long(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from long to uint64_t drops decimal values, producing an incorrect result)
                uint64_t value = decrypt_128_64(this->ciphertext);
                return *((long*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_long operator++() __attribute__((always_inline))  { 
                long value = this->decrypt_long();
                enc_long temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_long operator++(int) __attribute__((always_inline))  { 
                long value = this->decrypt_long();
                enc_long temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
        inline enc_long operator-() __attribute__((always_inline))  { 
                long value = this->decrypt_long();
                enc_long temp(-value); 
                return temp; 
        }
       /****************************/
       /*** One-Input Operations ***/
        inline enc_long operator+=(const long& rhs) __attribute__((always_inline))  { long temp = this->decrypt_long() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_long operator+=(const int& rhs)  __attribute__((always_inline))  { long temp = this->decrypt_long() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_long operator-=(const long& rhs) __attribute__((always_inline))  { long temp = this->decrypt_long() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_long   operator+   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() + rhs.decrypt_long()); }
        inline friend enc_long   operator-   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() - rhs.decrypt_long()); }
        inline friend enc_long   operator*   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() * rhs.decrypt_long()); }   
        inline friend enc_long   operator/   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() / rhs.decrypt_long()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_long   operator+   (enc_long lhs, long rhs)            __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() + rhs); }
        inline friend enc_long   operator-   (enc_long lhs, long rhs)            __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() - rhs); }
        inline friend enc_long   operator*   (enc_long lhs, long rhs)            __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() * rhs); }
        inline friend enc_long   operator/   (enc_long lhs, long rhs)            __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() / rhs); }
        inline friend enc_long   operator%   (enc_long lhs, long rhs)            __attribute__((always_inline))  { return enc_long(lhs.decrypt_long() % rhs); }
        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_long   operator+   (long lhs, enc_long rhs)            __attribute__((always_inline))  { return enc_long(lhs + rhs.decrypt_long()); }
        inline friend enc_long   operator-   (long lhs, enc_long rhs)            __attribute__((always_inline))  { return enc_long(lhs - rhs.decrypt_long()); }
        inline friend enc_long   operator*   (long lhs, enc_long rhs)            __attribute__((always_inline))  { return enc_long(lhs * rhs.decrypt_long()); }
        inline friend enc_long   operator/   (long lhs, enc_long rhs)            __attribute__((always_inline))  { return enc_long(lhs / rhs.decrypt_long()); }
        inline friend enc_long   operator%   (long lhs, enc_long rhs)            __attribute__((always_inline))  { return enc_long(lhs % rhs.decrypt_long()); }
        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_long()) == (rhs.decrypt_long())));   }
        inline friend enc_bool     operator!=  (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_long()) != (rhs.decrypt_long())));   }
        inline friend enc_bool     operator<   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_long()) < (rhs.decrypt_long())));   }
        inline friend enc_bool     operator<=  (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_long()) > (rhs.decrypt_long())));  }
        inline friend enc_bool     operator>   (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_long()) < (lhs.decrypt_long()));     }
        inline friend enc_bool     operator>=  (enc_long lhs, enc_long rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_long()) < (rhs.decrypt_long())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_long()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_long()) == rhs));   }
        inline friend enc_bool     operator<   (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_long()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_long()) > rhs));    }
        inline friend enc_bool     operator>   (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_long()));       }
        inline friend enc_bool     operator>=  (enc_long lhs, long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_long()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_long()));     }
        inline friend enc_bool     operator!=  (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_long())));  }
        inline friend enc_bool     operator<   (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_long())));    }
        inline friend enc_bool     operator<=  (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_long())));   }
        inline friend enc_bool     operator>   (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_long()) < lhs);      }
        inline friend enc_bool     operator>=  (long lhs, enc_long rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_long())));   }
        // // Bitwise, Enc-Plaintext Op
        inline friend enc_bool     operator>   (enc_long lhs, enc_double rhs)      __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_long()) > rhs.decrypt_double());      }
        inline friend enc_long     operator>>  (enc_long lhs, int rhs)             __attribute__((always_inline))   {  return enc_long(lhs.decrypt_long() >> rhs);  }
       /*************/
       /*** Math ***/
        // inline friend enc_long   sqrt   (enc_long x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_long(sqrt(x.decrypt_long())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_long   pow    (enc_long x, long y)                     __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_long(pow(x.decrypt_long(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_long   acos   (enc_long x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_long(acos(x.decrypt_long())); }   //TODO Should implement d.o. acos function
        // inline friend enc_long   cos    (enc_long x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_long(cos(x.decrypt_long())); }    //TODO Should implement d.o. cos function
        // inline friend enc_long   fabs   (enc_long x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_long(fabs(x.decrypt_long())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        long GET_DECRYPTED_VALUE(){ /*std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";*/ return this->decrypt_long(); }
        inline friend enc_long       CMOV(enc_bool cond, enc_long v_true, enc_long v_false) __attribute__((always_inline)) {
                long result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_long()), "r" (v_false.decrypt_long())
                    : "cc"
                );
                return enc_long(result);
        }

        /*** Type Casting ***/
        operator enc_double        () {     return enc_double((double)(this->decrypt_long()));   }
        // operator bool           () const{  log("EXTERNAL CAST TO BOOL\n");           abort(); }
        // operator char           () const{  log("EXTERNAL CAST TO CHAR\n");           abort(); }
        // operator int            () const{  log("EXTERNAL CAST TO INT\n");            abort(); }
        // operator unsigned int   () const{  log("EXTERNAL CAST TO UNSIGNED INT\n");   abort(); }
        // operator long           () const{  log("EXTERNAL CAST TO LONG\n");           abort(); }
        // operator unsigned long  () const{  log("EXTERNAL CAST TO UNSIGNED LONG\n");  abort(); } 
        // operator double         () const{  log("EXTERNAL CAST TO DOUBLE\n");         abort(); }
        // operator float          () const{  log("EXTERNAL CAST TO FLOAT\n");          abort(); }
        /*************/
    
    } enc_long;
/*****************************************************************************/


/*****************************************************************************/
/*  · Encrypted Unsinged Long 
        Keyword: enc_ulong
        A 16 byte (128-bit) ciphertext containing: 
        An 8 byte double precision floating point (aka "double") value followed by 
        8 bytes (64 bits) of random salt
*/
    typedef struct enc_ulong{
       /*******************************/
        bit128_t ciphertext; 
       /*******************************/
       /*** Constructors & Equality ***/
        enc_ulong()                    { unsigned long value = 0; ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_ulong(unsigned long value)        { ciphertext = encrypt_64_128(((uint64_t*) &value)); }
        enc_ulong(bit128_t value)      { ciphertext = value; }
        inline enc_ulong operator= (unsigned long i1) __attribute__((always_inline)) { ciphertext = encrypt_64_128(((uint64_t*) &i1)); return *this; } //trying fix for pointer cast
        // private: 
        unsigned long decrypt_ulong(){
                // More complex decryption function that implements pointer casts rather than static casts 
                // (A static cast from long to uint64_t drops decimal values, producing an incorrect result)
                uint64_t value = decrypt_128_64(this->ciphertext);
                return *((unsigned long*) &value);
        }
        public:
       /*****************************/
       /*** Zero-Input Operations ***/
        inline enc_ulong operator++() __attribute__((always_inline))  { 
                unsigned long value = this->decrypt_ulong();
                enc_ulong temp(++value); // Pre-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        }
        inline enc_ulong operator++(int) __attribute__((always_inline))  { 
                unsigned long value = this->decrypt_ulong();
                enc_ulong temp(value++); // Post-Increment
                this->ciphertext = encrypt_64_128((uint64_t*) &value);
                return temp; 
        } 
        inline enc_ulong operator-() __attribute__((always_inline))  { 
                unsigned long value = this->decrypt_ulong();
                enc_ulong temp(-value); 
                return temp; 
        }
       /****************************/
       /*** One-Input Operations ***/
        inline enc_ulong operator+=(const unsigned long& rhs) __attribute__((always_inline))  { unsigned long temp = this->decrypt_ulong() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_ulong operator+=(const int& rhs)  __attribute__((always_inline))           { unsigned long temp = this->decrypt_ulong() + rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
        inline enc_ulong operator-=(const unsigned long& rhs) __attribute__((always_inline))  { unsigned long temp = this->decrypt_ulong() - rhs; this->ciphertext = encrypt_64_128((uint64_t*) &temp); return *this; }
       /****************************/
       /*** Two-Input Operations ***/
        // Arithmetic, Enc-Enc Op
        inline friend enc_ulong   operator+   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() + rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator-   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() - rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator*   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() * rhs.decrypt_ulong()); }   
        inline friend enc_ulong   operator/   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() / rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator%   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() % rhs.decrypt_ulong()); }
        // Arithmetic, Enc-Plaintext Op
        inline friend enc_ulong   operator+   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() + rhs); }
        inline friend enc_ulong   operator-   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() - rhs); }
        inline friend enc_ulong   operator*   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() * rhs); }
        inline friend enc_ulong   operator/   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() / rhs); }
        inline friend enc_ulong   operator%   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() % rhs); }

        inline friend enc_ulong   operator&   (enc_ulong lhs, unsigned int rhs)            __attribute__((always_inline))  { return enc_ulong(lhs.decrypt_ulong() & rhs); }

        // // Arithmetic, Plaintext-Enc Op
        inline friend enc_ulong   operator+   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  { return enc_ulong(lhs + rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator-   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  { return enc_ulong(lhs - rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator*   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  { return enc_ulong(lhs * rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator/   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  { return enc_ulong(lhs / rhs.decrypt_ulong()); }
        inline friend enc_ulong   operator%   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  { return enc_ulong(lhs % rhs.decrypt_ulong()); }

        // // Logic, Enc-Enc Op          
        inline friend enc_bool     operator==  (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_ulong()) == (rhs.decrypt_ulong())));   }
        inline friend enc_bool     operator!=  (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_ulong()) != (rhs.decrypt_ulong())));   }
        inline friend enc_bool     operator<   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_ulong()) < (rhs.decrypt_ulong())));   }
        inline friend enc_bool     operator<=  (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_ulong()) > (rhs.decrypt_ulong())));  }
        inline friend enc_bool     operator>   (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_ulong()) < (lhs.decrypt_ulong()));     }
        inline friend enc_bool     operator>=  (enc_ulong lhs, enc_ulong rhs)        __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_ulong()) < (rhs.decrypt_ulong())));  }
        // // Logic, Enc-Plaintext Op
        inline friend enc_bool     operator==  (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_ulong()) == rhs);      }
        inline friend enc_bool     operator!=  (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_ulong()) == rhs));   }
        inline friend enc_bool     operator<   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool(((lhs.decrypt_ulong()) < rhs));     }
        inline friend enc_bool     operator<=  (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_ulong()) > rhs));    }
        inline friend enc_bool     operator>   (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool(rhs < (lhs.decrypt_ulong()));       }
        inline friend enc_bool     operator>=  (enc_ulong lhs, unsigned long rhs)            __attribute__((always_inline))  {  return enc_bool(!((lhs.decrypt_ulong()) < rhs));    }
        // // Logic, Plaintext-Enc Op
        inline friend enc_bool     operator==  (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool(lhs == (rhs.decrypt_ulong()));     }
        inline friend enc_bool     operator!=  (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs == (rhs.decrypt_ulong())));  }
        inline friend enc_bool     operator<   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool((lhs < (rhs.decrypt_ulong())));    }
        inline friend enc_bool     operator<=  (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs > (rhs.decrypt_ulong())));   }
        inline friend enc_bool     operator>   (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool((rhs.decrypt_ulong()) < lhs);      }
        inline friend enc_bool     operator>=  (unsigned long lhs, enc_ulong rhs)            __attribute__((always_inline))  {  return enc_bool(!(lhs < (rhs.decrypt_ulong())));   }
        // // Bitwise, Enc-Plaintext Op
        inline friend enc_bool     operator>   (enc_ulong lhs, enc_double rhs)      __attribute__((always_inline))  {  return enc_bool((lhs.decrypt_ulong()) > rhs.decrypt_double());      }
        inline friend enc_ulong    operator>>  (enc_ulong lhs, int rhs)             __attribute__((always_inline))   {  return enc_ulong(lhs.decrypt_ulong() >> rhs);  }
        inline friend enc_ulong    operator<<  (enc_ulong lhs, int rhs)             __attribute__((always_inline))   {  return enc_ulong(lhs.decrypt_ulong() << rhs);  }
        inline friend enc_ulong    operator|   (enc_ulong lhs, enc_ulong rhs)       __attribute__((always_inline))   {  return enc_ulong(lhs.decrypt_ulong() | rhs.decrypt_ulong());  }

       /*************/
       /*** Math ***/
        // inline friend enc_ulong   sqrt   (enc_ulong x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_ulong(sqrt(x.decrypt_ulong())); }   //TODO Should implement d.o. sqrt function
        // inline friend enc_ulong   pow    (enc_ulong x, long y)                     __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_ulong(pow(x.decrypt_ulong(), y)); } //TODO Should implement d.o. pow function
        // inline friend enc_ulong   acos   (enc_ulong x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_ulong(acos(x.decrypt_ulong())); }   //TODO Should implement d.o. acos function
        // inline friend enc_ulong   cos    (enc_ulong x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_ulong(cos(x.decrypt_ulong())); }    //TODO Should implement d.o. cos function
        // inline friend enc_ulong   fabs   (enc_ulong x)                               __attribute__((always_inline)) {std::cerr << "EXTERNAL CALL TO UNSAFE MATH FUNCTION\n"; return enc_ulong(fabs(x.decrypt_ulong())); }   //TODO Should implement d.o. fabs function
       /**************/
       /*** Other ***/
        unsigned long GET_DECRYPTED_VALUE(){ /*std::cerr << "EXTERNAL CALL TO UNSAFE DECRYPT FUNCTION\n";*/ return this->decrypt_ulong(); }
        inline friend enc_ulong       CMOV(enc_bool cond, enc_ulong v_true, enc_ulong v_false) __attribute__((always_inline)) {
                unsigned long result;
                __asm__ __volatile__ (
                    "test   %1, %1;"
                    "cmovz  %3, %0;"  //zero flag cleared?
                    "cmovnz %2, %0;" //zero flag not cleared?
                    : "=r" (result)
                    : "r" (cond.GET_DECRYPTED_VALUE()), "r" (v_true.decrypt_ulong()), "r" (v_false.decrypt_ulong())
                    : "cc"
                );
                return enc_ulong(result);
        }

        /*** Type Casting ***/
        operator enc_double        () {     return enc_double((double)(this->decrypt_ulong()));   }
        operator enc_uint32        () {     return enc_uint32((unsigned int)(this->decrypt_ulong()));   }
        // operator bool           () const{  log("EXTERNAL CAST TO BOOL\n");           abort(); }
        // operator char           () const{  log("EXTERNAL CAST TO CHAR\n");           abort(); }
        // operator int            () const{  log("EXTERNAL CAST TO INT\n");            abort(); }
        // operator unsigned int   () const{  log("EXTERNAL CAST TO UNSIGNED INT\n");   abort(); }
        // operator long           () const{  log("EXTERNAL CAST TO LONG\n");           abort(); }
        // operator unsigned long  () const{  log("EXTERNAL CAST TO UNSIGNED LONG\n");  abort(); } 
        // operator double         () const{  log("EXTERNAL CAST TO DOUBLE\n");         abort(); }
        // operator float          () const{  log("EXTERNAL CAST TO FLOAT\n");          abort(); }
        /*************/
    
    } enc_ulong;
/*****************************************************************************/


/************************************************************************************/
  


// Derived C++ Data Types
/************************************************************************************/  
/*  · Array
        An ordered collection of items stored at adjacent memory locations. 
        All array elements have the same data type. Array elements are accessed
        by their index, however C/C++ does not support out of bounds checking.

/*  + String: NEW!

/*  - Function: NOT SUPPORTED
        A block of code that performs some specific task. 

/*  - Pointer: NOT SUPPORTED 
        A symbolic representation of memory addresses that enables programs to
        simulate call-by-reference and manipulate dynamic data structures. 

/*  - Reference: NOT SUPPORTED 
        An alternative name for an existing variable. A variable is is declared
        as a reference by putting '&' in the declaration.       
/************************************************************************************/


// User-Defined C++ Data Types  
/************************************************************************************/
/*  - Class: NOT SUPPORTED
        A user-defined data type that holds its own data members and member functions,
        which are accessed and used via creating an instance of the class.
        (Note: You could make a class with encrypted data members)

/*  - Structure: NOT SUPPORTED
        A structure is used to group items of possibly different types into a single type.
        (Note: You could make a struct with encrypted items)

/*  - Union: NOT SUPPORTED
        A structure where all members share the same memory location.

/*  · Enumeration: 
        A user-defined data type in C that assigns names to integral constants, making 
        program easier to read and maintain.
/************************************************************************************/



}
#endif
