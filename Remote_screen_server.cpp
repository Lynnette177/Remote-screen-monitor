#include "server.hpp"
#include "key_gen.hpp"
#include "crypto.h"
#include <iostream>
int main()
{
    keygen(private_key,public_key);

    const wchar_t* Server_Address = L"127.0.0.1";
    int Server_Port = 5005;
    Server server(Server_Address, Server_Port);
    std::thread serverThread(&Server::start_server, &server);
    serverThread.detach(); //线程在后台运行
    while (!GetAsyncKeyState(VK_F1)) {}
    server.stop();
    return 0;

    system("pause");
}
