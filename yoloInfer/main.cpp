/*
 * Copyright(C) 2022. Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include "MxBase/MxBase.h"
#include "MxBase/MemoryHelper/MemoryHelper.h"
#include "MxBase/DeviceManager/DeviceManager.h"
#include "MxBase/Log/Log.h"
#include "plugins/yolov7PostProcessPlugin/Yolov7PostProcess.h"
#include "plugins/yolov8PosePostProcessPlugin/Yolov8PosePostProcess.h"
// #include "MxPlugins/ModelPostProcessors/Yolov7PostProcess.h"
#include "MxBase/postprocess/include/KeypointPostProcessors/OpenPosePostProcess.h"

#include "MxBase/E2eInfer/VideoDecoder/VideoDecoder.h"

using namespace MxBase;
using namespace std;

const int MODEL_INPUT_WIDTH = 640;
const int MODEL_INPUT_HEIGHT = 640;
const int RGB_EXTEND = 3;
const int PAD_COLOR = 114;
const int OPENCV_8UC3 = 16;
const int YUV_DIVISION = 2;
const int R_CHANNEL = 2;
const int AVG_PARAM = 2;
const int ARG_NUM = 12;
const long MAX_FILE_SIZE = 1024 * 1024 * 1024; // 1g

const float YUV_Y_R = 0.299;
const float YUV_Y_G = 0.587;
const float YUV_Y_B = 0.114;
const float YUV_U_R = -0.169;
const float YUV_U_G = 0.331;
const float YUV_U_B = 0.500;
const float YUV_V_R = 0.500;
const float YUV_V_G = 0.419;
const float YUV_V_B = 0.081;
const int YUV_DATA_SIZE = 3;
const int YUV_OFFSET = 2;
const int YUV_OFFSET_S = 1;
const int YUV_OFFSET_UV = 128;
const int ALIGN_LEFT = 16;
APP_ERROR CheckFileVaild(const std::string &filePath)
{
    struct stat buf;
    if (lstat(filePath.c_str(), &buf) != 0 || S_ISLNK(buf.st_mode)) {
        LogError << "Input file is invalid and cannot be a link";
        return APP_ERR_COMM_NO_EXIST;
    }
    char c[PATH_MAX + 1] = {0x00};
    size_t count = filePath.copy(c, PATH_MAX + 1);
    if (count != filePath.length()) {
        LogError << "Failed to copy file path.";
        return APP_ERR_COMM_FAILURE;
    }
    char path[PATH_MAX + 1] = {0x00};
    if (realpath(c, path) == nullptr) {
        LogError << "Failed to get the file.";
        return APP_ERR_COMM_NO_EXIST;
    }
    FILE *fp = fopen(path, "rb");
    if (fp == nullptr) {
        LogError << "Failed to open file";
        return APP_ERR_COMM_OPEN_FAIL;
    }
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fileSize <= 0 || fileSize > MAX_FILE_SIZE) {
        fclose(fp);
        return APP_ERR_COMM_FAILURE;
    }
    fclose(fp);
    return APP_ERR_OK;
}

std::vector<std::vector<ObjectInfo>> SDKPostProcess(std::string &yolov7ConfigPath, std::string &yolov7LabelPath,
    std::vector<Tensor> &yolov7Outputs, std::vector<ResizedImageInfo> &imagePreProcessInfos)
{
    std::map<std::string, std::string> postConfig;
    postConfig.insert(pair<std::string, std::string>("postProcessConfigPath", yolov7ConfigPath));
    postConfig.insert(pair<std::string, std::string>("labelPath", yolov7LabelPath));

    Yolov7PostProcess yolov7PostProcess;
    yolov7PostProcess.Init(postConfig);

    std::vector<TensorBase> tensors;
    for (size_t i = 0; i < yolov7Outputs.size(); i++) {
        MemoryData memoryData(yolov7Outputs[i].GetData(), yolov7Outputs[i].GetByteSize());
        TensorBase tensorBase(memoryData, true, yolov7Outputs[i].GetShape(), TENSOR_DTYPE_INT32);
        tensors.push_back(tensorBase);
    }
    std::vector<std::vector<ObjectInfo>> objectInfos;
    yolov7PostProcess.Process(tensors, objectInfos, imagePreProcessInfos);
    for (size_t i = 0; i < objectInfos.size(); i++) {
        LogInfo << "objectInfos-" << i;
        for (size_t j = 0; j < objectInfos[i].size(); j++) {
            LogInfo << " objectInfo-" << j;
            LogInfo << "      x0 is:" << objectInfos[i][j].x0;
            LogInfo << "      y0 is:" << objectInfos[i][j].y0;
            LogInfo << "      x1 is:" << objectInfos[i][j].x1;
            LogInfo << "      y1 is:" << objectInfos[i][j].y1;
            LogInfo << "      confidence is: " << objectInfos[i][j].confidence;
            LogInfo << "      classId is: " << objectInfos[i][j].classId;
            LogInfo << "      className is: " << objectInfos[i][j].className;
        }
    }
    return objectInfos;
}

std::vector<std::vector<Yolov8PoseDetectionInfo>> SDKPostPoseProcess(std::string &yoloConfigPath, std::string &yoloLabelPath,
    std::vector<Tensor> &yoloOutputs, std::vector<ResizedImageInfo> &imagePreProcessInfos)
{
    std::map<std::string, std::string> postConfig;
    postConfig.insert(pair<std::string, std::string>("postProcessConfigPath", yoloConfigPath));
    postConfig.insert(pair<std::string, std::string>("labelPath", yoloLabelPath));

    // OpenPosePostProcess openPosePostProcess;
    Yolov8PosePostProcess openPosePostProcess;
    openPosePostProcess.Init(postConfig);

    std::vector<TensorBase> tensors;
    for (size_t i = 0; i < yoloOutputs.size(); i++) {
        MemoryData memoryData(yoloOutputs[i].GetData(), yoloOutputs[i].GetByteSize());
        TensorBase tensorBase(memoryData, true, yoloOutputs[i].GetShape(), TENSOR_DTYPE_INT32);
        tensors.push_back(tensorBase);
    }

    std::vector<std::vector<Yolov8PoseDetectionInfo>> objectInfos;
    // std::vector<std::vector<KeyPointDetectionInfo>> objectInfoss;

    openPosePostProcess.Process(tensors, objectInfos, imagePreProcessInfos);
    LogInfo << "------------------------------";
    for (size_t i = 0; i < objectInfos.size(); i++) {
        LogInfo << "objectInfos-" << i;
        for (size_t j = 0; j < objectInfos[i].size(); j++) {
            LogInfo << " objectInfo-" << j;
            for (auto tmp : objectInfos[i][j].keyPointMap)
            {
                LogInfo << "  " << tmp.first << " " << tmp.second[0];
            }
            for (auto tmp : objectInfos[i][j].scoreMap)
            {
                LogInfo << "scoreMap  " << tmp.first << " " << tmp.second;
            }
            LogInfo << "      confidence is: " << objectInfos[i][j].score;
        }
    }
    for (auto tmp : objectInfos)
    {
        std:cout << "result size: " << tmp.size() << std::endl;
    }
    // KeyPointDetectionInfo
    return objectInfos;
    // return std::vector<std::vector<MxBase::ObjectInfo>>();
}

APP_ERROR PaddingProcess(ImageProcessor &imageProcessor, std::pair<int, int> resizeInfo, int deviceId,
    Image &resizeImage, Image &pastedImg)
{
    int resizedWidth = resizeInfo.first;
    int resizedHeight = resizeInfo.second;
    int leftOffset = (MODEL_INPUT_WIDTH - resizedWidth) / AVG_PARAM;
    int topOffset = (MODEL_INPUT_HEIGHT - resizedHeight) / AVG_PARAM;
    uint32_t dataSize = MODEL_INPUT_WIDTH * MODEL_INPUT_HEIGHT * RGB_EXTEND;
    MxBase::Size imageSize(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT);
    if (leftOffset > 0) {
        MemoryData srcData(resizeImage.GetData().get(), resizeImage.GetDataSize(), MemoryData::MemoryType::MEMORY_DVPP,
            deviceId);
        MemoryData resHostData(nullptr, resizeImage.GetDataSize(), MemoryData::MemoryType::MEMORY_HOST, -1);
        if (MemoryHelper::MxbsMallocAndCopy(resHostData, srcData) != APP_ERR_OK) {
            LogError << "Failed to mallloc and copy dvpp memory.";
            return APP_ERR_ACL_BAD_COPY;
        }
        cv::Mat resizedHost(resizeImage.GetSize().height, resizeImage.GetSize().width, OPENCV_8UC3,
            resHostData.ptrData);
        cv::Rect roi = cv::Rect(0, 0, resizedWidth, resizedHeight);
        cv::Mat extendedImage;
        cv::copyMakeBorder(resizedHost(roi), extendedImage, 0, 0, leftOffset,
            MODEL_INPUT_WIDTH - leftOffset - resizedWidth, cv::BORDER_CONSTANT,
            cv::Scalar(PAD_COLOR, PAD_COLOR, PAD_COLOR));
        int maxFillRow = std::min(MODEL_INPUT_WIDTH, (int)resizeImage.GetSize().width + leftOffset);
        for (int col = 0; col < MODEL_INPUT_WIDTH; col++) {
            for (int row = resizedWidth + leftOffset; row < maxFillRow; row++) {
                extendedImage.at<cv::Vec3b>(col, row)[0] = PAD_COLOR;
                extendedImage.at<cv::Vec3b>(col, row)[1] = PAD_COLOR;
                extendedImage.at<cv::Vec3b>(col, row)[R_CHANNEL] = PAD_COLOR;
            }
        }
        uint8_t* pasteHostData = (uint8_t*)malloc(dataSize);
        if (pasteHostData == nullptr) {
            return APP_ERR_ACL_BAD_ALLOC;
        }
        for (size_t i = 0; i < dataSize; i++) {
            pasteHostData[i] = extendedImage.data[i];
        }
        std::shared_ptr<uint8_t> dataPaste((uint8_t*)pasteHostData, free);
        Image pastedImgTmp(dataPaste, dataSize, -1, imageSize, ImageFormat::BGR_888);
        pastedImgTmp.ToDevice(0);
        pastedImg = pastedImgTmp;
    } else {
        MemoryData imgData(dataSize, MemoryData::MemoryType::MEMORY_DVPP, deviceId);
        if (MemoryHelper::Malloc(imgData) != APP_ERR_OK) {
            return APP_ERR_ACL_BAD_ALLOC;
        }
        std::shared_ptr<uint8_t> pastedData((uint8_t*)imgData.ptrData, imgData.free);
        if (MemoryHelper::Memset(imgData, PAD_COLOR, dataSize) != APP_ERR_OK) {
            LogError << "Failed to memset dvpp memory.";
            return APP_ERR_ACL_BAD_ALLOC;
        }
        Rect RectSrc(0, 0, resizedWidth, resizedHeight);
        Rect RectDst(leftOffset, topOffset, leftOffset + resizedWidth, topOffset + resizedHeight);
        std::pair<Rect, Rect> cropPasteRect = {RectSrc, RectDst};
        Image pastedImgTmp(pastedData, dataSize, deviceId, imageSize, ImageFormat::BGR_888);
        if (imageProcessor.CropAndPaste(resizeImage, cropPasteRect, pastedImgTmp) != APP_ERR_OK) {
            LogError << "Failed to padding the image by dvpp";
            return APP_ERR_COMM_FAILURE;
        }
        pastedImg = pastedImgTmp;
    }
    return APP_ERR_OK;
}

APP_ERROR SetImageBackground(MxBase::MemoryData& data)
{
    auto dataPtr = data.ptrData;
    float yuvY = YUV_Y_R * PAD_COLOR + YUV_Y_G * PAD_COLOR + YUV_Y_B * PAD_COLOR;
    float yuvU = YUV_U_R * PAD_COLOR - YUV_U_G * PAD_COLOR + YUV_U_B * PAD_COLOR + YUV_OFFSET_UV;
    float yuvV = YUV_V_R * PAD_COLOR - YUV_V_G * PAD_COLOR - YUV_V_B * PAD_COLOR + YUV_OFFSET_UV;

    APP_ERROR ret = MxBase::MemoryHelper::MxbsMemset(data, (int)yuvY, data.size);
    if (ret != APP_ERR_OK) {
        LogError << "Failed to memset dvpp memory";
        return ret;
    }
    int offsetSize = MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH / YUV_OFFSET;
    data.ptrData = (uint8_t *)data.ptrData + MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH;
    ret = MxBase::MemoryHelper::MxbsMemset(data, (int)yuvU, offsetSize);
    if (ret != APP_ERR_OK) {
        LogError << "Failed to memset dvpp memory";
        data.ptrData = dataPtr;
        return ret;
    }
    data.ptrData = (uint8_t *)data.ptrData + YUV_OFFSET_S;
    for (int i = 0; i < offsetSize / YUV_OFFSET; i++) {
        ret = MxBase::MemoryHelper::MxbsMemset(data, (int)yuvV, YUV_OFFSET_S);
        if (ret != APP_ERR_OK) {
            LogError << "Failed to memset dvpp memory";
            data.ptrData = dataPtr;
            return ret;
        }
        data.ptrData = (uint8_t *)data.ptrData + YUV_OFFSET;
    }
    data.ptrData = dataPtr;
    return APP_ERR_OK;
}

APP_ERROR DvppPreprocessorYuv(ImageProcessor &imageProcessor, std::string &imagePath, vector<Tensor> &yolov7Inputs,
    std::vector<ResizedImageInfo> &imagePreProcessInfos, int deviceId)
{
    Image decodeImage;
    APP_ERROR ret = imageProcessor.Decode(imagePath, decodeImage);
    if (ret != APP_ERR_OK) {
        LogError << "ImageProcessor decode failed.";
        return ret;
    }
    Image resizeImage;
    uint32_t originalWidth = decodeImage.GetOriginalSize().width;
    uint32_t originalHeight = decodeImage.GetOriginalSize().height;
    float scaleWidth = MODEL_INPUT_WIDTH * 1.0 / originalWidth;
    float scaleHeight = MODEL_INPUT_HEIGHT * 1.0 / originalHeight;
    float minScale = scaleWidth < scaleHeight ? scaleWidth : scaleHeight;
    int resizedWidth = std::round(originalWidth * minScale);
    int resizedHeight = std::round(originalHeight * minScale);
    ret = imageProcessor.Resize(decodeImage, MxBase::Size(resizedWidth, resizedHeight), resizeImage,
        Interpolation::BILINEAR_SIMILAR_OPENCV);
    if (ret != APP_ERR_OK) {
        LogError << "ImageProcessor resize failed.";
        return ret;
    }
    uint32_t dataSize = MODEL_INPUT_WIDTH * MODEL_INPUT_HEIGHT * RGB_EXTEND / YUV_DIVISION;
    MxBase::Size imageSize(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT);
    MemoryData imgData(dataSize, MemoryData::MemoryType::MEMORY_DVPP, deviceId);
    if (MemoryHelper::Malloc(imgData) != APP_ERR_OK) {
        LogError << "Failed to malloc dvpp memory.";
        return APP_ERR_ACL_BAD_ALLOC;
    }
    std::shared_ptr<uint8_t> pastedData((uint8_t*)imgData.ptrData, imgData.free);
    if (SetImageBackground(imgData) != APP_ERR_OK) {
        LogError << "Failed to memset dvpp memory.";
        return APP_ERR_ACL_BAD_ALLOC;
    }
    int leftOffset = (MODEL_INPUT_WIDTH - resizedWidth) / AVG_PARAM;
    int topOffset = (MODEL_INPUT_HEIGHT - resizedHeight) / AVG_PARAM;
    topOffset = topOffset % AVG_PARAM == 0 ? topOffset : topOffset - 1;
    leftOffset = leftOffset < ALIGN_LEFT ? 0 : leftOffset / ALIGN_LEFT * ALIGN_LEFT;
    Rect RectSrc(0, 0, resizedWidth, resizedHeight);
    Rect RectDst(leftOffset, topOffset, leftOffset + resizedWidth, topOffset + resizedHeight);
    std::pair<Rect, Rect> cropPasteRect = {RectSrc, RectDst};
    Image pastedImgTmp(pastedData, dataSize, deviceId, imageSize, ImageFormat::YUV_SP_420);
    if (imageProcessor.CropAndPaste(resizeImage, cropPasteRect, pastedImgTmp) != APP_ERR_OK) {
        LogError << "Failed to padding the image by dvpp";
        return APP_ERR_COMM_FAILURE;
    }
    yolov7Inputs.push_back(pastedImgTmp.ConvertToTensor());
    ResizedImageInfo imagePreProcessInfo(resizedWidth, resizedHeight, originalWidth, originalHeight,
        RESIZER_MS_KEEP_ASPECT_RATIO, minScale);
    imagePreProcessInfos.push_back(imagePreProcessInfo);
    return APP_ERR_OK;
}

APP_ERROR OpenCVPreProcessor(std::string &imagePath, vector<Tensor> &yolov7Inputs,
    std::vector<ResizedImageInfo> &imagePreProcessInfos, int deviceId)
{
    auto image = cv::imread(imagePath);
    size_t originalWidth = image.cols;
    size_t originalHeight = image.rows;
    float scaleWidth = MODEL_INPUT_WIDTH * 1.0 / originalWidth;
    float scaleHeight = MODEL_INPUT_HEIGHT * 1.0 / originalHeight;
    float minScale = scaleWidth < scaleHeight ? scaleWidth : scaleHeight;
    int resizedWidth = std::round(originalWidth * minScale);
    int resizedHeight = std::round(originalHeight * minScale);
    cv::Mat resizedImg;
    cv::resize(image, resizedImg, cv::Size(resizedWidth, resizedHeight));
    int leftOffset = (MODEL_INPUT_WIDTH - resizedWidth) / AVG_PARAM;
    int topOffset = (MODEL_INPUT_HEIGHT - resizedHeight) / AVG_PARAM;
    uint32_t dataSize = MODEL_INPUT_HEIGHT * MODEL_INPUT_WIDTH * RGB_EXTEND;
    MxBase::Size imageSize(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT);
    cv::Mat extendedImage;
    cv::copyMakeBorder(resizedImg, extendedImage, topOffset, MODEL_INPUT_HEIGHT - topOffset - resizedHeight, leftOffset,
        MODEL_INPUT_WIDTH - leftOffset - resizedWidth, cv::BORDER_CONSTANT,
        cv::Scalar(PAD_COLOR, PAD_COLOR, PAD_COLOR));
    uint8_t *pasteHostData = (uint8_t *)malloc(dataSize);
    if (pasteHostData == nullptr) {
        return APP_ERR_ACL_BAD_ALLOC;
    }
    for (size_t i = 0; i < dataSize; i++) {
        pasteHostData[i] = extendedImage.data[i];
    }
    std::shared_ptr<uint8_t> dataPaste((uint8_t *)pasteHostData, free);
    Image pastedImage(dataPaste, dataSize, -1, imageSize, ImageFormat::BGR_888);
    pastedImage.ToDevice(deviceId);
    yolov7Inputs.push_back(pastedImage.ConvertToTensor());
    ResizedImageInfo imagePreProcessInfo(resizedWidth, resizedHeight, originalWidth, originalHeight,
        RESIZER_TF_KEEP_ASPECT_RATIO, minScale);
    imagePreProcessInfos.push_back(imagePreProcessInfo);
    return APP_ERR_OK;
}

APP_ERROR DvppPreprocessor(std::string &imagePath, vector<Tensor> &yolov7Inputs,
    std::vector<ResizedImageInfo> &imagePreProcessInfos, int deviceId, bool isYuvInput)
{
    ImageProcessor imageProcessor(deviceId);
    if (isYuvInput) {
        return DvppPreprocessorYuv(imageProcessor, imagePath, yolov7Inputs, imagePreProcessInfos, deviceId);
    } else {
        if (DeviceManager::IsAscend310P()) {
            Image decodeImage;
            APP_ERROR ret = imageProcessor.Decode(imagePath, decodeImage, ImageFormat::BGR_888);
            if (ret != APP_ERR_OK) {
                LogError << "ImageProcessor decode failed.";
                return OpenCVPreProcessor(imagePath, yolov7Inputs, imagePreProcessInfos, deviceId);
            }
            Image resizeImage;
            uint32_t originalWidth = decodeImage.GetOriginalSize().width;
            uint32_t originalHeight = decodeImage.GetOriginalSize().height;
            float scaleWidth = MODEL_INPUT_WIDTH * 1.0 / originalWidth;
            float scaleHeight = MODEL_INPUT_HEIGHT * 1.0 / originalHeight;
            float minScale = scaleWidth < scaleHeight ? scaleWidth : scaleHeight;
            int resizedWidth = std::round(originalWidth * minScale);
            int resizedHeight = std::round(originalHeight * minScale);
            ret = imageProcessor.Resize(decodeImage, MxBase::Size(resizedWidth, resizedHeight), resizeImage,
                Interpolation::BILINEAR_SIMILAR_OPENCV);
            if (ret != APP_ERR_OK) {
                LogError << "ImageProcessor resize failed.";
                return ret;
            }
            Image pastedImage;
            std::pair<int, int> resizedInfo(resizedWidth, resizedHeight);
            ret = PaddingProcess(imageProcessor, resizedInfo, deviceId, resizeImage, pastedImage);
            if (ret != APP_ERR_OK) {
                LogError << "ImageProcessor padding failed.";
                return ret;
            }
            yolov7Inputs.push_back(pastedImage.ConvertToTensor());
            ResizedImageInfo imagePreProcessInfo(resizedWidth, resizedHeight, originalWidth, originalHeight,
                RESIZER_TF_KEEP_ASPECT_RATIO, minScale);
            imagePreProcessInfos.push_back(imagePreProcessInfo);
        } else {
            return OpenCVPreProcessor(imagePath, yolov7Inputs, imagePreProcessInfos, deviceId);
        }
    }
    return APP_ERR_OK;
}


void draw_pose(cv::Mat &image, const KeyPoint &keypoints, const float point_circle_ratio = 1.0, const bool scale_by_score = true, float pose_conf_threshold = 0.5)
{

	std::vector<cv::Point3f> resultVector;

    // 使用范围循环将结构体成员变量的值添加到vector中
    for (const auto& point : {keypoints.nose, keypoints.leftEye, keypoints.rightEye,
                              keypoints.leftEar, keypoints.rightEar, keypoints.leftShoulder,
                              keypoints.rightShoulder, keypoints.leftElbow, keypoints.rightElbow,
                              keypoints.leftWrist, keypoints.rightWrist, keypoints.leftHip,
                              keypoints.rightHip, keypoints.leftKnee, keypoints.rightKnee,
                              keypoints.leftAnkle, keypoints.rightAnkle})
    {
        resultVector.push_back(point);
    }

	std::vector<cv::Scalar> pose_palette = {
		{255, 0, 127}, {254, 37, 103}, {251, 77, 77}, {248, 115, 51}, {242, 149, 25}, {235, 180, 0}, {227, 205, 24}, {217, 226, 50}, {206, 242, 76}, {193, 251, 102}, {179, 254, 128}, {165, 251, 152}, {149, 242, 178}, {132, 226, 204}, {115, 205, 230}, {96, 178, 255}, {78, 149, 255}, {59, 115, 255}, {39, 77, 255}, {18, 37, 255}, {0, 0, 255}};

	std::vector<cv::Point> skeleton = {
		{15, 13}, {13, 11}, {16, 14}, {14, 12}, {11, 12}, {5, 11}, {6, 12}, {5, 6}, {5, 7}, {6, 8}, {7, 9}, {8, 10}, {1, 2}, {0, 1}, {0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 6}};
	std::vector<cv::Scalar> limb_color = {
		pose_palette[9], pose_palette[9], pose_palette[9], pose_palette[9], pose_palette[7],
		pose_palette[7], pose_palette[7], pose_palette[0], pose_palette[0], pose_palette[0],
		pose_palette[0], pose_palette[0], pose_palette[16], pose_palette[16], pose_palette[16],
		pose_palette[16], pose_palette[16], pose_palette[16], pose_palette[16]};

	std::vector<cv::Scalar> kpt_color = pose_palette;

	float scale = 1 / 150.0f;
	int thickness = std::min((int)(image.rows * scale), (int)(image.cols * scale));

	for (int i = 0; i < skeleton.size(); ++i)
	{

		auto &index = skeleton[i];
		auto &pos1 = resultVector[index.x];
		auto &pos2 = resultVector[index.y];

		std::vector<cv::Point3f> point_pair = {pos1, pos2};

		for (const auto & point: point_pair) {
			if (point.x >= 0 && point.y >= 0 & point.z >= pose_conf_threshold) {
				float base_radius_f = thickness * 1.6 * point_circle_ratio;
				float radius_f = scale_by_score ? (base_radius_f * point.z) : base_radius_f;
				int radius = static_cast<int>(radius_f);
				cv::circle(image, cv::Point(point.x, point.y), radius, kpt_color[i], -1, cv::LINE_AA);
			}
		}

		if (pos1.z < pose_conf_threshold || pos2.z < pose_conf_threshold)
			continue;

		if (pos1.x < 0 || pos1.y < 0 || pos2.x < 0 || pos2.y < 0)
			continue;

		cv::line(image, cv::Point(pos1.x, pos1.y), cv::Point(pos2.x, pos2.y), limb_color[i], 2, cv::LINE_AA);
	}
}

void draw_yolov8Pose(std::string &imagePath, std::vector<std::vector<Yolov8PoseDetectionInfo>> &objectInfos)
{
	// get origin image
	cv::Mat imgBgr = cv::imread(imagePath);

    // print result
    std::cout << "Size of objectInfos is " << objectInfos.size() << std::endl;
    for (size_t i = 0; i < objectInfos.size(); i++)
    {
        std::cout << "objectInfo-" << i << " ,Size:"<< objectInfos[i].size() << std::endl;
        for (size_t j = 0; j < objectInfos[i].size(); j++)
        {
            auto objectInfo = objectInfos[i][j].objectInfo;
            std::cout << std::endl << "*****objectInfo-" << i << ":" << j << std::endl;
            std::cout << "x0 is " << objectInfo.x0 << std::endl;
            std::cout << "y0 is " << objectInfo.y0 << std::endl;
            std::cout << "x1 is " << objectInfo.x1 << std::endl;
            std::cout << "y1 is " << objectInfo.y1 << std::endl;
            std::cout << "confidence is " << objectInfo.confidence << std::endl;
            std::cout << "classId is " << objectInfo.classId << std::endl;
            std::cout << "className is " << objectInfo.className << std::endl;

			uint32_t y0 = objectInfo.y0;
			uint32_t x0 = objectInfo.x0;
			uint32_t y1 = objectInfo.y1;
			uint32_t x1 = objectInfo.x1;
			
			cv::putText(imgBgr, objectInfo.className + std::to_string(objectInfo.confidence), cv::Point(x0 + 10, y0 + 10), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255,0), 4, 8);
			cv::rectangle(imgBgr, cv::Rect(x0, y0, x1 - x0, y1 - y0), cv::Scalar(0, 255, 0), 4);
            // rader keyPoint
            draw_pose(imgBgr, objectInfos[i][j].keyPoint, 1, true, 0.25);
        }
    }
    
    // output result
	cv::imwrite("result.jpg", imgBgr);
}

void draw_yolov5(std::string &imagePath, std::vector<std::vector<ObjectInfo>> &objectInfos)
{
	// get origin image
	cv::Mat imgBgr = cv::imread(imagePath);

    // print result
    std::cout << "Size of objectInfos is " << objectInfos.size() << std::endl;
    for (size_t i = 0; i < objectInfos.size(); i++)
    {
        std::cout << "objectInfo-" << i << " ,Size:"<< objectInfos[i].size() << std::endl;
        for (size_t j = 0; j < objectInfos[i].size(); j++)
        {
            auto objectInfo = objectInfos[i][j];
            std::cout << std::endl << "*****objectInfo-" << i << ":" << j << std::endl;
            std::cout << "x0 is " << objectInfo.x0 << std::endl;
            std::cout << "y0 is " << objectInfo.y0 << std::endl;
            std::cout << "x1 is " << objectInfo.x1 << std::endl;
            std::cout << "y1 is " << objectInfo.y1 << std::endl;
            std::cout << "confidence is " << objectInfo.confidence << std::endl;
            std::cout << "classId is " << objectInfo.classId << std::endl;
            std::cout << "className is " << objectInfo.className << std::endl;

			uint32_t y0 = objectInfo.y0;
			uint32_t x0 = objectInfo.x0;
			uint32_t y1 = objectInfo.y1;
			uint32_t x1 = objectInfo.x1;
			
			cv::putText(imgBgr, objectInfo.className + std::to_string(objectInfo.confidence), cv::Point(x0 + 10, y0 + 10), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255,0), 4, 8);
			cv::rectangle(imgBgr, cv::Rect(x0, y0, x1 - x0, y1 - y0), cv::Scalar(0, 255, 0), 4);
        }
    }
    // output result
	cv::imwrite("result.jpg", imgBgr);
}

APP_ERROR E2eInfer(std::map<std::string, std::string> pathMap, int32_t deviceId, bool isYuvInput)
{
    std::string imagePath = pathMap["imgPath"];
    APP_ERROR ret = CheckFileVaild(imagePath);
    if (ret != APP_ERR_OK) {
        return ret;
    }
    vector<Tensor> yoloInputs;
    std::vector<ResizedImageInfo> resizedImageInfos;
    ret = DvppPreprocessor(imagePath, yoloInputs, resizedImageInfos, deviceId, isYuvInput);
    if (ret != APP_ERR_OK) {
        return ret;
    }
    string modelPath = pathMap["modelPath"];
    ret = CheckFileVaild(modelPath);
    if (ret != APP_ERR_OK) {
        return ret;
    }
    Model yolo(modelPath, deviceId);
    std::cout << "infer" << std::endl;
    vector<Tensor> yoloOutputs = yolo.Infer(yoloInputs);
    if (yoloOutputs.size() == 0) {
        LogError << "YOLO infer failed.";
        return APP_ERR_COMM_FAILURE;
    }
    for (size_t i = 0; i < yoloOutputs.size(); i++) {
        yoloOutputs[i].ToHost();
    }
    std::vector<Rect> cropConfigVec;
    string yoloConfigPath = pathMap["modelConfigPath"];
    string yoloLabelPath = pathMap["modelLabelPath"];
    string postType = pathMap["postType"];

    ret = CheckFileVaild(yoloConfigPath);
    if (ret != APP_ERR_OK) {
        return ret;
    }
    ret = CheckFileVaild(yoloLabelPath);
    if (ret != APP_ERR_OK) {
        return ret;
    }
    if (postType == "yolov5")
    {
        std::vector<std::vector<ObjectInfo>> objectInfos =
            SDKPostProcess(yoloConfigPath, yoloLabelPath, yoloOutputs, resizedImageInfos);
        draw_yolov5(imagePath, objectInfos);
    }
    else if (postType == "yolov8Pose")
    {
        std::vector<std::vector<Yolov8PoseDetectionInfo>> objectInfos =
            SDKPostPoseProcess(yoloConfigPath, yoloLabelPath, yoloOutputs, resizedImageInfos);
        draw_yolov8Pose(imagePath, objectInfos);
    }


    return APP_ERR_OK;
}

void usage()
{
    std::cout << "Usage: " << std::endl;
    std::cout << "./sample -m model_path -c model_config_path -l model_label_path -i image_path  -t post_Type [-y] " << std::endl;
}

int main(int argc, char *argv[])
{
    MxInit();
    if (argc > ARG_NUM || argc < ARG_NUM - 1) {
        usage();
        return 0;
    }
    int32_t deviceId = 0;
    bool isYuvInput = 0;
    std::map<std::string, std::string> pathMap;
    int input;
    std::cout << argc << std::endl;
    const char* optString = "i:m:c:l:y:t:h";
    while ((input = getopt(argc, argv, optString)) != -1) {
        switch (input) {
            std::cout << input << std::endl;
            case 'm':
                pathMap.insert({ "modelPath", optarg });
                break;
            case 'i':
                pathMap.insert({ "imgPath", optarg });
                break;
            case 'c':
                pathMap.insert({ "modelConfigPath", optarg });
                break;
            case 'l':
                pathMap.insert({ "modelLabelPath", optarg });
                break;
            case 'y':
                isYuvInput = true;
                break;
            case 't':
                pathMap.insert({ "postType", optarg });
                std::cout << input << " ------ : "<< optarg << std::endl;
                break;
            case 'h':
                usage();
                return 0;
            case '?':
                usage();
                return 0;
        }
    }
    if (pathMap.count("modelPath") <= 0 || pathMap.count("imgPath") <= 0 || pathMap.count("modelConfigPath") <= 0 ||
        pathMap.count("modelLabelPath") <= 0) {
        LogError << "Invalid input params";
        usage();
        return 0;
    }
    std::cout << "start infer"<< std::endl;
    APP_ERROR ret = E2eInfer(pathMap, deviceId, isYuvInput);
    if (ret != APP_ERR_OK) {
        LogError << "Failed to run E2eInfer";
    }
    return 0;
}