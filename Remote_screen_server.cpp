﻿#include "global.h"
#include "server.hpp"
#include "key_gen.hpp"
#include "crypto.h"
#include <iostream>
#include "UI.h"
int main()
{
    // 随机生成RSA公钥和私钥
    keygen(private_key,public_key);


    // 定义IP地址和端口
    const wchar_t* Server_Address = L"127.0.0.1";
    int Server_Port = 5005;

    // 初始化Server对象
    Server server(Server_Address, Server_Port);
    // 创建线程执行Serve类成员函数start_server
    std::thread serverThread(&Server::start_server, &server);
    // 线程在后台运行
    serverThread.detach(); 
    //启动绘制
    UI_imgui::Render();
    //绘制结束了代表用户关闭了菜单，停止服务器并退出即可
    server.stop();
    return 0;

    system("pause");
}
