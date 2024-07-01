import socket
from crypto_util import *

public_key_pem_recvd = ""
server_secret = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1"

def tcp_client(ip, port):
    global public_key_pem_recvd
    # 创建TCP/IP套接字
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 连接服务器
    server_address = (ip, port)
    print(f"连接到 {server_address}")
    sock.connect(server_address)
    try:
        # 用于累积接收到的数据
        buffer = b""
        while True:
            data = sock.recv(1024)
            if not data:
                break
            buffer += data
            if buffer.endswith(b"\x01"):
                buffer = buffer[:-1]
                if buffer.endswith(b"-----END PUBLIC KEY-----\n") and buffer.startswith(b"-----BEGIN PUBLIC KEY-----"):
                    print(f"收到了完整公钥\n")
                    public_key_pem_recvd = buffer.decode()
                    break
                else:
                    buffer = b""
        aes_key, aes_iv = generate_aes_key()
        plain_text = f"test;crack;{aes_key.decode()};{aes_iv.decode()}"
        str_to_response = rsa_crypto(public_key_pem_recvd, plain_text)
        sock.sendall(str_to_response)

        buffer = b""
        decrypted_result = b""
        while True:
            data = sock.recv(1024)
            if not data:
                break
            buffer += data
            if buffer.endswith(b"\x01"):
                buffer = buffer[:-1]
                decrypted_result = aes_decrypt(buffer)
                print(decrypted_result)
                break

        print(decrypted_result)
        if decrypted_result.decode() == server_secret:
            sock.sendall(aes_encrypt(b"Correct").encode())


    finally:
        # 关闭连接
        print("关闭连接")
        sock.close()


if __name__ == "__main__":
    tcp_client('127.0.0.1', 5005)
# 使用示例
#
