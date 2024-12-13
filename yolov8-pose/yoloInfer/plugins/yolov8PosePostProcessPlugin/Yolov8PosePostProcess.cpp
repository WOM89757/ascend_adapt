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

#include "Yolov8PosePostProcess.h"
#include <algorithm>
#include "MxBase/Log/Log.h"
#include "MxBase/Maths/FastMath.h"
#include "MxBase/CV/ObjectDetection/Nms/Nms.h"
#include "MxBase/DeviceManager/DeviceManager.h"

namespace MxBase {
const int MODEL_INPUT_SIZE = 640;
const int ALIGN_LEFT = 16;
const int CONFIDENCE_IDX = 4;
const int LABEL_START_OFFSET = 5;
const int OUTPUT_DIMS = 3;
const int XOFFSET = 2;
const int YOFFSET = 3;
const int AVG_PARAM = 2;
const float EPSILON = 1e-6;
Yolov8PosePostProcess::Yolov8PosePostProcess() {}

Yolov8PosePostProcess &Yolov8PosePostProcess::operator = (const Yolov8PosePostProcess &other)
{
    if (this == &other) {
        return *this;
    }
    KeypointPostProcessBase::operator = (other);
    scoreThresh_ = other.scoreThresh_;
    classNum_ = other.classNum_;
    numKeyPoints_ = other.numKeyPoints_;
    // separateScoreThresh_ = other.separateScoreThresh_;
    // paddingType_ = other.paddingType_;
    return *this;
}

APP_ERROR Yolov8PosePostProcess::Init(const std::map<std::string, std::string> &postConfig)
{
    LogDebug << "Start to Init Yolov8PosePostProcess. ";
    APP_ERROR ret = KeypointPostProcessBase::Init(postConfig);
    if (ret != APP_ERR_OK) {
        LogError << GetError(ret) << "Fail to superInit in ObjectPostProcessorBase";
        return ret;
    }
    ret = configData_.GetFileValue<int>("KEYPOINT_NUM", numKeyPoints_, 0, 50);
    if (ret != APP_ERR_OK) {
        LogWarn << GetError(ret) << "Fail to read NUM_KEYPOINTS from config, default is :" << numKeyPoints_;
    }
    ret = configData_.GetFileValue<float>("SCORE_THRESH", scoreThresh_, 0.0f, 1.0f);
    if (ret != APP_ERR_OK) {
        LogWarn << GetError(ret) << "Fail to read SCORE_THRESH from config, default is :" << scoreThresh_;
    }
    ret = configData_.GetFileValue<int>("PADDING_TYPE", paddingType_, 0, 1);
    if (ret != APP_ERR_OK) {
        LogWarn << GetError(ret) << "Fail to read PADDING_TYPE from config, default is :" << paddingType_;
    }
    // separateScoreThresh_ = {0.6};
    ret = configData_.GetFileValue<float>("IOU_THRESH", iouThresh_, 0.0f, 1.0f);
    if (ret != APP_ERR_OK) {
        LogWarn << GetError(ret) << "Fail to read IOU_THRESH from config, default is :" << iouThresh_;
    }

    LogDebug << "End to Init Yolov8PosePostProcess. ";
    return APP_ERR_OK;
}

APP_ERROR Yolov8PosePostProcess::DeInit()
{
    return APP_ERR_OK;
}

void Yolov8PosePostProcess::ConstructBoxFromOutput(float *output, size_t offset, std::vector<Yolov8PoseDetectionInfo> &yolov8PoseObjectInfos, const ResizedImageInfo &resizedImageInfo)
{
    size_t index = offset * (3 * numKeyPoints_ + LABEL_START_OFFSET);
    if (output[index + CONFIDENCE_IDX] <= scoreThresh_) {
        return;
    }

    float prob = output[index + CONFIDENCE_IDX];
    // std::cout << output[index] << " " << output[index + 1] << " " << output[index +2 ] << " " << output[index +3] << " " << output[index + 4] << " " << output[index + 5] << std::endl;

    int tmpResizedWidth = resizedImageInfo.widthResize;
    int tmpResizedHeight = resizedImageInfo.heightResize;
    double division = 1;
    if (std::fabs(resizedImageInfo.keepAspectRatioScaling) > EPSILON) {
        division = resizedImageInfo.keepAspectRatioScaling;
    }
    if (tmpResizedWidth == tmpResizedHeight && tmpResizedHeight == MODEL_INPUT_SIZE) {
        tmpResizedWidth = std::round(resizedImageInfo.widthOriginal * division);
        tmpResizedHeight = std::round(resizedImageInfo.heightOriginal * division);
    }
    int offsetLeft = (MODEL_INPUT_SIZE - tmpResizedWidth) / AVG_PARAM;
    int offsetTop = (MODEL_INPUT_SIZE - tmpResizedHeight) / AVG_PARAM;
    if (paddingType_ == 0) {
        offsetTop = offsetTop % AVG_PARAM == 0 ? offsetTop : offsetTop - 1;
        offsetLeft = offsetLeft < ALIGN_LEFT ? 0 : offsetLeft / ALIGN_LEFT * ALIGN_LEFT;
    }
    auto leftX = (output[index] - output[index + XOFFSET] / AVG_PARAM - offsetLeft) / division;
    auto leftY = (output[index + 1] - output[index + YOFFSET] / AVG_PARAM - offsetTop) / division;
    auto rightX = (output[index] + output[index + XOFFSET] / AVG_PARAM - offsetLeft) / division;
    auto rightY = (output[index + 1] + output[index + YOFFSET] / AVG_PARAM - offsetTop) / division;
    // keyPoint
    KeyPointDetectionInfo keyPointInfo;
    KeyPoint keyPoint;
    cv::Point3f *keyPointsArray[] = 
    {
        &keyPoint.nose,
        &keyPoint.leftEye,
        &keyPoint.rightEye,
        &keyPoint.leftEar,
        &keyPoint.rightEar,
        &keyPoint.leftShoulder,
        &keyPoint.rightShoulder,
        &keyPoint.leftElbow,
        &keyPoint.rightElbow,
        &keyPoint.leftWrist,
        &keyPoint.rightWrist,
        &keyPoint.leftHip,
        &keyPoint.rightHip,
        &keyPoint.leftKnee,
        &keyPoint.rightKnee,
        &keyPoint.leftAnkle,
        &keyPoint.rightAnkle
    };
    for (int i = 0; i < numKeyPoints_; i++)
    {
        int startIndex = index + LABEL_START_OFFSET;
        startIndex += i * 3;
        float pointX = (output[startIndex] - offsetLeft) / division;
        float pointY = (output[startIndex + 1] - offsetTop) / division;
        float pointConf = (output[startIndex + 2]);
        // std::cout << pointX << " " << pointY << " " << pointConf << std::endl;
        std::vector<float> pointInfo = {pointX, pointY};
        keyPointInfo.keyPointMap[i] = pointInfo;
        keyPointInfo.scoreMap[i] = pointConf;

        (*keyPointsArray[i]).x = pointX;
        (*keyPointsArray[i]).y = pointY;
        (*keyPointsArray[i]).z = pointConf;
    }
    keyPointInfo.score = prob;

    Yolov8PoseDetectionInfo yolov8PoseObjectInfo;
    auto& obj = yolov8PoseObjectInfo.objectInfo;
    obj.x0 = leftX < 0.0 ? 0.0 : leftX;
    obj.y0 = leftY < 0.0 ? 0.0 : leftY;
    obj.x1 = rightX > resizedImageInfo.widthOriginal ? resizedImageInfo.widthOriginal : rightX;
    obj.y1 = rightY > resizedImageInfo.heightOriginal ? resizedImageInfo.heightOriginal : rightY;
    obj.confidence = prob;
    obj.className = configData_.GetClassName(obj.classId);
    yolov8PoseObjectInfo = keyPointInfo;
    yolov8PoseObjectInfo.keyPoint = keyPoint;
    yolov8PoseObjectInfos.push_back(yolov8PoseObjectInfo);
}

void Yolov8PosePostProcess::LogObjectInfo(std::vector<std::vector<Yolov8PoseDetectionInfo>> &objectInfos)
{
    for (size_t i = 0; i < objectInfos.size(); i++) {
        LogInfo << "Objects in Image No." << i << " are listed:";
        for (auto &poseObjInfo : objectInfos[i]) {
            auto& objInfo = poseObjInfo.objectInfo;
            auto number = (separateScoreThresh_.size() > objInfo.classId) ? separateScoreThresh_[(int)objInfo.classId] :
                scoreThresh_;
            LogInfo << "Find object: classId(" << objInfo.classId << "), confidence("  << objInfo.confidence <<
                "), scoreThresh(" << number <<"), Coordinates (x0, y0)=(" << objInfo.x0 << ", " << objInfo.y0 <<
                "); (x1, y1)=(" << objInfo.x1 << ", " << objInfo.y1 << "). ";
            for (int j = 0; j < poseObjInfo.keyPointMap.size(); j++)
            {
                auto keyPoint = poseObjInfo.keyPointMap[j];
                LogInfo << keyPoint[0] << " " << keyPoint[1] << " " << poseObjInfo.scoreMap[j];
            }

        }
    }
}

// 非极大值抑制
void Yolov8PosePostProcess::NmsSort(std::vector<Yolov8PoseDetectionInfo>& detBoxes, float iouThresh, IOUMethod method) {
    // 按置信度降序排序
    std::sort(detBoxes.begin(), detBoxes.end(), [](const Yolov8PoseDetectionInfo& a, const Yolov8PoseDetectionInfo& b) {
        return a.objectInfo.confidence > b.objectInfo.confidence;
    });

    std::vector<bool> suppressed(detBoxes.size(), false);

    for (size_t i = 0; i < detBoxes.size(); ++i) {
        if (suppressed[i]) {
            continue;
        }

        for (size_t j = i + 1; j < detBoxes.size(); ++j) {
            if (suppressed[j]) {
                continue;
            }

            // float iou = this->CalcIou(detBoxes[i].objectInfo, detBoxes[j].objectInfo, method);
            float iou = MxBase::CalcIou(detBoxes[i].objectInfo, detBoxes[j].objectInfo, method);
            if (iou > iouThresh) {
                suppressed[j] = true;
            }
        }
    }

    // 删除被抑制的框
    detBoxes.erase(std::remove_if(detBoxes.begin(), detBoxes.end(), [&suppressed, &detBoxes](const Yolov8PoseDetectionInfo & box) {
        size_t index = &box - &detBoxes[0];
        return suppressed[index];
    }), detBoxes.end());
}

void Yolov8PosePostProcess::ParseOutput(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<Yolov8PoseDetectionInfo>>& batchYolov8PoseDetectionInfo,
                            std::vector<std::vector<KeyPointDetectionInfo>>& keyPointInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos)
{
    uint32_t batchSize = tensors[0].GetShape()[0];
    // LogInfo << "tensor all size: " << tensors[0].GetSize() << std::endl;
    size_t rows = tensors[0].GetSize() / ((3 * numKeyPoints_ + LABEL_START_OFFSET) * batchSize);
    for (size_t k = 0; k < batchSize; k++) {
        auto output = (float*)GetBuffer(tensors[0], k);
        std::vector<Yolov8PoseDetectionInfo> yolov8PoseObjectInfos;
        std::vector<KeyPointDetectionInfo> keyPointInfo;
        for (size_t i = 0; i < rows; ++i) {
            ConstructBoxFromOutput(output, i, yolov8PoseObjectInfos, resizedImageInfos[k]);
        };
        this->NmsSort(yolov8PoseObjectInfos, iouThresh_);

        for (size_t i = 0; i < yolov8PoseObjectInfos.size(); ++i) {
            KeyPointDetectionInfo keyPoint;
            for (auto tmp : yolov8PoseObjectInfos[i].keyPointMap)
            {
                keyPoint.keyPointMap[tmp.first] = tmp.second;
            }
            for (auto tmp : yolov8PoseObjectInfos[i].scoreMap)
            {
                keyPoint.scoreMap[tmp.first] = tmp.second;
            }
            keyPoint.score = yolov8PoseObjectInfos[i].score;
            keyPointInfo.emplace_back(keyPoint);
        }

        batchYolov8PoseDetectionInfo.push_back(yolov8PoseObjectInfos);
        keyPointInfos.push_back(keyPointInfo);
        std::cout << "yolov8PoseObjectInfos size: " << yolov8PoseObjectInfos.size() << std::endl;
        std::cout << "keyPoint size: " << keyPointInfo.size() << std::endl;
    }
}

APP_ERROR Yolov8PosePostProcess::Check(const std::vector<TensorBase>& tensors,
                            const std::vector<ResizedImageInfo>& resizedImageInfos)
{
    // std::cout << "----------- " << tensors[0].GetShape().size() << std::endl;
    for (int i = 0; i < tensors[0].GetShape().size(); i++)
    {
        std::cout << tensors[0].GetShape()[i] << " ";
    }
    std::cout << std::endl;
    if (resizedImageInfos.empty() || tensors.empty() || tensors[0].GetShape().size() != OUTPUT_DIMS) {
        LogError << "Tensors or ResizedImageInfos is not provided for yolov8 pose postprocess";
        return APP_ERR_INPUT_NOT_MATCH;
    }
    // std::cout << "numKeyPoints : " << numKeyPoints_ << std::endl;
    if (tensors[0].GetShape()[OUTPUT_DIMS - 1] != (3 * numKeyPoints_ + LABEL_START_OFFSET)) {
        LogError << "The model output tensor[2] != numKeyPoints.";
        return APP_ERR_INPUT_NOT_MATCH;
    }
    if (resizedImageInfos.size() != tensors.size()) {
        LogError << "The size of resizedImageInfos does not match the size of tensors.";
        return APP_ERR_INPUT_NOT_MATCH;
    }
    return APP_ERR_OK;
}

APP_ERROR Yolov8PosePostProcess::Process(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<KeyPointDetectionInfo>>& keyPointInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos,
                            const std::map<std::string, std::shared_ptr<void>> &configParamMap)
{
    LogDebug << "Start to Process Yolov8PosePostProcess.";
    auto ret = Check(tensors, resizedImageInfos);
    if (ret != APP_ERR_OK)
    {
        return ret;
    }
    std::vector<std::vector<Yolov8PoseDetectionInfo>> batchYolov8PoseObjectInfos;
    ParseOutput(tensors, batchYolov8PoseObjectInfos, keyPointInfos, resizedImageInfos);
    LogObjectInfo(batchYolov8PoseObjectInfos);
    LogInfo << "End to Process Yolov8PosePostProcess.";
    return APP_ERR_OK;
}

APP_ERROR Yolov8PosePostProcess::Process(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<Yolov8PoseDetectionInfo>>& batchYolov8PoseObjectInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos,
                            const std::map<std::string, std::shared_ptr<void>> &configParamMap)
{
    LogDebug << "Start to Process Yolov8PosePostProcess.";
    auto ret = Check(tensors, resizedImageInfos);
    if (ret != APP_ERR_OK)
    {
        return ret;
    }
    std::vector<std::vector<KeyPointDetectionInfo>> keyPointInfos;
    ParseOutput(tensors, batchYolov8PoseObjectInfos, keyPointInfos, resizedImageInfos);
    LogObjectInfo(batchYolov8PoseObjectInfos);
    LogInfo << "End to Process Yolov8PosePostProcess.";
    return APP_ERR_OK;
}

uint64_t Yolov8PosePostProcess::GetCurrentVersion()
{
    return 1;
}

extern "C" {
std::shared_ptr<MxBase::Yolov8PosePostProcess> GetObjectInstance()
{
    LogInfo << "Begin to get Yolov8PosePostProcess instance";
    auto instance = std::make_shared<MxBase::Yolov8PosePostProcess>();
    LogInfo << "End to get Yolov8PosePostProcess instance";
    return instance;
}
}
}