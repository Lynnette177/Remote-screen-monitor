#pragma once
#include <thread>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <cstring>
#include <vector>
#include <atomic>
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "structures.hpp"
#pragma comment(lib, "Ws2_32.lib")



#define KEY_BUFFER_SIZE 2048
#define RSA_KEY_LENGTH 2048
#define RSA_PLAINTEXT_SIZE 128
std::string server_info = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1";

char private_key[KEY_BUFFER_SIZE] = { 0 };
char public_key[KEY_BUFFER_SIZE] = { 0 };

void parse_userInfo(userInfo* output, std::string receive_str) {
    // 用于存储分割后的子字符串
    std::string username, password, key, iv;
    // 创建一个字符串输入流
    std::istringstream ss(receive_str);
    // 使用getline函数和分号作为分隔符进行分割
    std::getline(ss, username, ';');
    std::getline(ss, password, ';');
    std::getline(ss, key, ';');
    std::getline(ss, iv, ';');
    output->name = username;
    output->password = password;
    output->aes_key = key;
    output->aes_iv = iv;
}