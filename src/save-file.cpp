#include <iostream>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <boost/multiprecision/cpp_int.hpp>
#include "util.h"

using namespace boost::multiprecision;

// Helper to convert Boost cpp_int to OpenSSL BIGNUM
BIGNUM* boost_to_bn(const cpp_int& val) {
    std::stringstream ss;
    ss << std::hex << val;
    BIGNUM* bn = nullptr;
    BN_hex2bn(&bn, ss.str().c_str());
    return bn;
}

void save_pem(const cpp_int& n, const cpp_int& e, const cpp_int& d, 
              const cpp_int& p, const cpp_int& q) {

    std::cout << "\nSaving PEM file...\n";
    RSA* rsa = RSA_new();
    if (!rsa) {
        std::cerr << "Error: Failed to create RSA structure\n";
        return;
    }
    
    // Set n, e, d
    RSA_set0_key(rsa, boost_to_bn(n), boost_to_bn(e), boost_to_bn(d));
    
    // Set p, q
    RSA_set0_factors(rsa, boost_to_bn(p), boost_to_bn(q));

    // Calculate and set CRT parameters
    cpp_int dmp1 = d % (p - 1);
    cpp_int dmq1 = d % (q - 1);
    cpp_int iqmp = mod_inverse(q, p);
    
    RSA_set0_crt_params(rsa, boost_to_bn(dmp1), boost_to_bn(dmq1), boost_to_bn(iqmp));
    
    FILE* fp = fopen("private_key.pem", "wb");
    if (!fp) {
        std::cerr << "Error: Could not open private_key.pem for writing\n";
        RSA_free(rsa);
        return;
    }
    
    int result = PEM_write_RSAPrivateKey(fp, rsa, nullptr, nullptr, 0, nullptr, nullptr);
    if (!result) {
        std::cerr << "Error: PEM_write_RSAPrivateKey failed\n";
        ERR_print_errors_fp(stderr);
    } else {
        std::cout << "Successfully saved private_key.pem" << std::endl;
    }
    
    fflush(fp);
    fclose(fp);
    RSA_free(rsa);
}