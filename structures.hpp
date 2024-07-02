#pragma once
//这个文件用来存放所有数据结构相关的结构体
#include "includes.h"
struct userInfo
{
    std::string name;
    std::string password;
    std::string mac;
    std::string aes_key;
    std::string aes_iv;
};
