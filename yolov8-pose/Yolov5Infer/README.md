# 基于C++ V2接口的yoloV3推理

## 1 简介

基于C++ V2接口的yoloV3推理样例使用mxVision SDK进行开发，以昇腾Atlas 200I/500 A2和Atlas 推理系列产品为主要的硬件平台，主要支持以下功能：

1. 图片读取解码：本样例支持JPG及PNG格式图片，使用图像处理单元进行解码。
2. 图片缩放/保存：使用图像处理单元相关接口进行图片的缩放，并输出一份缩放后的副本保存。
3. 模型推理：使用yoloV3网络识别输入图片中对应的目标，并打印输出大小。
4. 模型后处理：使用SDK中的模型后处理接口对推理结果进行计算，并输出相关结果，

### 1.1 支持的产品

昇腾Atlas 200I/500 A2和Atlas 推理系列产品


### 1.2 支持的版本

| 软件名称     | 版本              |
|----------|-----------------|
| cmake    | 3.10+           |
| mxVision | 5.0RC3及之后版本     |
| Python   | 3.9.2+          |
| CANN     | 请使用mxVision配套版本 |



### 1.3 代码主要目录介绍

本代码仓名称为mxSdkReferenceApps，工程目录如下图所示：

```
|-- YOLOV3CPPV2
|   |-- CMakeLists.txt
|   |-- main.cpp
|   |-- README.md
|   |-- run.sh
|   |-- test.jpg    #测试使用的图片
|   |-- model
|   |   |-- yolov3_tf_bs1_fp16.cfg
|   |   |-- aipp_yolov3_416_416.aippconfig
|   |   |-- yolov3_tf_bs1_fp16.OM       #OM模型需要自行下载转换
|   |   |-- yolov3.names

```
## 2 设置环境变量

```bash
# 设置环境变量（请确认SDK_INSTALL_PATH路径是否正确）
. /usr/local/Ascend/ascend-toolkit/set_env.sh #toolkit默认安装路径，根据实际安装路径修改
. ${SDK_INSTALL_PATH}/mxVision/set_env.sh
```

## 3 准备模型

**步骤1**：在ModelZoo上下载YOLOv3模型。[下载地址](https://gitee.com/link?target=https%3A%2F%2Fobs-9be7.obs.cn-east-2.myhuaweicloud.com%2F003_Atc_Models%2Fmodelzoo%2Fyolov3_tf.pb)

**步骤2**：将获取到的YOLOv3模型pb文件存放至`./model/`。


## 4 编译与运行

**步骤1**：简易运行样例
```bash
bash run.sh

atc --model=./yolov3_tf.pb --framework=3 --output=./yolov3_tf_bs1_fp16 --soc_version=Ascend310P3 --insert_op_conf=./aipp_yolov3_416_416.aippconfig --input_shape="input:1,416,416,3" --out_nodes="yolov3/yolov3_head/Conv_6/BiasAdd:0;yolov3/yolov3_head/Conv_14/BiasAdd:0;yolov3/yolov3_head/Conv_22/BiasAdd:0
```

**步骤2**：查看结果
> 运行结束时，会打印检测的相关信息

## 5 其他
对于需要自行开发后处理的用户，请修改YoloV3PostProcess函数完成对应功能