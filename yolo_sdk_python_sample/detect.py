import cv2
import numpy as np
import torch
import os
from det_utils import letterbox, nms, scale_coords
# from ais_bench.infer.interface import InferSession
from time import time

import sys
sys.path.append("/home/HwHiAiUser/code/samples/inference/acllite/python/")


from acllite_model import AclLiteModel
from acllite_resource import AclLiteResource

model_path = "./model/yolov5s_bs1.om"  # om格式模型文件
label_path = './coco_names.txt'  # 标签

detect = '/home/HwHiAiUser/code/ascend/yolo_sdk_python_sample/world_cup.jpg'  # 输入文件or目录
result = './result.jpg'  # 输出文件or目录

def preprocess_image(image, cfg, bgr2rgb=True):  # 图片预处理
    img, scale_ratio, pad_size = letterbox(image, new_shape=cfg['input_shape'])  # image尺度不定，故需调整尺寸适配模型输入
    if bgr2rgb:
        img = img[:, :, ::-1]
    img = img.transpose(2, 0, 1)  # HWC2CHW
    img = np.ascontiguousarray(img, dtype=np.float32)  # 将输入数组转换为连续存储数组，加速运算效率
    return img, scale_ratio, pad_size


def draw_bbox(bbox, img0, color, wt, names):
    """在图片上画预测框"""
    det_result_str = ''
    for idx, class_id in enumerate(bbox[:, 5]):
        if float(bbox[idx][4] < float(0.05)):
            continue
        img0 = cv2.rectangle(img0, (int(bbox[idx][0]), int(bbox[idx][1])), (int(bbox[idx][2]), int(bbox[idx][3])),
                             color, wt)
        img0 = cv2.putText(img0, str(idx) + ' ' + names[int(class_id)], (int(bbox[idx][0]), int(bbox[idx][1] + 16)),
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
        img0 = cv2.putText(img0, '{:.4f}'.format(bbox[idx][4]), (int(bbox[idx][0]), int(bbox[idx][1] + 32)),
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
        det_result_str += '{} {} {} {} {} {}\n'.format(
            names[bbox[idx][5]], str(bbox[idx][4]), bbox[idx][0], bbox[idx][1], bbox[idx][2], bbox[idx][3])
    return img0


def get_labels_from_txt(path):
    """从txt文件获取图片标签"""
    labels_dict = dict()
    with open(path) as f:
        for cat_id, label in enumerate(f.readlines()):
            labels_dict[cat_id] = label.strip()
    return labels_dict


def detect_img(model, detect_path, result_path):
    raw_img = cv2.imread(detect_path)  # 载入原始图片
    labels = get_labels_from_txt(label_path)
    # 预处理
    cfg = {
        'conf_thres': 0.4,  # 模型置信度阈值，阈值越低，得到的预测框越多
        'iou_thres': 0.5,  # IOU阈值，重叠率过低的框会被过滤
        'input_shape': [640, 640],  # 输入尺寸
    }
    img, scale_ratio, pad_size = preprocess_image(raw_img, cfg)
    img = img / 255.0  # 训练模型时将0~255值域转化为了0~1，故推理阶段也需同样处理

    # 检测
    t1 = time()
    # output = model.infer([img])[0]
    print(img.shape)
    # output = model.execute(img,])[0]
    output = model.execute([img,])[0]
    print(output)
    # print(len(output))
    print(output.shape)
    output = torch.tensor(output)

    # 非极大值抑制后处理
    boxout = nms(output, conf_thres=cfg["conf_thres"], iou_thres=cfg["iou_thres"])
    pred_all = boxout[0].numpy()
    # 预测坐标转换
    print(pred_all)
    print(pred_all.shape)
    scale_coords(cfg['input_shape'], pred_all[:, :4], raw_img.shape, ratio_pad=(scale_ratio, pad_size))
    t2 = time()
    print("detect time: %fs" % (t2 - t1))

    # 跟踪结果保存
    draw_bbox(pred_all, raw_img, (0, 255, 0), 2, labels)
    cv2.imwrite(result_path, raw_img)


if __name__ == "__main__":
    # model = InferSession(0, model_path)

    #ACL resource initialization
    acl_resource = AclLiteResource()
    acl_resource.init()
    #load model
    model = AclLiteModel(model_path)

    detect_img(model, detect, result)
    print('Detect OK!')
