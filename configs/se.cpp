#ifndef SE_CPP
#define SE_CPP

#include "se.h"

void  SEInit(){
    setParameters(SymmCipher::AES128, 0x0505050505050505, 0x0505050505050505, 3022359314);
}

#endif
