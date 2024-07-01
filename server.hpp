#pragma once
#include "includes.h"

class ClientHandler {
private:
    SOCKET clnt_control_sock;
    SOCKET clnt_data_sock;

public:
    ClientHandler(SOCKET _clnt_control_sock) : clnt_control_sock(_clnt_control_sock) {
        printClientInfo(true);
    }

    bool send_message(const char* message, int command = 0) {
        try {
            
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

    void printClientInfo(bool init = false) {
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

    void handle() {
        try {
            char buffer[1024] = {};
            int bytes_received;
            while ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }

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