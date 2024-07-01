#pragma once
#include <thread>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
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
#pragma comment(lib, "Ws2_32.lib")

#define KEY_BUFFER_SIZE 2048
#define RSA_KEY_LENGTH 2048
#define RSA_PLAINTEXT_SIZE 128

char private_key[KEY_BUFFER_SIZE] = { 0 };
char public_key[KEY_BUFFER_SIZE] = { 0 };