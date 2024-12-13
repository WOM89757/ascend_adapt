# coding=utf-8

import cv2  # 图片处理三方库，用于对图片进行前后处理
import numpy as np  # 用于对多维数组进行计算
import torch  # 深度学习运算框架，此处主要用来处理数据

from mindx.sdk import Tensor  # mxVision 中的 Tensor 数据结构
from mindx.sdk import base  # mxVision 推理接口

from det_utils import get_labels_from_txt, letterbox, scale_coords, nms, draw_bbox  # 模型前后处理相关函数


def preprocess_warpAffine(image, dst_width=640, dst_height=640):
    scale = min((dst_width / image.shape[1], dst_height / image.shape[0]))
    ox = (dst_width  - scale * image.shape[1]) / 2
    oy = (dst_height - scale * image.shape[0]) / 2
    M = np.array([
        [scale, 0, ox],
        [0, scale, oy]
    ], dtype=np.float32)
    
    img_pre = cv2.warpAffine(image, M, (dst_width, dst_height), flags=cv2.INTER_LINEAR,
                             borderMode=cv2.BORDER_CONSTANT, borderValue=(114, 114, 114))
    IM = cv2.invertAffineTransform(M)

    img_pre = (img_pre[...,::-1] / 255.0).astype(np.float32)
    img_pre = img_pre.transpose(2, 0, 1)[None]
    img_pre = torch.from_numpy(img_pre)
    return img_pre, IM

def iou(box1, box2):
    def area_box(box):
        return (box[2] - box[0]) * (box[3] - box[1])

    left   = max(box1[0], box2[0])
    top    = max(box1[1], box2[1])
    right  = min(box1[2], box2[2])
    bottom = min(box1[3], box2[3])
    cross  = max((right-left), 0) * max((bottom-top), 0)
    union  = area_box(box1) + area_box(box2) - cross
    if cross == 0 or union == 0:
        return 0
    return cross / union

def NMS(boxes, iou_thres):

    remove_flags = [False] * len(boxes)

    keep_boxes = []
    for i, ibox in enumerate(boxes):
        if remove_flags[i]:
            continue

        keep_boxes.append(ibox)
        for j in range(i + 1, len(boxes)):
            if remove_flags[j]:
                continue

            jbox = boxes[j]
            if iou(ibox, jbox) > iou_thres:
                remove_flags[j] = True
    return keep_boxes

import sys
def postprocess(pred, IM=[], conf_thres=0.25, iou_thres=0.45):

    # 输入是模型推理的结果，即8400个预测框
    # 1,8400,56 [cx,cy,w,h,conf,17*3]
    # print(pred.shape)
    boxes = []
    for img_id, box_id in zip(*np.where(pred[...,4] > conf_thres)):
        item = pred[img_id, box_id]
        # print(item)
        # sys.exit()
        cx, cy, w, h, conf = item[:5]
        left    = cx - w * 0.5
        top     = cy - h * 0.5
        right   = cx + w * 0.5
        bottom  = cy + h * 0.5
        keypoints = item[5:].reshape(-1, 3)
        keypoints[:, 0] = keypoints[:, 0] * IM[0][0] + IM[0][2]
        keypoints[:, 1] = keypoints[:, 1] * IM[1][1] + IM[1][2]
        boxes.append([left, top, right, bottom, conf, *keypoints.reshape(-1).tolist()])

    boxes = np.array(boxes)
    lr = boxes[:,[0, 2]]
    tb = boxes[:,[1, 3]]
    boxes[:,[0,2]] = IM[0][0] * lr + IM[0][2]
    boxes[:,[1,3]] = IM[1][1] * tb + IM[1][2]
    boxes = sorted(boxes.tolist(), key=lambda x:x[4], reverse=True)
    
    return NMS(boxes, iou_thres)

def hsv2bgr(h, s, v):
    h_i = int(h * 6)
    f = h * 6 - h_i
    p = v * (1 - s)
    q = v * (1 - f * s)
    t = v * (1 - (1 - f) * s)
    
    r, g, b = 0, 0, 0

    if h_i == 0:
        r, g, b = v, t, p
    elif h_i == 1:
        r, g, b = q, v, p
    elif h_i == 2:
        r, g, b = p, v, t
    elif h_i == 3:
        r, g, b = p, q, v
    elif h_i == 4:
        r, g, b = t, p, v
    elif h_i == 5:
        r, g, b = v, p, q

    return int(b * 255), int(g * 255), int(r * 255)

def random_color(id):
    h_plane = (((id << 2) ^ 0x937151) % 100) / 100.0
    s_plane = (((id << 3) ^ 0x315793) % 100) / 100.0
    return hsv2bgr(h_plane, s_plane, 1)


# 初始化资源和变量
base.mx_init()  # 初始化 mxVision 资源
DEVICE_ID = 0  # 设备id
model_path = '/home/HwHiAiUser/code/ascend/yolov8-pose/yolov8s-pose_bs1.om'  # 模型路径
# image_path = '/home/HwHiAiUser/code/ascend/yolo_sdk_python_sample/world_cup.jpg'  # 测试图片路径
image_path = '/home/HwHiAiUser/code/ascend/yolo_sdk_python_sample/bus.jpg'  # 测试图片路径
# image_path = '/home/HwHiAiUser/code/ascend/yolo_sdk_python_sample/zidane.jpg'  # 测试图片路径

# 数据前处理
img_bgr = cv2.imread(image_path, cv2.IMREAD_COLOR)  # 读入图片
img, IM = preprocess_warpAffine(img_bgr)

# img, scale_ratio, pad_size = letterbox(img_bgr, new_shape=[640, 640])  # 对图像进行缩放与填充，保持长宽比
# img = img[:, :, ::-1].transpose(2, 0, 1)  # BGR to RGB, HWC to CHW
# img = np.expand_dims(img, 0).astype(np.float32)  # 将形状转换为 channel first (1, 3, 640, 640)，即扩展第一维为 batchsize

# # img = np.expand_dims(img, axis=0)  # 扩展第一维度，适应模型输入
# img = np.ascontiguousarray(img) / 255.0  # 转换为内存连续存储的数组
img = np.ascontiguousarray(img, dtype=np.float16)
# img = img.astype('uint8')
print(img.shape)
img = Tensor(img) # 将numpy转为转为Tensor类

# 模型推理, 得到模型输出
model = base.model(modelPath=model_path, deviceId=DEVICE_ID)  # 初始化 base.model 类
keypoints = model.infer([img])[0]
print(keypoints.shape)

keypoints.to_host()
keypoints = np.array(keypoints)
keypoints = torch.Tensor(keypoints)

# 假设keypoints的形状为 (1, num_keypoints, 3)，需要去除batch维度
# keypoints = keypoints[0]
# print(keypoints)
print(keypoints.shape)
result = keypoints.transpose(-1, -2)  # 1,8400,56
# result = keypoints  # 1,8400,56
print(result.shape)

# print(img_bgr.shape[:2])

boxes = postprocess(result, IM)
names = "-"
skeleton = [[16, 14], [14, 12], [17, 15], [15, 13], [12, 13], [6, 12], [7, 13], [6, 7], [6, 8], 
            [7, 9], [8, 10], [9, 11], [2, 3], [1, 2], [1, 3], [2, 4], [3, 5], [4, 6], [5, 7]]
pose_palette = np.array([[255, 128, 0], [255, 153, 51], [255, 178, 102], [230, 230, 0], [255, 153, 255],
                         [153, 204, 255], [255, 102, 255], [255, 51, 255], [102, 178, 255], [51, 153, 255],
                         [255, 153, 153], [255, 102, 102], [255, 51, 51], [153, 255, 153], [102, 255, 102],
                         [51, 255, 51], [0, 255, 0], [0, 0, 255], [255, 0, 0], [255, 255, 255]],dtype=np.uint8)
kpt_color  = pose_palette[[16, 16, 16, 16, 16, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9]]
limb_color = pose_palette[[9, 9, 9, 9, 7, 7, 7, 0, 0, 0, 0, 0, 16, 16, 16, 16, 16, 16, 16]]

print('resuts size {}'.format(len(boxes)))
for box in boxes:
    left, top, right, bottom = int(box[0]), int(box[1]), int(box[2]), int(box[3])
    confidence = box[4]
    label = 0
    color = random_color(label)
    cv2.rectangle(img_bgr, (left, top), (right, bottom), color, 2, cv2.LINE_AA)
    caption = f"{names[label]} {confidence:.2f}"
    w, h = cv2.getTextSize(caption, 0, 1, 2)[0]
    cv2.rectangle(img_bgr, (left - 3, top - 33), (left + w + 10, top), color, -1)
    cv2.putText(img_bgr, caption, (left, top - 5), 0, 1, (0, 0, 0), 2, 16)
    
    keypoints = box[5:]
    keypoints = np.array(keypoints).reshape(-1, 3)
    for i, keypoint in enumerate(keypoints):
        x, y, conf = keypoint
        color_k = [int(x) for x in kpt_color[i]]
        if conf < 0.5:
            continue
        if x != 0 and y != 0:
            cv2.circle(img_bgr, (int(x), int(y)), 5, color_k, -1, lineType=cv2.LINE_AA)
    
    for i, sk in enumerate(skeleton):
        pos1 = (int(keypoints[(sk[0] - 1), 0]), int(keypoints[(sk[0] - 1), 1]))
        pos2 = (int(keypoints[(sk[1] - 1), 0]), int(keypoints[(sk[1] - 1), 1]))

        conf1 = keypoints[(sk[0] - 1), 2]
        conf2 = keypoints[(sk[1] - 1), 2]
        if conf1 < 0.5 or conf2 < 0.5:
            continue
        if pos1[0] == 0 or pos1[1] == 0 or pos2[0] == 0 or pos2[1] == 0:
            continue
        cv2.line(img_bgr, pos1, pos2, [int(x) for x in limb_color[i]], thickness=2, lineType=cv2.LINE_AA)
    
cv2.imwrite('result.png', img_bgr)
print("save done")