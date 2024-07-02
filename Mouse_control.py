import pyautogui


def control_click(command, width, height):  #控制鼠标移动的函数
    button = command[0]
    co = command[1:]
    parts = co.split('&')
    x = int(parts[0])/1000*height
    y = int(parts[1])/1000*height
    pyautogui.moveTo(x, y, duration=0.1)  # 移动到指定位置，持续0.1秒
    if button == 'L':
        pyautogui.click()
    if button == 'R':
        pyautogui.rightClick()
