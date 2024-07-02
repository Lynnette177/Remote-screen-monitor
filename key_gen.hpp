#include "includes.h"

//这个文件是用来生成RSA密钥的相关函数

// 函数用于从PEM格式字符串中提取密钥内容
std::string extract_key_content(const std::string& pem_key) {
    size_t start = pem_key.find("-----BEGIN");
    size_t end = pem_key.find("-----END");

    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return ""; // 返回空字符串表示提取失败
    }

    start = pem_key.find("\n", start) + 1; // 找到起始行之后的第一个换行符

    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return ""; // 返回空字符串表示提取失败
    }

    return pem_key.substr(start, end - start);
}

int keygen(char* private_out, char* public_out) {
    // 初始化OpenSSL库
    OpenSSL_add_all_algorithms();

    // 创建EVP密钥对上下文
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!pctx) {
        std::cerr << "Error creating context" << std::endl;
        return 1;
    }

    // 初始化密钥生成
    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        std::cerr << "Error initializing keygen" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // 设置RSA密钥长度
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) {
        std::cerr << "Error setting RSA key length" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // 生成密钥对
    EVP_PKEY* pkey = NULL;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        std::cerr << "Error generating key pair" << std::endl;
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    // 清理上下文
    EVP_PKEY_CTX_free(pctx);

    // 写入私钥
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

    // 写入公钥
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

    // 清理
    EVP_PKEY_free(pkey);
    BIO_free(bio);

    // 提取密钥内容
    //std::string extracted_private_key = extract_key_content(privateKey);
    //std::string extracted_public_key = extract_key_content(publicKey);

    // 将密钥内容复制到输出参数
    strcpy_s(private_out, KEY_BUFFER_SIZE, privateKey.c_str());
    strcpy_s(public_out, KEY_BUFFER_SIZE, publicKey.c_str());

    return 0;
}
