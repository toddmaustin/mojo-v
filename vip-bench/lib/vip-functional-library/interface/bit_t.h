#ifndef _UINT128_T_H
#define _UINT128_T_H
#include <stdlib.h> 
#include <bitset>
#include <string.h>
#include <iostream>

struct bit128_t{

public: //SHOULD BE PRIVATE
    uint8_t value[16]; 
    /* THE bit128_t class stores values according to the following structure: 
    *
    * value[16] = [MSB] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [xxx] [LSB]
    *               0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15                                                                                                                   
    *             [------------------UPPER 64b------------------] [------------------LOWER 64b-------------------] 
    * 
    * FOR 64-bit values, like INT
    * value[16] = [INT] [INT] [INT] [INT] [INT] [INT] [INT] [INT] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad]
    *               0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15                                                                                                                   
    *             [------------------UPPER 64b------------------] [------------------LOWER 64b-------------------] 
    * 
    * FOR 8-bit values, like BOOL/CHAR
    * value[16] = [B/C] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad] [pad]
    *               0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15                                                                                                                   
    *             [------------------UPPER 64b------------------] [------------------LOWER 64b-------------------] 
    *             [---]
    *               ^ UPPER 8b
    */

public:
    /******** CONSTRUCTORS ********/
    bit128_t(){
        for(int i=0; i<16; i++){
            value[i] = 0x00; //0000 0000
        } 
    }
    bit128_t(uint64_t upper_value, uint64_t lower_pad){
        init(upper_value, lower_pad);
    }
    bit128_t(uint8_t upper_value, uint8_t* lower_pad){
        /* Store upper_value in MSB/Index 0 */
        value[0]= upper_value;
        /* Store lower_pad in 8-bit segments */
        for(int i=15; i>=1 ; i--){
            value[i] = lower_pad[i-1];
        } 
    }
    bit128_t(uint8_t* full_value){
        /* Store full_value in 8-bit segments */
        for(int i=0; i<16; i++){
            value[i] = full_value[i];
        } 
    }
    void init(uint64_t upper_value, uint64_t lower_pad){
        /* Break and store upper_value in 8-bit segments */
        for(int i=7; i>=0; i--){
            value[i] = upper_value & 0xFF ; 
            upper_value = upper_value >> 8;
        }
        /* Break and store lower_pad in 8-bit segments */
        for(int i=15; i>=8; i--){
            value[i] = lower_pad & 0xFF;
            lower_pad = lower_pad >> 8;
        } 
    }

    /******** VALUE ACCESS FUNCTIONS ********/
    uint64_t getUpperValue_64b(){
        uint64_t ret = 0;
        for(int i=7; i>=0; i--){
            ret = ret + ((uint64_t)value[8-i-1]);
            if(i != 0){ret = ret << 8;} 
        }
        return ret;        
    }

    uint32_t getUpperValue_32b(){
        uint32_t ret = 0;
        for(int i=4; i>=0; i--){
            ret = ret + ((uint32_t)value[8-i-1]);
            if(i != 0){ret = ret << 8;} 
        }
        return ret;        
    }
    uint32_t getMidValue_32b(){
        uint32_t ret = 0;
        for(int i=7; i>=4; i--){
            ret = ret + ((uint32_t)value[8-i-1]);
            if(i != 0){ret = ret << 8;} 
        }
        return ret;        
    }

    uint64_t getUpperValue_8b(){
        return value[0];
    }


    uint64_t getLowerValue_64b(){
        uint64_t ret = 0;
        for(int i=15; i>=8; i--){
            ret = ret + ((uint64_t)value[16-(i-7)]);
            if(i != 8){ret = ret << 8;} 
        }
        return ret;      
    }

};

#endif
