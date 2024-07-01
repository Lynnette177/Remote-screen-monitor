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
#include <Windows.h>
#include <tchar.h>
#include "d3d9.h"
#include "d3d11.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#define KEY_BUFFER_SIZE 2048
#define RSA_KEY_LENGTH 2048
#define RSA_PLAINTEXT_SIZE 128
