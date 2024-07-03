#pragma once
#include "includes.h"
#include "fonts.h"

//����ļ��������洢ȫ�ֱ�����
namespace fs = std::filesystem;
std::string server_info = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1";//��������Ϣ�����ڿͻ���ȷ�Ϸ����������

char private_key[KEY_BUFFER_SIZE] = { 0 };//RSA��Կ
char public_key[KEY_BUFFER_SIZE] = { 0 };//RSA˽Կ
std::vector<void*> all_connected_clients;//�洢���е������ӿͻ��˴�������ָ��
int client_width = 1920;//��������Ļ�Ŀ�͸�
int client_height = 1080;
void parse_userInfo(userInfo* output, std::string receive_str) {//����ͻ��˷����Ŀͻ���Ϣ�ַ������ָ����������ṹ��
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
std::string getCurrentTime() {//��ȡ��ǰʱ�䣬�����ͼʱ�ļ�����
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
std::string Get_path_by_id(const std::string& id) {
    std::string rpath = id;
    std::replace(rpath.begin(), rpath.end(), ':', '_');
    return rpath;
}
void createDirectoryIfNotExists(const std::string& directoryPath) {//���Ŀ¼�Ƿ���ڣ��������򴴽��������ͼʱ��
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
//��vector<uint8_t>�е����ݴ����ļ��С������滻����:��Ϊ���ļ������ܴ��ڵķǷ��ַ�
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
    // ���ļ�������Ϊ������ģʽ
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("�޷����ļ�: " + filePath);
    }

    // ��ȡ�ļ���С
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // ����һ�����ļ���С��ͬ��vector
    std::vector<uint8_t> buffer(fileSize);

    // ��ȡ�ļ����ݵ�vector��
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("��ȡ�ļ�ʧ��: " + filePath);
    }

    return buffer;
}