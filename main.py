import socket
import time
import sys
import tkinter as tk
import threading
from PyQt5.QtWidgets import QApplication, QMainWindow
from intro import Ui_MainWindow  # 导入生成的ui文件
from crypto_util import *
from screen_shots import *

udp_signal = 1

public_key_pem_recvd = ""
server_secret = ""
tcp_sock = None
udp_port = None
servers_ip = None


def on_button_click():
    global udp_signal
    udp_signal = 0
    exit(0)


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
        while udp_signal:
            screen_pic = get_screen_pic_byte_array()
            chunks = [screen_pic[i:i + chunk_size] for i in range(0, len(screen_pic), chunk_size)]
            # 发送每个小块
            for chunk in chunks:
                sent = sock.sendto(chunk, (server_ip, server_port))
            # 发送一个结束包，内容可以是一个特定的标识符“END”
            end_message = b'END'
            sent = sock.sendto(end_message, (server_ip, server_port))

            time.sleep(0.03)

    finally:
        # 关闭套接字
        print("Closing socket")
        sock.close()


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)
        self.lineEdit.setText("127.0.0.1")  # 设置服务器IP的初始值
        self.lineEdit_3.setText("5005")  # 设置服务器端口的初始值
        self.lineEdit_2.setText("askfkhAOSIDIUHkljdhfskjgMNCMZPSDFI2KASDa1")  # 设置服务器密钥的初始值
        self.pushButton.clicked.connect(self.on_button_click)

    def on_button_click(self):
        global server_secret
        global servers_ip
        server_ip = self.lineEdit.text()
        server_port = self.lineEdit_3.text()
        server_key = self.lineEdit_2.text()
        print(f"IP: {server_ip}, Port: {server_port}, Key: {server_key}")
        servers_ip = server_ip
        server_secret = server_key
        tcp_shake_hand(server_ip, int(server_port))
        if udp_port is not None:
            self.close()


if __name__ == "__main__":
    #tcp_shake_hand('127.0.0.1', 5005)
    #print(udp_port)
    #close_tcp()
    #udp_client('127.0.0.1',udp_port)
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit_code = app.exec_()
    # close_tcp()
    root = tk.Tk()

    # 设置窗口属性
    root.overrideredirect(True)  # 隐藏标题栏
    root.attributes('-alpha', 0.7)  # 设置透明度，0为完全透明，1为不透明
    root.attributes('-topmost', True)  # 窗口始终置于最顶层
    root.configure(bg='#333333')  # 设置窗口背景色为浅黑色，可以使用十六进制颜色码或颜色名称

    # 创建文本标签
    label = tk.Label(root, text="正在进行屏幕监控", font=("Helvetica", 12), fg='white', bg='#333333')
    label.pack(padx=10, pady=20)

    # 创建按钮
    button = tk.Button(root, text="点击退出", command=on_button_click)
    button.pack(pady=10)

    # 让窗口显示在屏幕中心
    window_width = 150
    window_height = 100
    screen_width = root.winfo_screenwidth()
    screen_height = root.winfo_screenheight()
    x_coordinate = int((screen_width - window_width))
    y_coordinate = int((screen_height - window_height)-100)
    root.geometry(f'{window_width}x{window_height}+{x_coordinate}+{y_coordinate}')
    thread = threading.Thread(target=udp_client, args=(servers_ip, udp_port))
    thread.start()
    root.mainloop()

