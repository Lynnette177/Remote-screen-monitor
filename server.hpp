#pragma once
#include "global.h"
#include "includes.h"
#include "dx11imageloader.h"
#include "crypto.h"
class ClientHandler {
    //ClientHandler类 每个客户端分配一个 用来处理与客户端之间的事务
private:
    SOCKET clnt_control_sock;//控制信息传输socket
    SOCKET clnt_data_sock;//数据传输socket
    int udp_port = 0;//为该客户分配的UDP端口号
    userInfo info_struct;//该客户信息的结构体
    std::thread hb_thread;//心跳和指令发送的线程
    std::vector<std::uint8_t> data_buffer;//图片缓冲区容器
public:
    bool show_history = false;//是否新建一个窗口展示截图
    bool able_to_save = false;//是否能够保存截图
    std::mutex image_lock;//图片容器读写互斥锁
    std::mutex command_lock;//指令读写的互斥锁
    std::string id;//客户身份 ID 保存文件时作为名字
    int command = 0;//鼠标指令，012分别为无 左键 右键
    int x = 0;//鼠标坐标
    int y = 0;//鼠标坐标
    std::string client_info;//要绘制出来的基本信息 text
    int frame_rate = 10;//客户端监控帧率
    bool main_monitoring = false;//是否启用高分辨率
    std::vector<std::uint8_t> image_data;//生成纹理用的图片数据容器
    std::vector<std::pair<std::string, Texture>> pic_textures;//历史记录图片容器
    bool generated_new_texture = false;//是否已经为图片生成过了纹理
    Texture thumb_texture;//图片纹理
    float aspect_ratio = 2.f;//客户端屏幕宽高比
    bool online = false;//客户是否在线
    uint64_t offline_time = 0;//客户离线时间
    bool do_delete_this_client = false;//如果用户点击了删除，置为真，结束线程回收内存
    bool offline_too_long_able_to_delete = false;//如果离线时间超过十秒，允许删除
    bool off_line_pic_generated = false;//客户离线的灰色截图已经生成过
    bool stop_hb_thread = false;//是否结束心跳线程
    bool save_image = false;//是否保存屏幕截图

    ClientHandler(SOCKET _clnt_control_sock) : clnt_control_sock(_clnt_control_sock) {
        printClientInfo(true);
    }
    void show_private_info() {//展示用户私有信息调试用
        std::cout << "\nUsername: " << info_struct.name << std::endl;
        std::cout << "Password: " << info_struct.password << std::endl;
        std::cout << "MAC Address: " << info_struct.mac << std::endl;
        std::cout << "Key: " << info_struct.aes_key << std::endl;
        std::cout << "IV: " << info_struct.aes_iv << std::endl;
    }
    bool send_control_message(const char* message) {
        //通过TCP SOCKET发送控制信息。TCP需要自己设置消息边界，这里以0x01作为消息的结束。并加入了异常处理
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
    void printClientInfo(bool init = false) {//打印客户端连接信息 基本信息和ID保存到变量中
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
            id = info_struct.name + "---" + info_struct.mac;
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
            printClientInfo();//打印基本连接信息
            send_control_message(public_key);//发送公钥
            if ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                std::string plain_text = rsa_decrypt_base(buffer);
                parse_userInfo(&info_struct, plain_text);
                show_private_info();

                //这里可以验证客户是否合法
                if (0){//如果非法的例子 不会进入的
                    std::string info_to_send = "UNAUTHORIZED";
                    std::string cipher_mesg = aes_encrypt_base(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)info_to_send.c_str());
                    send_control_message(cipher_mesg.c_str());
                    return;
                }

                init_udp_server();
                std::string info_to_send = server_info + ";" + std::to_string(udp_port);
                std::string cipher_mesg = aes_encrypt_base(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)info_to_send.c_str());
                std::cout << cipher_mesg;
                send_control_message(cipher_mesg.c_str());
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }
            if ((bytes_received = recv(clnt_control_sock, buffer, sizeof(buffer), 0)) > 0) {
                std::cout << "\n" << buffer << "\n";
                //两种解密方式的例子 返回值不同 分别是string和指针读写:
                std::string plain_text = aes_decrypt_base_to_string(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer);
                
                unsigned char* ptext = NULL;
                aes_decrypt_base_to_bytes(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer, &ptext);

                delete[]  ptext;
                int split_index = plain_text.find(';');
                if (split_index == -1) {
                    printf("客户端认证失败\n");
                    return;
                }
                else {
                    std::string code = std::string(plain_text.substr(0, split_index));
                    std::string ratio = std::string(plain_text.substr(split_index+1, plain_text.length() - split_index+1));
                    if (code == "Correct" && std::stof(ratio) > 0.01f) {
                        printf("客户端认证成功\n");
                        this->aspect_ratio = std::stof(ratio);
                        authorized = true;
                    }
                }
                memset(buffer, 0, 1024);//处理结束清空缓冲区
            }
            if (!authorized) {
                closesocket(clnt_control_sock);
                return;
            }
            else {
                //客户端认证成功了，把指针保存到数组中方便绘制用。同时启动UDP处理，TCP控制线程，从客户端开始接收数据、收发指令。
                all_connected_clients.push_back(this);
                hb_thread = std::thread(&ClientHandler::heart_beat_detect, this);
                printClientInfo();
                udp_handler();
                //如果udp_handler退出了代表客户端已经终止了连接。
                closesocket(clnt_control_sock);
                closesocket(clnt_data_sock);
                auto it = std::remove(all_connected_clients.begin(), all_connected_clients.end(), this);
                // 使用erase删除从std::remove返回的迭代器到vector末尾的元素
                all_connected_clients.erase(it, all_connected_clients.end());
                hb_thread.join();
                return;
            }
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
        struct timeval tv;
        tv.tv_sec = 10;  // 超时时间为10秒
        tv.tv_usec = 0; // 微秒部分置为0

        if (setsockopt(clnt_data_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
            std::cerr << "Error setting socket options" << std::endl;
            return;
        }
        while (!do_delete_this_client) {
            char buffer[1024] = { 0 };
            struct sockaddr_in client_addr;
            int client_addr_len = sizeof(client_addr);

            // 接收数据包
            int recv_len = recvfrom(clnt_data_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
            if (!(recv_len > 0)) continue;
            if (new_pic) {
                data_buffer.clear();
                new_pic = false;
            }
            if (recv_len == SOCKET_ERROR) {
                std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            if (strcmp(buffer, "-!END") == 0) {
               // std::cout << "Received END, processing data..." << std::endl;
                //std::cout << "Data size: " << data_buffer.size() << " bytes" << std::endl;
                {
                    std::lock_guard<std::mutex> lock(image_lock); // 自动管理锁
                    image_data.clear();
                    image_data = data_buffer;
                    data_buffer.clear();
                    new_pic = true;
                    generated_new_texture = false;
                } // 这里锁会在作用域结束时自动释放
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
    void heart_beat_detect() {
        char buffer[1024] = {};
        // 接收数据
        while (!stop_hb_thread) {
            int recv_len = recv(clnt_control_sock, buffer, sizeof(buffer), 0);
            if (recv_len < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    //std::cerr << "Receive timeout!" << std::endl;
                    online = false;
                }
                else {
                    //std::cerr << "Receive failed! " << errno << std::endl;
                    online = false;
                }
            }
            else if (recv_len == 0) {
                //std::cout << "Connection closed by peer!" << std::endl;
                online = false;
            }
            else {
                buffer[recv_len] = '\0'; // 确保字符串以NULL结尾
                std::string plain_text = aes_decrypt_base_to_string(info_struct.aes_key, info_struct.aes_iv, (unsigned char*)buffer);
                //std::cout << "Received Decryptied: " << plain_text << std::endl;
                online = true;
                std::string contrl_msg = std::to_string(frame_rate) + ";";
                if (this->main_monitoring) contrl_msg += "M;";
                else contrl_msg += "N;";
                command_lock.lock();
                if (command == 0) {
                    contrl_msg += "O";
                }
                else if (command == 1) {
                    std::string x_co = std::to_string(x);
                    std::string y_co = std::to_string(y);
                    contrl_msg += "L" + x_co + "&" + y_co;
                }
                else if (command == 2) {
                    std::string x_co = std::to_string(x);
                    std::string y_co = std::to_string(y);
                    contrl_msg += "R" + x_co + "&" + y_co;
                }
                command = 0;
                x = 0;
                y = 0;
                command_lock.unlock();
                send_control_message(aes_encrypt_base(info_struct.aes_key, info_struct.aes_iv, (unsigned char *)contrl_msg.c_str()).c_str());
            }
            if (!online && offline_time == 0) {
                offline_time = GetTickCount64();
                off_line_pic_generated = false;
            }
            else if (online && offline_time != 0) {
                offline_time = 0;
                offline_too_long_able_to_delete = false;
            }
            else if (!online && offline_time != 0) {
                if (GetTickCount64() - offline_time > 10000) {
                    offline_too_long_able_to_delete = true;
                    break;
                }
            }
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
    //客户端私有信息。其中client_threads存储了所有客户端的线程
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

        while (!stop_server) {//循环等待客户端连接 连接后放到容器中 启动线程
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
        /*
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        */
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