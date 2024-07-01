#pragma once
#pragma warning(push)     //����4996�����ֹopenssl��汾���ⱨ�������ļ�β��ȡ�����������
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
// AES���ܺ���
int aes_encrypt(unsigned char* plaintext, int plaintext_len,
    unsigned char* key, unsigned char* iv,
    unsigned char* ciphertext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int ciphertext_len;
    // ��������ʼ������������
    if (!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();
    // ��ʼ�����ܲ�����ѡ��AES-256-CBC�����㷨
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleOpenSSLErrors();
    // ִ�м��ܲ���
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleOpenSSLErrors();
    ciphertext_len = len;
    // �������ܲ���
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleOpenSSLErrors();
    ciphertext_len += len;
    // �������������
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}
// AES���ܺ���
int aes_decrypt(unsigned char* ciphertext, int ciphertext_len,
    unsigned char* key, unsigned char* iv,
    unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int plaintext_len;
    // ��������ʼ������������
    if (!(ctx = EVP_CIPHER_CTX_new())) handleOpenSSLErrors();
    // ��ʼ�����ܲ�����ѡ��AES-256-CBC�����㷨
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleOpenSSLErrors();
    // ִ�н��ܲ���
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleOpenSSLErrors();
    plaintext_len = len;
    // �������ܲ���
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleOpenSSLErrors();
    plaintext_len += len;
    // �������������
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

//�����Ǳ�׼�ӽ��ܺ����������ǻ��õ��ĺ�����������ȷ�ĸ�ʽת��֮������Ϸ���׼������

std::string rsa_decrypt_base(const std::string& encoded) {//����base64Ȼ��RSA����
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
//AES���ܺ����base64
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
//����base64Ȼ��AES���� ��ȡ�ַ�����������Ϣ��
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
//����base64Ȼ��AES���� �����������С�����Ҫ�ǵ��ͷ��ڴ�
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