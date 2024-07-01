from Crypto.Cipher import PKCS1_v1_5
from Crypto.Random import get_random_bytes
from Crypto.PublicKey import RSA
from Crypto.Cipher import AES
from base64 import b64decode, b64encode

AES_KEY_SIZE = 32  # 256-bit key
AES_BLOCK_SIZE = 16  # AES block size is always 16 bytes

def rsa_crypto(public_key_pem, message):
    public_key = RSA.import_key(public_key_pem)
    # 创建加密对象
    cipher = PKCS1_v1_5.new(public_key)
    # 要加密的消息
    # 加密消息
    encrypted_message = cipher.encrypt(message.encode())
    # 将加密的消息转换为base64编码的字符串
    encoded_encrypted_message = b64encode(encrypted_message)
    return encoded_encrypted_message


def aes_decrypt(ciphertext):
    # 初始化AES解密器
    cipher = AES.new(AES_KEY, AES.MODE_CBC, AES_IV)
    # 解密数据
    plaintext = cipher.decrypt(b64decode(ciphertext))
    # 去除填充数据
    pad_len = plaintext[-1]
    return plaintext[:-pad_len]


def aes_encrypt(plaintext):
    # 初始化AES加密器
    cipher = AES.new(AES_KEY, AES.MODE_CBC, AES_IV)
    # 填充数据到16字节的倍数
    pad_len = 16 - len(plaintext) % 16
    padding = bytes([pad_len] * pad_len)
    plaintext_padded = plaintext + padding
    # 加密数据
    ciphertext = cipher.encrypt(plaintext_padded)
    # 返回base64编码的密文
    return b64encode(ciphertext).decode('utf-8')


def generate_aes_key():
    # 生成AES密钥和初始向量IV
    global AES_KEY
    global AES_IV
    AES_KEY = get_random_bytes(AES_KEY_SIZE)
    AES_IV = get_random_bytes(AES_BLOCK_SIZE)
    # Print key and iv (for demonstration purposes)
    print("AES Key:", b64encode(AES_KEY))
    print("IV:", b64encode(AES_IV))
    return b64encode(AES_KEY), b64encode(AES_IV)