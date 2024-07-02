#pragma once
#include "includes.h"
#include "fonts.h"
std::string server_info = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1";

char private_key[KEY_BUFFER_SIZE] = { 0 };
char public_key[KEY_BUFFER_SIZE] = { 0 };
std::vector<void*> all_connected_clients;//�洢���е������ӿͻ��˴�������ָ��
int client_width = 1920;
int client_height = 1080;
void parse_userInfo(userInfo* output, std::string receive_str) {
    // ���ڴ洢�ָ������ַ���
    std::string username, password,mac, key, iv;
    // ����һ���ַ���������
    std::istringstream ss(receive_str);
    // ʹ��getline�����ͷֺ���Ϊ�ָ������зָ�
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
    // ��ȡ��ǰʱ���
    std::time_t now = std::time(nullptr);
    // ���� tm �ṹ��ʹ�� localtime_s ��ȫ��ת��ʱ���
    std::tm localTime;
    localtime_s(&localTime, &now);

    // ʹ���ַ�������ʽ��ʱ��
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