import socket
import time
from crypto_util import *
from screen_shots import *


public_key_pem_recvd = ""
server_secret = "askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1"
tcp_sock = None
udp_port = None


def tcp_shake_hand(ip, port):
    global public_key_pem_recvd
    global tcp_sock
    global udp_port
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

        parts = decrypted_result.decode().split(';')
        part1 = parts[0]
        part2 = parts[1]
        if part1 == server_secret:
            sock.sendall(aes_encrypt(b"Correct").encode())
            tcp_sock = sock
            udp_port = int(part2)
            return True
        else:
            sock.sendall(aes_encrypt(b"Failed.").encode())
            return False


    except Exception as e:
        print(f"发送错误，关闭套接字{e}")
        sock.close()
        return False

def close_tcp():
    try:
        tcp_sock.close()
        return True
    except:
        return False


def udp_client(server_ip, server_port, chunk_size=1024):
    # 创建UDP套接字
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        while True:
            screen_pic = get_screen_pic_byte_array()
            chunks = [screen_pic[i:i + chunk_size] for i in range(0, len(screen_pic), chunk_size)]
            # 发送每个小块
            for chunk in chunks:
                sent = sock.sendto(chunk, (server_ip, server_port))
            # 发送一个结束包，内容可以是一个特定的标识符“END”
            end_message = b'END'
            sent = sock.sendto(end_message, (server_ip, server_port))

            time.sleep(1)

    finally:
        # 关闭套接字
        print("Closing socket")
        sock.close()


if __name__ == "__main__":
    tcp_shake_hand('127.0.0.1', 5005)
    print(udp_port)
    close_tcp()
    udp_client('127.0.0.1',udp_port)

