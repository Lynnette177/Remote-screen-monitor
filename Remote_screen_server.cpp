#include "server.hpp"
#include "key_gen.hpp"
#include "crypto.h"
#include <iostream>
int main()
{
    char* private_key;
    char * public_key;
    private_key = new char[KEY_BUFFER_SIZE];
    public_key = new char[KEY_BUFFER_SIZE];
    memset(private_key, 0, KEY_BUFFER_SIZE);
    memset(public_key, 0, KEY_BUFFER_SIZE);
    keygen(private_key,public_key);
    printf("%s\n%s", private_key, public_key);
    unsigned char plaintext[RSA_PLAINTEXT_SIZE] = "test\0";
    test(public_key, private_key,plaintext, RSA_PLAINTEXT_SIZE);
    system("pause");
}
