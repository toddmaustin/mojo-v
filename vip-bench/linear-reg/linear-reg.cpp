#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <math.h>
using namespace std;

#include "../config.h"
#include <vector>

void create_projected_vector(int N, int size, int min, int max, int copies, VIP_ENCINT* v) 
{
        VIP_ENCINT value = myrand() % max + min;        
        for (int i = 0; i < size; i++) {
            for (int c = 0; c < copies; c++) {
            v[i+c*size] = value;
            }
        }        
}

void create_vector(int N, int size, int min, int max, int copies, VIP_ENCINT* v)
{
    for (int i = 0; i < size; i++)
    {
        VIP_ENCINT value = myrand() % max + min;
        for (int c = 0; c < copies; c++)
        {
            v[i + c * size] = value;
        }
    }        
}

int
main(void) 
{ 
  VIP_INIT; 
  mysrand(42);
  size_t poly_modulus_degree = 8192;

    /*** Initialize data vectors, similar to HE ***/
    /**********************************************/
    VIP_ENCINT v0[poly_modulus_degree], v1[poly_modulus_degree], v2[poly_modulus_degree], v3[poly_modulus_degree];
    create_vector(poly_modulus_degree, poly_modulus_degree, 0, 4, 1, v0);           //[Mod!] Init by passing by reference
    create_vector(poly_modulus_degree, poly_modulus_degree, 0, 4, 1, v1);           //[Mod!] Init by passing byreference
    create_projected_vector(poly_modulus_degree, poly_modulus_degree, 0, 4, 1, v2); //[Mod!] Init by passing by reference
    create_projected_vector(poly_modulus_degree, poly_modulus_degree, 0, 4, 1, v3); //[Mod!] Init by passing by reference         
    VIP_VEC_ENCINT c0(v0, poly_modulus_degree), c1(v1, poly_modulus_degree), c2(v2, poly_modulus_degree), c3(v3, poly_modulus_degree);
    VIP_VEC_ENCINT c4, c5, c_result;

    /*** Benchmark ***/
    /*****************/
    {
        Stopwatch s("Linear Regression");
        c4 = c2 * c0;
        c5 = c1 - c4;
        c_result = c5 - c3;
    }

    /*** Print results, "Native" code now removed ***/
    /************************************************/
    // std::cout<<c_result;
    // realRes=(std::vector<int64_t>)c_result;
    int64_t N=poly_modulus_degree;
    // std::vector<int64_t> golden(N, 0);
    // int64_t m = v2[0];
    // int64_t b = v3[0];            
    // {
    //     Stopwatch s("Native Linear Regression");
    //     for(int iter=0;iter<iterations;iter++){
    //         for (int i = 0; i < N; i++) {
    //             golden[i] = v1[i] - (m * v0[i] + b);
    //         }
    //     }
    // }            

    // bool correct = true;
    for (int i = 0; i < N; i++) {
        // correct &= (golden[i] == realRes[i]);
        // if (!correct && i < 5){
            std::cout << i << " golden result " << VIP_DEC(c_result[i]) << " " << /*realRes[i] <<*/ std::endl;
        // }
    }
    // std::cout<<"Correct "<<correct<<std::endl;

    return 0;
}
