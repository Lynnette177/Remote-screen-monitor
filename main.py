import socket
import time
import sys
import tkinter as tk
import threading
from PyQt5.QtWidgets import QApplication, QMainWindow
from Mouse_control import *
import screen_shots
from intro import Ui_MainWindow  # 导入生成的ui文件
from crypto_util import *
from screen_shots import *

udp_signal = 1

global_frame_rate = 30

public_key_pem_recvd = ""
server_secret = ""
tcp_sock = None
udp_port = None
servers_ip = None
width = 1920
height = 1080

def on_button_click():
    global udp_signal
    udp_signal = 0
    sys.exit(0)


def tcp_shake_hand(ip, port):
    global public_key_pem_recvd
    global tcp_sock
    global udp_port
    global width
    global height
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
        print(str_to_response)
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
            width, height = get_w_h()
            aspect_ratio = width/height
            sock.sendall(aes_encrypt(f"Correct;{aspect_ratio:.2f}".encode()).encode())
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


def heart_beat():
    global global_frame_rate
    global tcp_sock
    global width
    global height
    try:
        while True:
            tcp_sock.sendall(aes_encrypt(b"HeartBeat").encode())
            buffer = b""
            while True:
                data = tcp_sock.recv(1024)
                if not data:
                    break
                buffer += data
                if buffer.endswith(b"\x01"):
                    buffer = buffer[:-1]
                    decrypted_result = aes_decrypt(buffer)
                    parts = decrypted_result.decode().split(';')
                    part1 = parts[0]
                    part2 = parts[1]
                    part3 = parts[2]
                    if not part3.startswith('O'):
                        control_click(part3, width, height)
                    if not part1.isdigit():
                        break
                    frame_rate = int(part1)
                    if global_frame_rate != frame_rate:
                        global_frame_rate = frame_rate
                    if part2 == 'M':
                        screen_shots.main_monitoring = True
                    break
            time.sleep(0.2)
    except Exception as e:
        print(e)
        return False



def close_tcp():
    try:
        tcp_sock.close()
        return True
    except:
        return False


def udp_client(server_ip, server_port, chunk_size=700):
    # 创建UDP套接字
    global global_frame_rate
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        while udp_signal:
            screen_pic = get_screen_pic_byte_array()
            chunks = [screen_pic[i:i + chunk_size] for i in range(0, len(screen_pic), chunk_size)]
            # 发送每个小块
            for chunk in chunks:
                encrypted_chuck = aes_encrypt(chunk)
                sent = sock.sendto(encrypted_chuck.encode(), (server_ip, server_port))
            # 发送一个结束包，内容可以是一个特定的标识符“-!END”
            end_message = b'-!END'
            sent = sock.sendto(end_message, (server_ip, server_port))

            time.sleep(1/global_frame_rate)

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
    heartbeat_thread = threading.Thread(target=heart_beat, args=())
    thread.daemon = True
    heartbeat_thread.daemon = True
    thread.start()
    heartbeat_thread.start()
    root.mainloop()

