#ifndef SEAL_CPP
#define SEAL_CPP

#include "seal-3.4.h"

using namespace seal; 
using namespace he_lib;


void SEAL_BFV_Init()
{
    SEALCipherText::scheme=scheme_type::BFV;
    EncryptionParameters parms(SEALCipherText::scheme);
   
        size_t poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree,20));
        auto context = SEALContext::Create(parms);
        
        SEALCipherText::context=context;
        KeyGenerator keygen(context);

        SEALCipherText::public_key = new seal::PublicKey();
        SEALCipherText::secret_key = new seal::SecretKey();
        SEALCipherText::relin_keys = new seal::RelinKeys();
        SEALCipherText::galois_keys = new seal::GaloisKeys();
        *SEALCipherText::public_key=keygen.public_key();
        *SEALCipherText::secret_key=keygen.secret_key();
        *SEALCipherText::relin_keys=keygen.relin_keys();
        *SEALCipherText::galois_keys=keygen.galois_keys();

        SEALCipherText::evaluator=new Evaluator(context);
        SEALCipherText::decryptor=new Decryptor(context, *SEALCipherText::secret_key);
        SEALCipherText::encryptor=new Encryptor(context, *SEALCipherText::public_key);
        SEALCipherText::intEncoder=new IntegerEncoder(context);
        SEALCipherText::batchEncoder=new BatchEncoder(context);    
}


void  SEAL_CKKS_Init(){
        int scaleBits=40;
        SEALCipherText::scheme=scheme_type::CKKS;
        EncryptionParameters parms(SEALCipherText::scheme);
        size_t poly_modulus_degree = 16384;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        auto coefMod=CoeffModulus::Create(poly_modulus_degree,{60,scaleBits,scaleBits,60});
        parms.set_coeff_modulus(coefMod);

        auto context = SEALContext::Create(parms);
        SEALCipherText::context=context;

        KeyGenerator keygen(SEALCipherText::context);
            
        SEALCipherText::public_key = new seal::PublicKey();
        SEALCipherText::secret_key = new seal::SecretKey();
        SEALCipherText::relin_keys = new seal::RelinKeys();
        SEALCipherText::galois_keys = new seal::GaloisKeys();
        *SEALCipherText::public_key=keygen.public_key();
        *SEALCipherText::secret_key=keygen.secret_key();
        *SEALCipherText::relin_keys=keygen.relin_keys();
        *SEALCipherText::galois_keys=keygen.galois_keys();

        SEALCipherText::evaluator=new Evaluator(context);
        SEALCipherText::decryptor=new Decryptor(context, *SEALCipherText::secret_key);
        SEALCipherText::encryptor=new Encryptor(context, *SEALCipherText::public_key);
        SEALCipherText::scale=pow(2,scaleBits);
        SEALCipherText::ckksEncoder=new CKKSEncoder(context);

        if(SEALCipherText::encryptor==NULL)
            std::cout<<"Failed to allocate space for the encrypter"<<std::endl;
}

#endif



