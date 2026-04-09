#ifndef SEAL_WRAPPER_H
#define SEAL_WRAPPER_H

#include <cstddef>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <memory>
#include <limits>
#include <algorithm>
#include <numeric>
#include <valarray>

#include "seal.h" 

namespace he_lib
{
    
class SEALCipherText{
    
    private:
        bool batchingEnabled;

    public:
        seal::Ciphertext cipher;
        
        static seal::scheme_type scheme;
        static std::shared_ptr<seal::SEALContext> context;
        static seal::Evaluator* evaluator;
        static seal::Encryptor* encryptor;
        static seal::Decryptor* decryptor;
        static seal::PublicKey* public_key;
        static seal::SecretKey* secret_key;
        static seal::RelinKeys* relin_keys;
        static seal::GaloisKeys* galois_keys;
        static double scale;
        static seal::IntegerEncoder* intEncoder;
        static seal::BatchEncoder* batchEncoder;
        static seal::CKKSEncoder*  ckksEncoder; 

        SEALCipherText();
        
        SEALCipherText(int i1);
        
        SEALCipherText(std::vector<int64_t>& i1);

        SEALCipherText(std::vector<uint64_t>& i1);

        SEALCipherText(double d1);
        
        SEALCipherText(std::vector<double>& d1);


        SEALCipherText(int i1, int size);
        
        SEALCipherText(std::vector<int64_t>& i1, int size);

        SEALCipherText(std::vector<uint64_t>& i1, int size);

        SEALCipherText(double d1, int size);
        
        SEALCipherText(std::vector<double>& d1, int size);
                
        SEALCipherText(seal::Plaintext plain);
        
        SEALCipherText(SEALCipherText &c1);
        
        ~SEALCipherText();

        bool isBatchingEnabled();
        void initFromVector(std::vector<int>& i1);

        operator SEALCipherText();

        //SEALCipherText& operator = (SEALCipherText &c1);
        std::valarray<int> decrypt_bfv(); 
        operator std::vector<int64_t> ();
        operator std::vector<double> ();
        std::valarray<double> decrypt_ckks(); 
        //friend std::vector<double>& operator = (SEALCipherText &c1);
        //friend std::vector<int64_t>& operator = (SEALCipherText &c1);
       
        SEALCipherText& operator += (SEALCipherText &c1);
        
        SEALCipherText& operator += (seal::Plaintext &p1);

        SEALCipherText& operator += (int p1);

        SEALCipherText& operator += (double p1);
        
        SEALCipherText& operator -= (SEALCipherText &c1);
        
        SEALCipherText& operator -= (seal::Plaintext &p1);

        SEALCipherText& operator -= (int p1);

        SEALCipherText& operator -= (double p1);
       
        SEALCipherText& operator *= (SEALCipherText &c1);
        
        SEALCipherText& operator *= (seal::Plaintext &p1);

        SEALCipherText& operator *= (int p1);

        SEALCipherText& operator *= (double p1);
        
        SEALCipherText& operator -();
        
        SEALCipherText& operator -(SEALCipherText &c1);
        
        SEALCipherText& operator -(seal::Plaintext &p1);
        
        SEALCipherText& operator -(double d1);

        SEALCipherText& operator -(std::vector<double> d1);
        
        friend SEALCipherText& operator -(seal::Plaintext &p1, SEALCipherText &c1);
        
        friend SEALCipherText& operator -(double d1, SEALCipherText &c1);
        
        SEALCipherText& operator +(SEALCipherText &c1);
                
        SEALCipherText& operator +(seal::Plaintext &p1);
        
        SEALCipherText& operator +(double d1);

        SEALCipherText& operator +(std::vector<int64_t> d1);
        
        SEALCipherText& operator +(std::vector<double> d1);
        
        friend SEALCipherText& operator +(seal::Plaintext &p1, SEALCipherText &c1);
        
        friend SEALCipherText& operator +(double d1, SEALCipherText &c1);
        
        SEALCipherText& operator *(SEALCipherText &c1);
        
        SEALCipherText& operator *(seal::Plaintext &p1);

        SEALCipherText& operator *(int i1);

        SEALCipherText& operator *(double d1);

        SEALCipherText& operator *(std::vector<int64_t> d1);
        
        SEALCipherText& operator *(std::vector<double> d1);

        void operator <<=(int i1);

        void operator >>=(int i1);

        SEALCipherText& operator <<(int i1);

        SEALCipherText& operator >>(int i1);

        friend SEALCipherText& operator ^(SEALCipherText c1,int i1);
        
        friend SEALCipherText& operator ^(SEALCipherText c1,double i1);
        
        friend void operator <<(seal::Plaintext p1,int i1);

        friend void operator >>(seal::Plaintext p1,int i1);


        
        friend SEALCipherText& operator *(seal::Plaintext &p1, SEALCipherText &c1);

        //friend SEALCipherText& operator *(std::vector<int64_t> i1, SEALCipherText &c1);
        //friend SEALCipherText& operator *(std::vector<double> p1, SEALCipherText &c1);
        friend SEALCipherText& operator *(seal::Plaintext &p1, SEALCipherText &c1);
        
        friend SEALCipherText& operator *(int i1, SEALCipherText &c1);
        
        friend SEALCipherText& operator *(double d1, SEALCipherText &c1);
        

        friend std::ostream& operator <<(std::ostream &out, SEALCipherText &c1);
       
        int save(std::string fileName); //returns the size of the ciphertext saved



};

}
inline void print_parameterz(std::shared_ptr<seal::SEALContext> context);

#endif