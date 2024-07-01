#pragma once
#include "includes.h"
#include "crypto.h"

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
class ClientHandler {
private:
    SOCKET clnt_control_sock;
    SOCKET clnt_data_sock;
    userInfo info_struct;
public:
    ClientHandler(SOCKET _clnt_control_sock) : clnt_control_sock(_clnt_control_sock) {
        printClientInfo(true);
    }
    void show_private_info() {//展示用户私有信息调试用
        std::cout << "Username: " << info_struct.name << std::endl;
        std::cout << "Password: " << info_struct.password << std::endl;
        std::cout << "Key: " << info_struct.aes_key << std::endl;
        std::cout << "IV: " << info_struct.aes_iv << std::endl;
    }
    bool send_control_message(const char* message) {
        try {
            int iResult = send(clnt_control_sock, message, strlen(message), 0);
;           send(clnt_control_sock, "\x01", 1, 0);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            closesocket(clnt_control_sock);
            WSACleanup();
            system("pause");
            abort();
            return false;
        }
    }
    void printClientInfo(bool init = false) {//打印客户端连接信息
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(clnt_control_sock, (struct sockaddr*)&addr, &addr_len) == 0) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
            if (init)
                std::cout << "New Connection Established. Client IP: " << ip << ", Port: " << ntohs(addr.sin_port) << "\n";
            else
                std::cout << "Client IP: " << ip << ", Port: " << ntohs(addr.sin_port) << ":";
        }
        else {
            std::cerr << "Error getting client info" << std::endl;
        }
    }
    void handle() {//控制信息处理线程
        try {
            //对连接上的客户端发送公钥，让客户端回复加密后的用户名和密码，解密并验证。
            char buffer[1024] = {};
            int bytes_received;
            printClientInfo();
            send_control_message(public_key);
            if ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                std::string plain_text = rsa_decrypt_base(buffer);
                parse_userInfo(&info_struct,plain_text);
                show_private_info();
                std::string cipher_mesg = aes_encrypt_base(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)server_info.c_str());
                std::cout << cipher_mesg;
                send_control_message(cipher_mesg.c_str());
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }
            if ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                std::cout << "\n" << buffer << "\n";
                //两种解密方式 example:
                std::string plain_text = aes_decrypt_base_to_string(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer);
                
                unsigned char* ptext = NULL;
                aes_decrypt_base_to_bytes(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer, &ptext);
                delete[]  ptext;
                
                if (plain_text != "Correct") {
                    printf("客户端认证失败\n");
                    return;
                }
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }
            printf("客户端认证成功\n");
            closesocket(clnt_control_sock);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in client_handler: " << e.what() << std::endl;
        }
    }
};



class Server {
private:
    const wchar_t* ip;
    int port;
    SOCKET serv_sock;
    std::vector<std::thread> client_threads;
    std::atomic<bool> stop_server;

public:
    Server(const wchar_t* _ip, int _port) : ip(_ip), port(_port), serv_sock(INVALID_SOCKET), stop_server(false) {}

    bool initialize() {
        WSADATA wsadata;
        WORD w_req = MAKEWORD(2, 2);
        int err_code = WSAStartup(w_req, &wsadata);
        if (err_code != 0) {
            printf("Initialize failed.\n");
            return false;
        }
        else {
            printf("Initialized.\n");
        }
        if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
            printf("Version initialize failed.\n");
            WSACleanup();
            return false;
        }
        else {
            printf("All done\n");
            return true;
        }
    }

    void start_server() {
        if (!initialize()) {
            std::cerr << "Server initialization failed." << std::endl;
            return;
        }

        std::cout << "Server Starting" << std::endl;

        serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serv_sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed." << std::endl;
            return;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        int iResult = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            return;
        }

        iResult = listen(serv_sock, 20);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
            return;
        }
        std::cout << "Server Started" << std::endl;

        while (!stop_server) {
            struct sockaddr_in clnt_addr;
            socklen_t clnt_addr_size = sizeof(clnt_addr);
            SOCKET clnt_control_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_control_sock == INVALID_SOCKET) {
                std::cerr << "Accept failed or manually exited." << std::endl;
                continue;
            }
            client_threads.emplace_back([clnt_control_sock]() { ClientHandler handler(clnt_control_sock); handler.handle(); });
        }
    }

    void stop() {
        stop_server = true;

        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        if (serv_sock != INVALID_SOCKET) {
            closesocket(serv_sock);
            serv_sock = INVALID_SOCKET;
        }
        WSACleanup();
    }

    ~Server() {
        stop();
    }
};