from PIL import Image
import pyautogui
import io

main_monitoring = False


def get_w_h():  # 获取屏幕宽高
    screenshot = pyautogui.screenshot()
    return screenshot.width, screenshot.height


def get_screen_pic_byte_array():  #截图
    global main_monitoring
    # 获取截图
    screenshot = pyautogui.screenshot()
    division = 1
    quality = 80
    if not main_monitoring:
        quality = 20
        division = 6
    # 降低分辨率
    screenshot = screenshot.resize((screenshot.width // division, screenshot.height // division))
    # 将截图保存到字节流中，以JPEG格式，降低质量
    byte_io = io.BytesIO()
    screenshot.save(byte_io, format='JPEG', quality=quality)  # 质量参数根据服务器是否需要而不同
    # 获取字节数据
    byte_data = byte_io.getvalue()
    # 将字节数据转换为 bytearray
    pic_byte_array = bytearray(byte_data)
    byte_io.close()
    #print(len(pic_byte_array))
    return pic_byte_array
