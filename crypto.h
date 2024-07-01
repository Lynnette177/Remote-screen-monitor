#pragma warning(push)     //禁用4996警告防止openssl因版本问题报错，并在文件尾部取消掉这个禁用
#pragma warning(disable: 4996)
#include <iostream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <cstring>

void handleOpenSSLErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

RSA* createRSA(unsigned char* key, bool isPublic) {
    RSA* rsa = NULL;
    BIO* keybio = BIO_new_mem_buf(key, -1);
    if (keybio == NULL) {
        handleOpenSSLErrors();
    }
    if (isPublic) {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    }
    else {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    }
    if (rsa == NULL) {
        handleOpenSSLErrors();
    }
    BIO_free(keybio);
    return rsa;
}

int rsaEncrypt(unsigned char* data, int dataLen, unsigned char* key, unsigned char* encrypted) {
    RSA* rsa = createRSA(key, true);
    int result = RSA_public_encrypt(dataLen, data, encrypted, rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);
    return result;
}

int rsaDecrypt(unsigned char* encrypted, int encryptedLen, unsigned char* key, unsigned char* decrypted) {
    RSA* rsa = createRSA(key, false);
    int result = RSA_private_decrypt(encryptedLen, encrypted, decrypted, rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);
    return result;
}

int test(char * publicKey, char * privateKey,unsigned char * plaintext,int length) {

    unsigned char encrypted[256];
    unsigned char decrypted[256];

    int encryptedLen = rsaEncrypt(plaintext, length, (unsigned char*)publicKey, encrypted);
    if (encryptedLen == -1) {
        handleOpenSSLErrors();
    }

    int decryptedLen = rsaDecrypt(encrypted, encryptedLen, (unsigned char*)privateKey, decrypted);
    if (decryptedLen == -1) {
        handleOpenSSLErrors();
    }

    //decrypted[decryptedLen] = '\0';
    std::cout << "Decrypted text: " << decrypted << std::endl;

    return 0;
}


#pragma warning(pop)