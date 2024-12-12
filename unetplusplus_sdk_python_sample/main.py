import numpy as np  # 用于对多维数组进行计算
import cv2  # 图片处理三方库，用于对图片进行前后处理
from albumentations.augmentations import transforms  # 数据增强库，此处用于对像素值进行值域变换

from mindx.sdk import Tensor  # mxVision 中的 Tensor 数据结构
from mindx.sdk import base  # mxVision 推理接口

def sigmoid(x):
    y = 1.0 / (1 + np.exp(-x))  # 对矩阵的每个元素执行 1/(1+e^(-x))
    return y

def plot_mask(img, msk):
    """ 将推理得到的 mask 覆盖到原图上 """
    msk = msk + 0.5  # 将像素值范围变换到 0.5~1.5, 有利于下面转为二值图
    msk = cv2.resize(msk, (img.shape[1], img.shape[0]))  # 将 mask 缩放到原图大小
    msk = np.array(msk, np.uint8)  # 转为二值图, 只包含 0 和 1

    # 从 mask 中找到轮廓线, 其中第二个参数为轮廓检测的模式, 第三个参数为轮廓的近似方法
    # cv2.RETR_EXTERNAL 表示只检测外轮廓,  cv2.CHAIN_APPROX_SIMPLE 表示压缩水平方向、
    # 垂直方向、对角线方向的元素, 只保留该方向的终点坐标, 例如一个矩形轮廓只需要4个点来保存轮廓信息
    # contours 为返回的轮廓（list）
    contours, _ = cv2.findContours(msk, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # 在原图上画出轮廓, 其中 img 为原图, contours 为检测到的轮廓列表
    # 第三个参数表示绘制 contours 中的哪条轮廓, -1 表示绘制所有轮廓
    # 第四个参数表示颜色, （0, 0, 255）表示红色, 第五个参数表示轮廓线的宽度
    cv2.drawContours(img, contours, -1, (0, 0, 255), 1) 

    # 将轮廓线以内（即分割区域）覆盖上一层红色
    img[..., 2] = np.where(msk == 1, 255, img[..., 2])

    return img


# 初始化资源和变量
base.mx_init()  # 初始化 mxVision 资源
pic_path = 'img.png'  # 单张图片
model_path = "model/unetplusplus.om"  # 模型路径
num_class = 1  # 类别数量, 需要根据模型结构、任务类别进行改变; 此处我们只分割出细胞, 即为一分类
device_id = 0  # 指定运算的Device

# 前处理
img_bgr = cv2.imread(pic_path)  # 读入图片
img = cv2.resize(img_bgr, (96, 96))  # 将原图缩放到 96*96 大小
img = transforms.Normalize().apply(img)  # 将像素值标准化（减去均值除以方差）
img = img.astype('float32') / 255  # 将像素值缩放到 0~1 范围内
img = img.transpose(2, 0, 1)  # 将形状转换为 channel first (3, 96, 96)
img = np.expand_dims(img, 0)  # 将形状转换为 (1, 3, 96, 96)，即扩展第一维为 batchsize
img = np.ascontiguousarray(img)  # 将内存连续排列
img = Tensor(img) # 将numpy转为转为Tensor类

# 模型推理
model = base.model(modelPath=model_path, deviceId=device_id)  # 初始化 base.model 类
outputs = model.infer([img])  # 执行推理。输入数据类型：List[base.Tensor]， 返回模型推理输出的 List[base.Tensor]

# 后处理
model_out_msk = outputs[0]  # 取出模型推理结果, 推理结果形状为 (1, 1, 96, 96),即（batchsize, num_class, height, width）
model_out_msk.to_host()  # 移动tensor到内存中
model_out_msk = np.array(model_out_msk)  # 将 base.Tensor 类转为numpy array
model_out_msk = sigmoid(model_out_msk[0][0])  # 利用 sigmoid 将模型输出变换到 0~1 范围内
img_to_save = plot_mask(img_bgr, model_out_msk)  # 将处理后的输出画在原图上, 并返回

# 保存图片到文件
cv2.imwrite('result.png', img_to_save)  
print('save infer result success')
