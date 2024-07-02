#include "includes.h"

//����ļ�����������RSA��Կ����غ���

// �������ڴ�PEM��ʽ�ַ�������ȡ��Կ����
std::string extract_key_content(const std::string& pem_key) {
    size_t start = pem_key.find("-----BEGIN");
    size_t end = pem_key.find("-----END");

    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return ""; // ���ؿ��ַ�����ʾ��ȡʧ��
    }

    start = pem_key.find("\n", start) + 1; // �ҵ���ʼ��֮��ĵ�һ�����з�

    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return ""; // ���ؿ��ַ�����ʾ��ȡʧ��
    }

    return pem_key.substr(start, end - start);
}

int keygen(char* private_out, char* public_out) {
    // ��ʼ��OpenSSL��
    OpenSSL_add_all_algorithms();

    // ����EVP��Կ��������
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!pctx) {
        std::cerr << "Error creating context" << std::endl;
        return 1;
    }

    // ��ʼ����Կ����
    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        std::cerr << "Error initializing keygen" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // ����RSA��Կ����
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) {
        std::cerr << "Error setting RSA key length" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // ������Կ��
    EVP_PKEY* pkey = NULL;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        std::cerr << "Error generating key pair" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // ����������
    EVP_PKEY_CTX_free(pctx);

    // д��˽Կ
    BIO* bio = BIO_new(BIO_s_mem());
    if (PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL) <= 0) {
        std::cerr << "Error writing private key" << std::endl;
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return 1;
    }

    char* privKeyPEM = NULL;
    long privKeyLen = BIO_get_mem_data(bio, &privKeyPEM);
    std::string privateKey(privKeyPEM, privKeyLen);

    // д�빫Կ
    BIO_reset(bio);
    if (PEM_write_bio_PUBKEY(bio, pkey) <= 0) {
        std::cerr << "Error writing public key" << std::endl;
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return 1;
    }

    char* pubKeyPEM = NULL;
    long pubKeyLen = BIO_get_mem_data(bio, &pubKeyPEM);
    std::string publicKey(pubKeyPEM, pubKeyLen);

    // ����
    EVP_PKEY_free(pkey);
    BIO_free(bio);

    // ��ȡ��Կ����
    //std::string extracted_private_key = extract_key_content(privateKey);
    //std::string extracted_public_key = extract_key_content(publicKey);

    // ����Կ���ݸ��Ƶ��������
    strcpy_s(private_out, KEY_BUFFER_SIZE, privateKey.c_str());
    strcpy_s(public_out, KEY_BUFFER_SIZE, publicKey.c_str());

    return 0;
}
