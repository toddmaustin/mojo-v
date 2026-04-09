#ifndef SEAL_WRAPPER_CPP
#define SEAL_WRAPPER_CPP

#include "SEALWrapper.h"

using namespace he_lib;
using namespace seal;

        seal::scheme_type  SEALCipherText::scheme;
        std::shared_ptr<SEALContext>  SEALCipherText::context;
        seal::Evaluator*  SEALCipherText::evaluator;
        seal::Encryptor*  SEALCipherText::encryptor;
        seal::Decryptor*  SEALCipherText::decryptor;
        seal::PublicKey*  SEALCipherText::public_key;
        seal::SecretKey*  SEALCipherText::secret_key;
        seal::RelinKeys*  SEALCipherText::relin_keys;
        seal::GaloisKeys*  SEALCipherText::galois_keys;
        double  SEALCipherText::scale=1; 
        seal::IntegerEncoder* SEALCipherText::intEncoder;
        seal::BatchEncoder* SEALCipherText::batchEncoder;
        seal::CKKSEncoder*  SEALCipherText::ckksEncoder; 

        SEALCipherText::SEALCipherText()
        {
            if(batchEncoder!=NULL)
                {
                //     printf("Batching enabled!\n ");
                    batchingEnabled=true;
                }
                else
                batchingEnabled=false;
        }
        SEALCipherText::SEALCipherText(int i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                batchingEnabled=false;
                if(intEncoder!=NULL)
                    p1=intEncoder->encode(i1);
                else
                    {
                        std::cout<<"Integer Encoder not instantiated!"<<std::endl;
                        exit(1);
                    }
            }
            else
            {        
                if (ckksEncoder!=NULL)
                    ckksEncoder->encode((double)i1,scale,p1);
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            encryptor->encrypt(p1,cipher);
        }
        SEALCipherText::SEALCipherText(double d1)
        {
            Plaintext p1;
            print_parameterz(SEALCipherText::context);
            if(scheme==scheme_type::BFV){
                batchingEnabled=false;
                if(intEncoder!=NULL)
                    p1=intEncoder->encode((int)d1);
                else
                {
                    std::cout<<"Integer Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
                
            }
            else{        
                
                if(ckksEncoder!=NULL){
                    ckksEncoder->encode(d1,scale,p1);
                    std::cout<<"Number of slots:"<<ckksEncoder->slot_count()<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            {
                //Stopwatch s("Encrypt");
                //std::cout<<"Secrekey Length:"<<secret_key.save_size()<<std::endl;
                encryptor->encrypt(p1,cipher);
            }
        }
        SEALCipherText::SEALCipherText(std::vector<int64_t>& i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                
                if(batchEncoder!=NULL)
                {
                    //printf("Batching enabled!\n ");
                    batchingEnabled=true;
                    batchEncoder->encode(i1,p1);
                    encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"Batch Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            else
            {
                printf("Creating a batch unsuccessful!\n");
            }
            
        }

        SEALCipherText::SEALCipherText(std::vector<uint64_t>& i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                
                if(batchEncoder!=NULL)
                {
                    //printf("Batching enabled!\n ");
                    batchingEnabled=true;
                    batchEncoder->encode(i1,p1);
                    encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"Batch Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            else
            {
                printf("Creating a batch unsuccessful!\n");
            }
            
        }
        
        SEALCipherText::SEALCipherText(std::vector<double>& d1)
        {
            if(scheme==scheme_type::CKKS){
                Plaintext p1;
                if(ckksEncoder!=NULL){
                        ckksEncoder->encode(d1,scale,p1);
                        encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }  
            }
            else
            {
                std::cout<<"Scheme type has to be CKKS."<<std::endl;
                exit(1);
            }
            
                     
        }
        SEALCipherText::SEALCipherText(int i1, int size=0)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                batchingEnabled=false;
                if(intEncoder!=NULL)
                    p1=intEncoder->encode(i1);
                else
                    {
                        std::cout<<"Integer Encoder not instantiated!"<<std::endl;
                        exit(1);
                    }
            }
            else
            {        
                if (ckksEncoder!=NULL)
                    ckksEncoder->encode((double)i1,scale,p1);
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            encryptor->encrypt(p1,cipher);
        }
        SEALCipherText::SEALCipherText(double d1, int size=0)
        {
            Plaintext p1;
            print_parameterz(SEALCipherText::context);
            if(scheme==scheme_type::BFV){
                batchingEnabled=false;
                if(intEncoder!=NULL)
                    p1=intEncoder->encode((int)d1);
                else
                {
                    std::cout<<"Integer Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
                
            }
            else{        
                
                if(ckksEncoder!=NULL){
                    ckksEncoder->encode(d1,scale,p1);
                    std::cout<<"Number of slots:"<<ckksEncoder->slot_count()<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            {
                //Stopwatch s("Encrypt");
                //std::cout<<"Secrekey Length:"<<secret_key.save_size()<<std::endl;
                encryptor->encrypt(p1,cipher);
            }
        }
        SEALCipherText::SEALCipherText(std::vector<int64_t>& i1, int size=0)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                
                if(batchEncoder!=NULL)
                {
                    //printf("Batching enabled!\n ");
                    batchingEnabled=true;
                    batchEncoder->encode(i1,p1);
                    encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"Batch Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            else
            {
                printf("Creating a batch unsuccessful!\n");
            }
            
        }

        SEALCipherText::SEALCipherText(std::vector<uint64_t>& i1, int size=0)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                
                if(batchEncoder!=NULL)
                {
                    //printf("Batching enabled!\n ");
                    batchingEnabled=true;
                    batchEncoder->encode(i1,p1);
                    encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"Batch Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            else
            {
                printf("Creating a batch unsuccessful!\n");
            }
            
        }
        
        SEALCipherText::SEALCipherText(std::vector<double>& d1, int size=0)
        {
            if(scheme==scheme_type::CKKS){
                Plaintext p1;
                if(ckksEncoder!=NULL){
                        ckksEncoder->encode(d1,scale,p1);
                        encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"CKKS Encoder not instantiated!"<<std::endl;
                    exit(1);
                }  
            }
            else
            {
                std::cout<<"Scheme type has to be CKKS."<<std::endl;
                exit(1);
            }
            
                     
        }
        
        SEALCipherText::SEALCipherText(Plaintext plain)
        {
            encryptor->encrypt(plain,cipher);
        }
        SEALCipherText::SEALCipherText(SEALCipherText &c1)
        {
            cipher=c1.cipher;
        }
        SEALCipherText::~SEALCipherText()
        {
                cipher.release();
        }
        bool SEALCipherText::isBatchingEnabled() {return batchingEnabled;}
        SEALCipherText::operator SEALCipherText() {return (*this);}


        void SEALCipherText::initFromVector(std::vector<int>& i0)
        {
            Plaintext p1;
            std::vector<int64_t> i1(i0.size());
            for(int i=0; i<i0.size(); i++){
                i1[i] = (int64_t)i0[i];
            }
            if(scheme==scheme_type::BFV){
                
                if(batchEncoder!=NULL)
                {
                    //printf("Batching enabled!\n ");
                    batchingEnabled=true;
                    batchEncoder->encode(i1,p1);
                    encryptor->encrypt(p1,cipher);
                }
                else
                {
                    std::cout<<"Batch Encoder not instantiated!"<<std::endl;
                    exit(1);
                }
            }
            else
            {
                printf("Creating a batch unsuccessful!\n");
            }
        }

       /* SEALCipherText& SEALCipherText::operator = (SEALCipherText &c1)
        {
            cipher=c1.cipher;
            return *this;
        }*/

        
        SEALCipherText& SEALCipherText::operator += (SEALCipherText &c1)
        {
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(c1.cipher,cipher.parms_id());
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(cipher,c1.cipher.parms_id());
            }
            evaluator->add_inplace(cipher,c1.cipher);
            return *this;
        }
        SEALCipherText& SEALCipherText::operator += (Plaintext &p1)
        {
            if(scheme==scheme_type::CKKS)
            {
                p1.scale()=cipher.scale();
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }
            evaluator->add_plain_inplace(cipher,p1);
            return *this;
        }


        std::valarray<int> SEALCipherText::decrypt_bfv() 
        {
            Plaintext result;
            std::vector<int64_t> realRes;   //printf("Debug 1\n");        
            SEALCipherText::decryptor->decrypt(cipher,result);
            //printf("Debug 2\n");
            if(scheme==seal::scheme_type::BFV)
            { 
                 int64_t val;
                if(isBatchingEnabled())
                {
                    //printf("Debug 2-1\n");
                    batchEncoder->decode(result,realRes);
                    val=realRes[0];
                }
                else
                {
                    //printf("Debug 2-2\n");
                    IntegerEncoder *encoder;
                    encoder=new IntegerEncoder(context);
                    val=encoder->decode_int32(result);
                    realRes.push_back(val);
                }
                
            }
             //printf("Debug 3\n");
             std::valarray<int> ret(realRes.size());
             std::copy(realRes.begin(), realRes.end(), std::begin(ret));  
             return ret;
        }

        SEALCipherText::operator std::vector<int64_t> () 
        {
            Plaintext result;
            std::vector<int64_t> realRes;   //printf("Debug 1\n");        
            SEALCipherText::decryptor->decrypt(cipher,result);
            //printf("Debug 2\n");
            if(scheme==seal::scheme_type::BFV)
            { 
                 int64_t val;
                if(isBatchingEnabled())
                {
                    //printf("Debug 2-1\n");
                    batchEncoder->decode(result,realRes);
                    val=realRes[0];
                }
                else
                {
                    //printf("Debug 2-2\n");
                    IntegerEncoder *encoder;
                    encoder=new IntegerEncoder(context);
                    val=encoder->decode_int32(result);
                    realRes.push_back(val);
                }
                
            }
             //printf("Debug 3\n");
             return realRes;
        }
       
        SEALCipherText::operator std::vector<double> ()
        {
            Plaintext result;
            std::vector<double> realRes;           
            SEALCipherText::decryptor->decrypt(cipher,result);
            
                if(ckksEncoder!=NULL)
                {
                    
                    ckksEncoder->decode(result,realRes);
                    
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated"<<std::endl;
                }
                return realRes;
             
             
        }

        std::valarray<double> SEALCipherText::decrypt_ckks()
        {
            Plaintext result;
            std::vector<double> realRes;           
            SEALCipherText::decryptor->decrypt(cipher,result);
            
                if(ckksEncoder!=NULL)
                {
                    
                    ckksEncoder->decode(result,realRes);
                    
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated"<<std::endl;
                }
            
            std::valarray<double> ret(realRes.size());
            std::copy(realRes.begin(), realRes.end(), std::begin(ret));  
            return ret;
        }

         SEALCipherText& SEALCipherText::operator += (int i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode(i1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode((double)i1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            evaluator->add_plain_inplace(cipher,p1);
            return *this;
        }
         SEALCipherText& SEALCipherText::operator += (double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            evaluator->add_plain_inplace(cipher,p1);
            return *this;
        }
        SEALCipherText& SEALCipherText::operator -= (SEALCipherText &c1)
        {
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(c1.cipher,cipher.parms_id());
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(cipher,c1.cipher.parms_id());
            }
            evaluator->sub_inplace(cipher,c1.cipher);
            return *this;
        }
        SEALCipherText& SEALCipherText::operator -= (Plaintext &p1)
        {
            if(scheme==scheme_type::CKKS)
            {
                p1.scale()=cipher.scale();
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }
            evaluator->sub_plain_inplace(cipher,p1);
            return *this;
        }

        SEALCipherText& SEALCipherText::operator -= (int i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode(i1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode((double)i1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            evaluator->sub_plain_inplace(cipher,p1);
            return *this;
        }

        SEALCipherText& SEALCipherText::operator -= (double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            evaluator->sub_plain_inplace(cipher,p1);
            return *this;
        }


        SEALCipherText& SEALCipherText::operator *= (SEALCipherText &c1)
        {
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(c1.cipher,cipher.parms_id());
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(cipher,c1.cipher.parms_id());
            }
            evaluator->multiply_inplace(cipher,c1.cipher);
            evaluator->relinearize_inplace(cipher,*relin_keys);
            if(scheme==scheme_type::CKKS)
            {  
                evaluator->rescale_to_next_inplace(cipher);
                cipher.scale()=scale;
             }
            return *this;
        }
        SEALCipherText& SEALCipherText::operator *= (Plaintext &p1)
        {
            if(p1.scale()!=cipher.scale()&&scheme==scheme_type::CKKS)
            {
                p1.scale()=cipher.scale();
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }
            evaluator->multiply_plain_inplace(cipher,p1);
            evaluator->relinearize_inplace(cipher,*relin_keys);
            if(scheme==scheme_type::CKKS)
             {
                    evaluator->rescale_to_next_inplace(cipher);
                    //cipher.scale()=scale;
             }
            return *this;
        }
        SEALCipherText& SEALCipherText::operator *= (int i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode(i1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode((double)i1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            
            evaluator->multiply_plain_inplace(cipher,p1);
            evaluator->relinearize_inplace(cipher,*relin_keys);
            if(scheme==scheme_type::CKKS)
             {
                    
                    evaluator->rescale_to_next_inplace(cipher);
                    //cipher.scale()=scale;
             }
            return *this;
        }
        SEALCipherText& SEALCipherText::operator *= (double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //p1.scale()=scale;
                    //cipher.scale()=scale;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            
            evaluator->multiply_plain_inplace(cipher,p1);
            evaluator->relinearize_inplace(cipher,*relin_keys);
            if(scheme==scheme_type::CKKS)
             {
                    
                    evaluator->rescale_to_next_inplace(cipher);
                    //cipher.scale()=scale;
             }
            return *this;
        }
        SEALCipherText& SEALCipherText::operator -()
        {
            //evaluator->negate_inplace(cipher);
            //return *this;
            SEALCipherText *res= new SEALCipherText();
            evaluator->negate(cipher,res->cipher);
           
            return *res;
        }
         SEALCipherText& SEALCipherText::operator -(SEALCipherText &c1)
        {
            //std::cout<<"*********operator - ************"<<std::endl;
           /*  SEALCipherText *res= new SEALCipherText();
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(c1.cipher,cipher.parms_id());
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(cipher,c1.cipher.parms_id());
            } */
            SEALCipherText *res= new SEALCipherText();
            bool isC1=false;
            //std::cout<<cipher.coeff_mod_count()<<"  "<<c1.cipher.coeff_mod_count()<<std::endl;
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                //cipher.scale()=SEALCipherText::scale;
                //c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count()){
                    *res=c1;
                    isC1=true;
                    evaluator->mod_switch_to_inplace(res->cipher,cipher.parms_id());
                }
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                {
                    *res=*this;
                    
                    evaluator->mod_switch_to_inplace(res->cipher,c1.cipher.parms_id());
                }
            }
            else
            {
                *res=*this;
            }
            SEALCipherText *tempPtr= isC1? this : &c1;
            //std::cout<<*res<<"("<<res->cipher.coeff_mod_count()<<") - "<<*temp<<"("<<temp->cipher.coeff_mod_count()<<")"<<std::endl;
            evaluator->sub_inplace(res->cipher,tempPtr->cipher);
            //std::cout<<*res<<"("<<res->cipher.coeff_mod_count()<<")"<<std::endl;
            //std::cout<<"*********operator - ends *******"<<std::endl;
            return *res;
        }
         SEALCipherText& SEALCipherText::operator -(Plaintext &p1)
        {
            if(scheme==scheme_type::CKKS)
            {
                p1.scale()=cipher.scale();
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->sub_plain(cipher,p1,res->cipher);
            return *res;
        }
         SEALCipherText& SEALCipherText::operator -(double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                
                    p1=intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //p1.scale()=cipher.scale();
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->sub_plain(cipher,p1,res->cipher);
            return *res;
        }
        SEALCipherText& operator -(Plaintext &p1, SEALCipherText &c1)
        {
            SEALCipherText *res= new SEALCipherText();
            if(c1.scheme==scheme_type::CKKS)
            {
                p1.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
            }
            c1.evaluator->negate_inplace(c1.cipher);
            c1.evaluator->add_plain(c1.cipher,p1,res->cipher);
            return *res;
        }
        SEALCipherText& operator -(double d1, SEALCipherText &c1)
        {
            //std::cout<<c1<<" "<<d1<<std::endl;

            Plaintext p1;
            if(c1.scheme==scheme_type::BFV){
                if(c1.intEncoder!=NULL){
                
                    p1=c1.intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(c1.ckksEncoder!=NULL)
                {
                    c1.ckksEncoder->encode(d1,c1.cipher.scale(),p1);
                    c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
                    //p1.scale()=c1.scale;
                    //c1.cipher.scale()=c1.scale;
                    // std::cout<<"C1 Sclae:"<<c1.cipher.scale()<<"P1 Sclae:"<<p1.scale()<<"System Sclae:"<<c1.scale<<std::endl;
                    // std::vector<double> pd1;
                    // c1.ckksEncoder->decode(p1,pd1);
                    // std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            //std::cout<<c1<<std::endl;
            //std::cout<<"D1: "<<d1<<" - "<<c1<<" = ";
            c1.evaluator->negate(c1.cipher,res->cipher);//std::cout<<*res<<std::endl;
            c1.evaluator->add_plain_inplace(res->cipher,p1);//std::cout<<*res<<std::endl;
             //std::cout<<"C1 Sclae:"<<c1.cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<c1.scale<<std::endl;
            //std::cout<<*res<<std::endl;
            return *res;
        }
        SEALCipherText& SEALCipherText::operator +(SEALCipherText &c1)
        {
            //std::cout<<"*********operator + ************"<<std::endl;
            /* if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                std::cout<<cipher.scale()<<" "<<c1.cipher.scale()<<" "<<c1.cipher.coeff_mod_count()
                        <<" "<<cipher.coeff_mod_count()<<std::endl;
                cipher.scale()=SEALCipherText::scale;
                c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(c1.cipher,cipher.parms_id());
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                    evaluator->mod_switch_to_inplace(cipher,c1.cipher.parms_id());
                    std::cout<<cipher.scale()<<" "<<c1.cipher.scale()<<" "<<c1.cipher.coeff_mod_count()
                        <<" "<<cipher.coeff_mod_count()<<std::endl;
            } */

            SEALCipherText *res= new SEALCipherText();
            bool isC1=false;
            //std::cout<<cipher.coeff_mod_count()<<"  "<<c1.cipher.coeff_mod_count()<<std::endl;
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                //cipher.scale()=SEALCipherText::scale;
                //c1.cipher.scale()=SEALCipherText::scale;
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count()){
                    *res=c1;
                    isC1=true;
                    evaluator->mod_switch_to_inplace(res->cipher,cipher.parms_id());
                    //std::cout<<"1. "<<c1.cipher.coeff_mod_count()<<"  "<<res->cipher.coeff_mod_count()<<std::endl;
                    //std::cout<<"1. "<<*this<<" <-> "<<cipher.scale()<<"  "<<*res<<" <-> "<<res->cipher.scale()<<" "<<c1<<" <-> "<<c1.cipher.scale()<<std::endl;
                    res->cipher.scale()=cipher.scale();
                    //std::cout<<"1. "<<*this<<" <-> "<<cipher.scale()<<"  "<<*res<<" <-> "<<res->cipher.scale()<<" "<<c1<<" <-> "<<c1.cipher.scale()<<std::endl;
                }
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                {
                    *res=*this;
                    evaluator->mod_switch_to_inplace(res->cipher,c1.cipher.parms_id());
                    //std::cout<<"2. "<<cipher.coeff_mod_count()<<"  "<<res->cipher.coeff_mod_count()<<std::endl;
                    //std::cout<<"2. "<<cipher.scale()<<"  "<<res->cipher.scale()<<std::endl;
                    res->cipher.scale()=c1.cipher.scale();
                }
            }
            else
            {
                *res=*this;
            }
            SEALCipherText *temp= isC1? this:&c1;
            //std::cout<<*res<<"("<<res->cipher.coeff_mod_count()<<") + "<<*temp<<"("<<temp->cipher.coeff_mod_count()<<")"<<std::endl;
            //std::cout<<*res<<"("<<res->cipher.coeff_mod_count()<<") + "<<*temp<<"("<<temp->cipher.coeff_mod_count()<<")"<<std::endl;
            evaluator->add_inplace(res->cipher,temp->cipher);
            //std::cout<<*res<<"("<<res->cipher.coeff_mod_count()<<")"<<std::endl;
            //std::cout<<"*********operator + ends *******"<<std::endl;
            return *res;
        }
         SEALCipherText& SEALCipherText::operator +(Plaintext &p1)
        {
            SEALCipherText *res= new SEALCipherText();
            if(scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                p1.scale()=SEALCipherText::scale;
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }
            evaluator->add_plain(cipher,p1,res->cipher);
            return *res;
        }
        SEALCipherText& SEALCipherText::operator +(double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                    p1=intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //p1.scale()=scale;
                    //cipher.scale()=scale;
                    //std::cout<<"C1 Sclae:"<<cipher.scale()<<"P1 Sclae:"<<p1.scale()<<"System Sclae:"<<scale<<std::endl;
                    //std::vector<double> pd1;
                    //ckksEncoder->decode(p1,pd1);
                    //std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            
            SEALCipherText *res= new SEALCipherText();
           // std::cout<<"D1: "<<d1<<" + "<<*this<<" = ";
            evaluator->add_plain(cipher,p1,res->cipher);
           // std::cout<<*res<<std::endl;
            return *res;
        }
        SEALCipherText& operator +(Plaintext &p1, SEALCipherText &c1)
        {
           
            if(c1.scheme==scheme_type::CKKS){
                if(c1.ckksEncoder!=NULL)
                {
                    //c1.ckksEncoder->encode(d1,c1.scale,p1);
                    p1.scale()=c1.cipher.scale();
                    c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
             SEALCipherText *res= new SEALCipherText();
            c1.evaluator->add_plain(c1.cipher,p1,res->cipher);
            return *res;
        }
        SEALCipherText& operator +(double d1, SEALCipherText &c1)
        {
            Plaintext p1;
           if(c1.scheme==scheme_type::BFV){
                if(c1.intEncoder!=NULL){
                
                    p1=c1.intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(c1.ckksEncoder!=NULL)
                {
                    c1.ckksEncoder->encode(d1,c1.cipher.scale(),p1);
                    
                    c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
                    //p1.scale()=c1.scale;
                    //c1.cipher.scale()=c1.scale;
                   // std::cout<<"C1 Sclae:"<<c1.cipher.scale()<<"P1 Sclae:"<<p1.scale()<<"System Sclae:"<<c1.scale<<std::endl;
                    //std::vector<double> pd1;
                   // c1.ckksEncoder->decode(p1,pd1);
                   // std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            c1.evaluator->add_plain(c1.cipher,p1,res->cipher);
            return *res;
        }
        SEALCipherText& SEALCipherText::operator +(std::vector<int64_t> d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                    batchEncoder->encode(d1,p1);
                    //std::cout<<"Warning: possible loss of precision and undefined behaviour."<<std::endl;
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            
            
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->add_plain(cipher,p1,res->cipher);
            
            
            return *res;
        }
        SEALCipherText& SEALCipherText::operator +(std::vector<double> d1)
        {
            Plaintext p1;
                    
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //std::cout<<"C1 Sclae:"<<cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                    //p1.scale()=cipher.scale();
                   // std::vector<double> pd1;
                    //ckksEncoder->decode(p1,pd1);
                    //std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            
            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
               
               //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;

             }
            return *res;
        }
        SEALCipherText& SEALCipherText::operator *(SEALCipherText &c1)
        {
            //std::cout<<"*********operator * ************"<<std::endl;
            //std::cout<<*this<<"  "<<c1<<std::endl;
            SEALCipherText *res= new SEALCipherText();
            bool isC1=false;
        
            //std::cout<<cipher.coeff_mod_count()<<"  "<<c1.cipher.coeff_mod_count()<<std::endl;
            if(c1.cipher.coeff_mod_count()!=cipher.coeff_mod_count()&&scheme==scheme_type::CKKS)
            {
                
                if(c1.cipher.coeff_mod_count()>cipher.coeff_mod_count()){
                    *res=c1;
                    isC1=true;
                   // std::cout<<"1. "<<*this<<"  "<<c1<<" "<<*res<<" isC1:"<<isC1<<std::endl;
                    evaluator->mod_switch_to_inplace(res->cipher,cipher.parms_id());
                   // std::cout<<"1. "<<*this<<"  "<<c1<<" "<<*res<<" isC1:"<<isC1<<std::endl;
                    //res->cipher.scale()=scale;

                }
                else if (c1.cipher.coeff_mod_count()<cipher.coeff_mod_count())
                {
                    *res=*this;
                    evaluator->mod_switch_to_inplace(res->cipher,c1.cipher.parms_id());
                    //std::cout<<"2. "<<*this<<"  "<<c1<<" "<<*res<<" isC1:"<<isC1<<std::endl;
                    //res->cipher.scale()=scale;
                }
            }
            else
            {
                *res=*this;
            }
            
           // std::cout<<cipher.coeff_mod_count()<<"  "<<c1.cipher.coeff_mod_count()<<" "<<res->cipher.coeff_mod_count()<<" isC1:"<<isC1<<std::endl;
            //std::cout<<*this<<"  "<<c1<<" "<<*res<<" isC1:"<<isC1<<std::endl;
            SEALCipherText *temp = (isC1)? this : &c1;
            if(this==&c1)
                evaluator->square_inplace(res->cipher);
            else    
               evaluator->multiply_inplace(res->cipher,temp->cipher);//:evaluator->multiply_inplace(res->cipher,c1.cipher);
            //std::cout<<*this<<"  "<<c1<<" "<<*res<<" isC1:"<<isC1<<std::endl;
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS )
            {  
                 //evaluator->relinearize_inplace(res->cipher,*relin_keys);
                 
                 evaluator->rescale_to_next_inplace(res->cipher);
                 res->cipher.scale()=c1.scale;
                 //
            }
           // std::cout<<"*********operator * ends *******"<<std::endl;
            return *res;
        }
         SEALCipherText& SEALCipherText::operator *(Plaintext &p1)
        {
            if(scheme==scheme_type::CKKS)
            {
                cipher.scale()=SEALCipherText::scale;
                p1.scale()=SEALCipherText::scale;
                evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
            }

            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
                
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
             }
            return *res;
        }
        SEALCipherText& operator *(Plaintext &p1, SEALCipherText &c1)
        {
            if(c1.scheme==scheme_type::CKKS)
            {
                c1.cipher.scale()=SEALCipherText::scale;
                p1.scale()=SEALCipherText::scale;
                c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
            }
            SEALCipherText *res= new SEALCipherText();
            c1.evaluator->multiply_plain(c1.cipher,p1,res->cipher);
            c1.evaluator->relinearize_inplace(res->cipher,*c1.relin_keys);
            if(c1.scheme==scheme_type::CKKS){
                
                c1.evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=c1.scale;
             }
            return *res;
        }
        SEALCipherText& SEALCipherText::operator *(int i1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                    p1=intEncoder->encode(i1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode((double)i1,cipher.scale(),p1);
                    //p1.scale()=cipher.scale();
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
                
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
             }
            return *res;
        }
         SEALCipherText& SEALCipherText::operator *(double d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                    p1=intEncoder->encode((int)d1);
                    std::cout<<"Warning: possible loss of precision and undefined behaviour."<<std::endl;
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            else{        
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //std::cout<<"C1 Sclae:"<<cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                    //p1.scale()=cipher.scale();
                   // std::vector<double> pd1;
                    //ckksEncoder->decode(p1,pd1);
                    //std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
               
               //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;

             }
            return *res;
        }

        SEALCipherText& SEALCipherText::operator *(std::vector<int64_t> d1)
        {
            Plaintext p1;
            if(scheme==scheme_type::BFV){
                if(intEncoder!=NULL){
                    batchEncoder->encode(d1,p1);
                    //std::cout<<"Warning: possible loss of precision and undefined behaviour."<<std::endl;
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            
            
            }
            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
               
               //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;

             }
            return *res;
        }

        SEALCipherText& SEALCipherText::operator *(std::vector<double> d1)
        {
            Plaintext p1;
                    
                if(ckksEncoder!=NULL)
                {
                    ckksEncoder->encode(d1,cipher.scale(),p1);
                    
                    evaluator->mod_switch_to_inplace(p1,cipher.parms_id());
                    //std::cout<<"C1 Sclae:"<<cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                    //p1.scale()=cipher.scale();
                   // std::vector<double> pd1;
                    //ckksEncoder->decode(p1,pd1);
                    //std::cout<<"D1: "<<d1<<" encoded into ->"<<pd1[0]<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            
            SEALCipherText *res= new SEALCipherText();
            evaluator->multiply_plain(cipher,p1,res->cipher);
            evaluator->relinearize_inplace(res->cipher,*relin_keys);
            if(scheme==scheme_type::CKKS){
               
               //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;
                //res->cipher.scale()=scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<scale<<std::endl;

             }
            return *res;
        }



        SEALCipherText& operator *(int i1, SEALCipherText &c1)
        {
            Plaintext p1;
            if(c1.scheme==scheme_type::BFV){
                if(c1.intEncoder!=NULL){
                
                    p1=c1.intEncoder->encode(i1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(c1.ckksEncoder!=NULL)
                {
                    c1.ckksEncoder->encode((double)i1,c1.cipher.scale(),p1);
                    //p1.scale()=c1.cipher.scale();
                    c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            c1.evaluator->multiply_plain(c1.cipher,p1,res->cipher);
            c1.evaluator->relinearize_inplace(res->cipher,*c1.relin_keys);
            if(c1.scheme==scheme_type::CKKS){
                
                c1.evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=c1.scale;
             }
            return *res;
        }
         SEALCipherText& operator *(double d1, SEALCipherText &c1)
        {
            Plaintext p1;
            if(c1.scheme==scheme_type::BFV){
                if(c1.intEncoder!=NULL){
                
                    p1=c1.intEncoder->encode((int)d1);
                }
                else
                {
                    std::cout<<"Integer encoder needs to be instantiated."<<std::endl;
                    exit(1);
                } 
            }
            else{        
                if(c1.ckksEncoder!=NULL)
                {
                    c1.ckksEncoder->encode(d1,c1.cipher.scale(),p1);
                    
                   // std::cout<<"C1 Sclae:"<<c1.cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<c1.scale<<std::endl;
                    c1.evaluator->mod_switch_to_inplace(p1,c1.cipher.parms_id());
                    //p1.scale()=c1.cipher.scale();
                   // std::cout<<"C1 Sclae:"<<c1.cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<c1.scale<<std::endl;
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated."<<std::endl;
                    exit(1);
                }
            }
            SEALCipherText *res= new SEALCipherText();
            c1.evaluator->multiply_plain(c1.cipher,p1,res->cipher);
            c1.evaluator->relinearize_inplace(res->cipher,*c1.relin_keys);
            if(c1.scheme==scheme_type::CKKS){
               //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<c1.scale<<std::endl;
                c1.evaluator->rescale_to_next_inplace(res->cipher);
                //res->cipher.scale()=c1.scale;
                //std::cout<<"C1 Sclae:"<<res->cipher.scale()<<" P1 Sclae:"<<p1.scale()<<" System Sclae:"<<c1.scale<<std::endl;
             }
            return *res;
        }

        void SEALCipherText::operator <<=(int i1)
        {
            //if(*galois_keys!=NULL)
            {
                
                 if(scheme==scheme_type::BFV)
                {
                   //std::cout<<"Ciphertext rotation is not supported in BFV scheme. Only Plaintext can be rotated"<<std::endl;
                   evaluator->rotate_rows_inplace(cipher,-i1,*galois_keys);
                }
                else if (scheme==scheme_type::CKKS)
                {
                    evaluator->rotate_vector_inplace(cipher,i1,*galois_keys);
                } 
                
            }
            /* else
            {
                std::cout<<"Galois_keys need to be created before using this operator."<<std::endl;
                exit(1);
            } */
            
        }

        void SEALCipherText::operator >>=(int i1)
        {
            //if(galois_keys!=NULL)
            {
                
                 if(scheme==scheme_type::BFV)
                {
                   std::cout<<"Ciphertext rotation is not supported in BFV scheme. Only Plaintext can be rotated"<<std::endl;
                   evaluator->rotate_rows_inplace(cipher,i1,*galois_keys);
                }
                else if (scheme==scheme_type::CKKS)
                {
                    evaluator->rotate_vector_inplace(cipher,-i1,*galois_keys);
                } 
                
            }
            /* else
            {
                std::cout<<"Galois_keys need to be created before using this operator."<<std::endl;
                exit(1);
            } */
        }

        SEALCipherText& SEALCipherText::operator <<(int i1)
        {
            
           // if(*galois_keys!=NULL)
            {
                
                 if(scheme==scheme_type::BFV)
                {
                   //std::cout<<"Ciphertext rotation is not supported in BFV scheme. Only Plaintext can be rotated"<<std::endl;
                   SEALCipherText* res=new SEALCipherText();
                   evaluator->rotate_rows(cipher,i1,*galois_keys,res->cipher);
                   return *res;
                }
                else if (scheme==scheme_type::CKKS)
                {
                    SEALCipherText* res=new SEALCipherText();
                    evaluator->rotate_vector(cipher,i1,*galois_keys,res->cipher);
                    return *res;
                }
                
            }
           /*  else
            {
                std::cout<<"Galois_keys need to be created before using this operator."<<std::endl;
                exit(1);
            } */

        }

        SEALCipherText& SEALCipherText::operator >>(int i1)
        {
           // if(*galois_keys!=NULL)
            {
                 
                 if(scheme==scheme_type::BFV)
                {
                   //std::cout<<"Ciphertext rotation is not supported in BFV scheme. Only Plaintext can be rotated"<<std::endl;
                   SEALCipherText* res=new SEALCipherText();
                   evaluator->rotate_rows(cipher,-i1,*galois_keys,res->cipher);
                   return *res;
                }
                else if (scheme==scheme_type::CKKS)
                {
                   SEALCipherText* res=new SEALCipherText();
                 evaluator->rotate_vector(cipher,-i1,*galois_keys,res->cipher);
                 return *res;
                } 
                
            }
            /* else
            {
                std::cout<<"Galois_keys need to be created before using this operator."<<std::endl;
                exit(1);
            } */

            
        }
        void operator <<(Plaintext p1,int i1)
        {
             //if(SEALCipherText::*galois_keys!=NULL)
            {
                 
                 if(SEALCipherText::scheme==scheme_type::CKKS)
                {
                   std::cout<<"Only supported in BFV scheme."<<std::endl;
                }
                else if (SEALCipherText::scheme==scheme_type::BFV)
                {
                    //SEALCipherText* res=new SEALCipherText();
                    //SEALCipherText::evaluator->rotate_vector_inplace(p1,-i1,*SEALCipherText::*galois_keys);
                 
                } 
                
            }
            /* else
            {
                std::cout<<"Galois_keys need to be created before using this operator."<<std::endl;
                exit(1);
            } */
        }
        SEALCipherText& operator ^(SEALCipherText c1,int i1)
        {
                 SEALCipherText* res=new SEALCipherText();
                 c1.evaluator->exponentiate(c1.cipher,i1,*SEALCipherText::relin_keys,res->cipher);
                 return *res;
        }
        SEALCipherText& operator ^(SEALCipherText c1,double i1)
        {
                 SEALCipherText* res=new SEALCipherText();
                 c1.evaluator->exponentiate(c1.cipher,(int)i1,*SEALCipherText::relin_keys,res->cipher);
                 return *res;
        }
        void operator >>(Plaintext p1,int i1);
        std::ostream& operator <<(std::ostream &out, SEALCipherText &c1)
        {
            Plaintext result;
           {
            //Stopwatch s("Decrypt");
            SEALCipherText::decryptor->decrypt(c1.cipher,result);
           }
            if(c1.scheme==seal::scheme_type::BFV)
            { 
                 int val;
                if(c1.isBatchingEnabled())
                {
                    std::vector<int64_t> realRes;
                    c1.batchEncoder->decode(result,realRes);
                    val=realRes[0];
                }
                else
                {
                    IntegerEncoder *encoder;
                    encoder=new IntegerEncoder(c1.context);
                    val=encoder->decode_int32(result);
                }
                
                
                out<<val;
            }
             else
             {
                if(c1.ckksEncoder!=NULL)
                {
                    std::vector<double> realRes;
                    c1.ckksEncoder->decode(result,realRes);
                    out<<realRes[0];
                }
                else
                {
                    std::cout<<"CKKS encoder needs to be instantiated"<<std::endl;
                }
                
             }
             
            //encoder.decode_int32(result);
            out.flush();
           return out;
        }
        int SEALCipherText::save(std::string fileName)
        {
            std::ofstream file(fileName,std::ofstream::app);
            /*std::stringstream parms_stream;
            std::stringstream data_stream;
            std::stringstream sk_stream;
            auto sizeOfCipherText=cipher.save(data_stream);
            file<<data_stream.rdbuf();
            printf("Saved %d bytes.\n",sizeOfCipherText);
            file.close();*/
            size_t coef_modulus_size=cipher.coeff_mod_count();
            size_t poly_modulus_degree=cipher.poly_modulus_degree();
            size_t ctxtSize=cipher.size();
            size_t numOfCoefs=coef_modulus_size*poly_modulus_degree*ctxtSize;
            //printf("Size of ciphertext is : %d\n",);
            uint64_t *coeffs=new uint64_t[numOfCoefs];
            uint *coeffs32_t=(uint*)coeffs;
            for(size_t i=0;i<numOfCoefs;i++)
                coeffs[i]=cipher[i];
            
            for(size_t i=0;i<2*numOfCoefs;i+=2)
               file<<coeffs32_t[i]<<std::endl;
            
            printf("Saved %d bytes.\n",8*numOfCoefs);
            file.close();
            //auto sizeOfCipherText=sizeof(cipher[1]);
            //printf("size of data element:%d \n size of data array:%d\n",sizeOfCipherText,cipher.size());
            return 8*numOfCoefs;
        }

        /*
Helper function: Prints the parameters in a SEALContext.
*/
inline void print_parameterz(std::shared_ptr<seal::SEALContext> context)
{
    // Verify parameters
    if (!context)
    {
        throw std::invalid_argument("context is not set");
    }
    auto &context_data = *context->key_context_data();

    /*
    Which scheme are we using?
    */
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw std::invalid_argument("unsupported scheme");
    }
    std::cout << "/" << std::endl;
    std::cout << "| Encryption parameters :" << std::endl;
    std::cout << "|   scheme: " << scheme_name << std::endl;
    std::cout << "|   poly_modulus_degree: " <<
        context_data.parms().poly_modulus_degree() << std::endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    std::cout << "|   coeff_modulus size: ";
    std::cout << context_data.total_coeff_modulus_bit_count() << " (";
    auto coeff_modulus = context_data.parms().coeff_modulus();
    std::size_t coeff_mod_count = coeff_modulus.size();
    for (std::size_t i = 0; i < coeff_mod_count - 1; i++)
    {
        std::cout << coeff_modulus[i].bit_count() << " + ";
    }
    std::cout << coeff_modulus.back().bit_count();
    std::cout << ") bits" << std::endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == seal::scheme_type::BFV)
    {
        std::cout << "|   plain_modulus: " << context_data.
            parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\" << std::endl;
}

#endif