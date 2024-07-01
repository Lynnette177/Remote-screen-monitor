#pragma once
#include "global.h"
#include "includes.h"
#include "dx11imageloader.h"
#include "crypto.h"
class ClientHandler {
private:
    SOCKET clnt_control_sock;
    SOCKET clnt_data_sock;
    int udp_port = 0;
    userInfo info_struct;
public:
    std::string client_info;
    std::vector<std::uint8_t> data_buffer;
    bool generated_new_texture = false;
    Texture thumb_texture;
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
            client_info = ip;
            client_info += ":";
            client_info += std::to_string(ntohs(addr.sin_port));
        }
        else {
            std::cerr << "Error getting client info" << std::endl;
        }
    }
    void handle() {//控制信息处理线程
        try {
            //对连接上的客户端发送公钥，让客户端回复加密后的用户名和密码，解密并验证。
            bool authorized = false;
            char buffer[1024] = {};
            int bytes_received;
            printClientInfo();
            send_control_message(public_key);
            if ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                std::string plain_text = rsa_decrypt_base(buffer);
                parse_userInfo(&info_struct,plain_text);
                show_private_info();
                init_udp_server();
                std::string info_to_send = server_info + ";" + std::to_string(udp_port);
                std::string cipher_mesg = aes_encrypt_base(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)info_to_send.c_str());
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
                else {
                    printf("客户端认证成功\n");
                    authorized = true;
                }
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }
            if (!authorized) {
                closesocket(clnt_control_sock);
                return;
            }
            else {
                all_connected_clients.push_back(this);
                udp_handler();
            }
            closesocket(clnt_control_sock);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in client_handler: " << e.what() << std::endl;
        }
    }
    void init_udp_server() {
        SOCKET udp_serv_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp_serv_sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed." << std::endl;
            return;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(0);

        // 绑定套接字到指定端口
        int iResult = bind(udp_serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
            return;
        }
        socklen_t addr_len = sizeof(serv_addr);
        iResult = getsockname(udp_serv_sock, (struct sockaddr*)&serv_addr, &addr_len);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "getsockname failed: " << WSAGetLastError() << std::endl;
            return;
        }

        int assigned_port = ntohs(serv_addr.sin_port);
        std::cout << "Server Started and listening on port " << assigned_port << std::endl;
        clnt_data_sock = udp_serv_sock;
        udp_port = assigned_port;
    }
    void udp_handler() {
        bool new_pic = true;
        while (true) {
            char buffer[1024] = { 0 };
            struct sockaddr_in client_addr;
            int client_addr_len = sizeof(client_addr);

            // 接收数据包
            int recv_len = recvfrom(clnt_data_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (new_pic) {
                data_buffer.clear();
                new_pic = false;
            }
            if (recv_len == SOCKET_ERROR) {
                std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            if (strcmp(buffer, "END") == 0) {
               // std::cout << "Received END, processing data..." << std::endl;
                //std::cout << "Data size: " << data_buffer.size() << " bytes" << std::endl;
                new_pic = true;
                generated_new_texture = false;
            }
            else {
                // 累积数据
                unsigned char* plain = NULL;
                int plain_length = aes_decrypt_base_to_bytes(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer, &plain);
                data_buffer.insert(data_buffer.end(), (char *)plain, (char * )plain + plain_length);
                if (plain != NULL)
                    delete[] plain;
            }
            //char client_ip[INET_ADDRSTRLEN];
            //inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            // std::cout << "Received packet from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
             //std::cout << "Data: " << buffer << std::endl;

             // 你可以在这里发送响应数据包，如果需要
             // sendto(serv_sock, buffer, recv_len, 0, (struct sockaddr*)&client_addr, client_addr_len);
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