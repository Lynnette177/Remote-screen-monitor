#include "server.hpp"
#include "key_gen.hpp"
#include "crypto.h"
#include <iostream>
int main()
{
    keygen(private_key,public_key);
    printf("%s\n%s", private_key, public_key);
    unsigned char plaintext[RSA_PLAINTEXT_SIZE] = "test\0";
    test(public_key, private_key,plaintext, RSA_PLAINTEXT_SIZE);

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
