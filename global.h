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
    std::string username, password,mac, key, iv;
    // 创建一个字符串输入流
    std::istringstream ss(receive_str);
    // 使用getline函数和分号作为分隔符进行分割
    std::getline(ss, username, ';');
    std::getline(ss, password, ';');
    std::getline(ss, mac, ';');
    std::getline(ss, key, ';');
    std::getline(ss, iv, ';');
    output->name = username;
    output->password = password;
    output->mac = mac;
    output->aes_key = key;
    output->aes_iv = iv;
}


std::string getCurrentTime() {
    // 获取当前时间点
    std::time_t now = std::time(nullptr);
    // 定义 tm 结构并使用 localtime_s 安全地转换时间点
    std::tm localTime;
    localtime_s(&localTime, &now);

    // 使用字符串流格式化时间
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}


void createDirectoryIfNotExists(const std::string& directoryPath) {
    if (!fs::exists(directoryPath)) {
        if (fs::create_directories(directoryPath)) {
            std::cout << "Directory created: " << directoryPath << std::endl;
        }
        else {
            std::cerr << "Failed to create directory: " << directoryPath << std::endl;
        }
    }
    else {
        std::cout << "Directory already exists: " << directoryPath << std::endl;
    }
}
void saveVectorToBinaryFile(const std::vector<uint8_t>& data, const std::string& path, const std::string& filename) {
    std::string rpath = path;
    std::string rname = filename;
    std::replace(rpath.begin(), rpath.end(), ':', '_');
    std::replace(rname.begin(), rname.end(), ':', '_');
    std::cout << rpath << std::endl;
    std::cout << rname << std::endl;
    createDirectoryIfNotExists(rpath);
    std::string filename_with_path = rpath + "/" + rname;
    std::ofstream outputFile(filename_with_path, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error opening file for writing: " << filename_with_path << std::endl;
        return;
    }

    outputFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outputFile.close();

    if (!outputFile.good()) {
        std::cerr << "Error occurred while writing to file: " << filename_with_path << std::endl;
    }
}