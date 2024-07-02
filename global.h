#pragma once
#include "includes.h"
#include "fonts.h"
std::string server_info = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1";

char private_key[KEY_BUFFER_SIZE] = { 0 };
char public_key[KEY_BUFFER_SIZE] = { 0 };
std::vector<void*> all_connected_clients;//存储所有的已连接客户端处理器的指针
int client_width = 1920;
int client_height = 1080;
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