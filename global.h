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
    std::string username, password, key, iv;
    // ����һ���ַ���������
    std::istringstream ss(receive_str);
    // ʹ��getline�����ͷֺ���Ϊ�ָ������зָ�
    std::getline(ss, username, ';');
    std::getline(ss, password, ';');
    std::getline(ss, key, ';');
    std::getline(ss, iv, ';');
    output->name = username;
    output->password = password;
    output->aes_key = key;
    output->aes_iv = iv;
}