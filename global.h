#pragma once
#include "includes.h"
#include "fonts.h"

//这个文件是用来存储全局变量的
namespace fs = std::filesystem;
std::string server_info = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1";//服务器信息，用于客户端确认服务器的身份

char private_key[KEY_BUFFER_SIZE] = { 0 };//RSA公钥
char public_key[KEY_BUFFER_SIZE] = { 0 };//RSA私钥
std::vector<void*> all_connected_clients;//存储所有的已连接客户端处理器的指针
int client_width = 1920;//服务器屏幕的宽和高
int client_height = 1080;
void parse_userInfo(userInfo* output, std::string receive_str) {//处理客户端发来的客户信息字符串，分割出来并存入结构体
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
std::string getCurrentTime() {//获取当前时间，保存截图时文件名用
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
std::string Get_path_by_id(const std::string& id) {
    std::string rpath = id;
    std::replace(rpath.begin(), rpath.end(), ':', '_');
    return rpath;
}
void createDirectoryIfNotExists(const std::string& directoryPath) {//检查目录是否存在，不存在则创建。保存截图时用
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
//把vector<uint8_t>中的数据存入文件中。其中替换掉了:因为是文件名不能存在的非法字符
void saveVectorToBinaryFile(const std::vector<uint8_t>& data, const std::string& id, const std::string& filename) {
    std::string rpath = Get_path_by_id(id);

    std::string rname = filename;
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
std::vector<uint8_t> readFileToVector(const std::string& filePath) {
    // 打开文件并设置为二进制模式
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 创建一个与文件大小相同的vector
    std::vector<uint8_t> buffer(fileSize);

    // 读取文件内容到vector中
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("读取文件失败: " + filePath);
    }

    return buffer;
}