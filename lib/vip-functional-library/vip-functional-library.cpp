#ifndef VIP_CPP
#define VIP_CPP

#include "vip-functional-library.h"

void  VIPInit(){
    setParameters(SymmCipher::AES128, 0x0505050505050505, 0x0505050505050505, 3022359314);
}

#endif
