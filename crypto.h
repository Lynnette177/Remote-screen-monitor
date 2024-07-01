#pragma once
#pragma warning(push)     //禁用4996警告防止openssl因版本问题报错，并在文件尾部取消掉这个禁用
#pragma warning(disable: 4996)
#include "includes.h"

std::string base64Encode(unsigned char * input,int length) {
    BIO* bio, * b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // Ignore newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string encoded(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encoded;
}

std::string base64Decode(const std::string& encoded) {
    BIO* bio, * b64;
    BUF_MEM* bufferPtr;
    size_t decodeLen = encoded.length();
    char* buffer = (char*)malloc(decodeLen);

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(encoded.data(), encoded.length());
    bio = BIO_push(b64, bio);

    // Ignore newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decodeLen = BIO_read(bio, buffer, encoded.length());
    BIO_free_all(bio);

    std::string decoded(buffer, decodeLen);
    free(buffer);
    return decoded;
}


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

int rsa_test_example(char * publicKey, char * privateKey,unsigned char * plaintext,int length) {

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

    decrypted[decryptedLen] = '\0';
    std::cout << "Decrypted text: " << decrypted << std::endl;

    return 0;
}
// AES加密函数
int aes_encrypt(unsigned char* plaintext, int plaintext_len,
    unsigned char* key, unsigned char* iv,
    unsigned char* ciphertext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int ciphertext_len;
    // 创建并初始化加密上下文
    if (!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();
    // 初始化加密操作，选择AES-256-CBC加密算法
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleOpenSSLErrors();
    // 执行加密操作
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleOpenSSLErrors();
    ciphertext_len = len;
    // 结束加密操作
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleOpenSSLErrors();
    ciphertext_len += len;
    // 清理加密上下文
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}
// AES解密函数
int aes_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* key, unsigned char* iv,
    unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    // 创建并初始化解密上下文
    if (!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();
    // 初始化解密操作，选择AES-256-CBC加密算法
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleOpenSSLErrors();
    // 执行解密操作
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleOpenSSLErrors();
    plaintext_len = len;
    // 结束解密操作
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleOpenSSLErrors();
    plaintext_len += len;
    // 清理解密上下文
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}
int aes_test_example() {
    unsigned char key[AES_BLOCK_SIZE]; // 256-bit key
    unsigned char iv[AES_BLOCK_SIZE];  // initialization vector
    // Generate AES key and IV
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));
    const char* plaintext = "U:HEreisAtest;P:testIsGood;1234567890123456";
    int plaintext_len = strlen(plaintext);
    // Determine ciphertext length
    int ciphertext_len = ((plaintext_len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* ciphertext = new unsigned char[ciphertext_len];
    unsigned char* decryptedtext = new unsigned char[ciphertext_len];
    memset(ciphertext, 0, ciphertext_len);
    memset(decryptedtext, 0, ciphertext_len);
    // Encrypt plaintext
    aes_encrypt((unsigned char*)plaintext, plaintext_len, key, iv, ciphertext);
    // Decrypt ciphertext
    aes_decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
    decryptedtext[plaintext_len] = '\0';
    std::cout << "Original: " << plaintext << std::endl;
    std::cout << "Decrypted: " << decryptedtext << std::endl;
    delete[] ciphertext;
    delete[] decryptedtext;
    return 0;
}

//以上是标准加解密函数。下面是会用到的函数，经过正确的格式转换之后调用上方标准函数。

std::string rsa_decrypt_base(const std::string& encoded) {//解码base64然后RSA解密
    unsigned char decrypted[256];
    std::string base64_decoded = base64Decode(encoded);
    int decryptedLen = rsaDecrypt((unsigned char*)base64_decoded.c_str(), 256, (unsigned char*)private_key, decrypted);
    if (decryptedLen == -1) {
        handleOpenSSLErrors();
    }
    decrypted[decryptedLen] = '\0';
    std::string ret((char *)decrypted);
    return ret;
}
//AES加密后编码base64
std::string aes_encrypt_base(std::string key, std::string iv, unsigned char* plaintext) {
    std::string decoded_key = base64Decode(key);
    std::string decoded_iv = base64Decode(iv);
    int ciphertext_len = ((strlen((char*)plaintext) + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char* ciphertext = new unsigned char[ciphertext_len];
    int length = aes_encrypt(plaintext, strlen((char*)plaintext),
        (unsigned char*)decoded_key.c_str(), (unsigned char*)decoded_iv.c_str(),
        ciphertext);
    return base64Encode((unsigned char*)ciphertext,length);
}
//解码base64然后AES解密 获取字符串（控制信息）
std::string aes_decrypt_base_to_string(std::string key, std::string iv, unsigned char* ciphertext) {
    std::string decoded_key = base64Decode(key);
    std::string decoded_iv = base64Decode(iv);
    std::string base64_decoded = base64Decode(std::string((char *)ciphertext));
    int ciphertext_len = base64_decoded.length();
    unsigned char* plaintext = new unsigned char[ciphertext_len];
    int length = aes_decrypt((unsigned char *)base64_decoded.c_str(), ciphertext_len,
        (unsigned char*)decoded_key.c_str(), (unsigned char*)decoded_iv.c_str(),
        plaintext);
    plaintext[length] = '\0';
    return std::string((char *)plaintext);
}
//解码base64然后AES解密 拷贝到数组中。必须要记得释放内存
int aes_decrypt_base_to_bytes(std::string key, std::string iv, unsigned char* ciphertext,unsigned char** plaintext_ptr) {
    std::string decoded_key = base64Decode(key);
    std::string decoded_iv = base64Decode(iv);
    std::string base64_decoded = base64Decode(std::string((char*)ciphertext));
    int ciphertext_len = base64_decoded.length();
    unsigned char* plaintext = new unsigned char[ciphertext_len];
    int length = aes_decrypt((unsigned char*)base64_decoded.c_str(), ciphertext_len,
        (unsigned char*)decoded_key.c_str(), (unsigned char*)decoded_iv.c_str(),
        plaintext);
    plaintext[length] = '\0';
    *plaintext_ptr = new unsigned char[length+10];
    memset(*plaintext_ptr, 0, length + 10);
    memcpy_s(*plaintext_ptr, length, plaintext, length);
    return length;
}

#pragma warning(pop)