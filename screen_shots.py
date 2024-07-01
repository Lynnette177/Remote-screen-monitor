from PIL import Image
import pyautogui
import io


def get_screen_pic_byte_array():
    # 获取截图
    screenshot = pyautogui.screenshot()
    # 降低分辨率
    screenshot = screenshot.resize((screenshot.width // 6, screenshot.height // 6), Image.ANTIALIAS)
    # 将截图保存到字节流中，以JPEG格式，降低质量
    byte_io = io.BytesIO()
    screenshot.save(byte_io, format='JPEG', quality=20)  # 质量参数可以根据需要调整
    # 获取字节数据
    byte_data = byte_io.getvalue()
    # 将字节数据转换为 bytearray
    pic_byte_array = bytearray(byte_data)
    byte_io.close()
    #print(len(pic_byte_array))
    return pic_byte_array
