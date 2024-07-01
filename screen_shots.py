from PIL import Image
import pyautogui
import io

def get_screen_pic_byte_array():
    screenshot = pyautogui.screenshot()
    # 将截图保存到字节流中，以JPEG格式
    byte_io = io.BytesIO()
    screenshot.save(byte_io, format='JPEG', quality=85)  # 质量参数可以根据需要调整
    # 获取字节数据
    byte_data = byte_io.getvalue()
    # 将字节数据转换为 bytearray
    pic_byte_array = bytearray(byte_data)
    # 关闭字节流
    byte_io.close()
    # 检查 bytearray 的大小
    print(len(pic_byte_array))
    return pic_byte_array
