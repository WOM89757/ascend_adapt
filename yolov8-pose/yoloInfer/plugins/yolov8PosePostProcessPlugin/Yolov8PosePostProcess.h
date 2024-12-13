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

#ifndef YOLOV8_POSE_POST_PROCESS_H
#define YOLOV8_POSE_POST_PROCESS_H

#include "MxBase/PostProcessBases/KeypointPostProcessBase.h"
#include "MxBase/CV/ObjectDetection/Nms/Nms.h"
#include "opencv2/opencv.hpp"

namespace MxBase {
// 8.Class for key point and object detection
// class SDK_AVAILABLE_FOR_OUT Yolov8PoseDetectionInfo {
// public:
//     KeyPointDetectionInfo keyPointDetectionInfo;
//     ObjectInfo objectInfo;
// };

struct KeyPoint
{
    cv::Point3f nose;
    cv::Point3f leftEye;
    cv::Point3f rightEye;
    cv::Point3f leftEar;
    cv::Point3f rightEar;
    cv::Point3f leftShoulder;
    cv::Point3f rightShoulder;
    cv::Point3f leftElbow;
    cv::Point3f rightElbow;
    cv::Point3f leftWrist;
    cv::Point3f rightWrist;
    cv::Point3f leftHip;
    cv::Point3f rightHip;
    cv::Point3f leftKnee;
    cv::Point3f rightKnee;
    cv::Point3f leftAnkle;
    cv::Point3f rightAnkle;
    KeyPoint()
    {
        Reset();
    }
    ~KeyPoint()
    {
        Reset();
    }
    KeyPoint(const KeyPoint &other)
    {
        *this = other;
    }
    KeyPoint &operator=(const KeyPoint &other)
    {
        if (this != &other)
        {
            nose = other.nose;
            leftEye = other.leftEye;
            rightEye = other.rightEye;
            leftEar = other.leftEar;
            rightEar = other.rightEar;
            leftShoulder = other.leftShoulder;
            rightShoulder = other.rightShoulder;
            leftElbow = other.leftElbow;
            rightElbow = other.rightElbow;
            leftWrist = other.leftWrist;
            rightWrist = other.rightWrist;
            leftHip = other.leftHip;
            rightHip = other.rightHip;
            leftKnee = other.leftKnee;
            rightKnee = other.rightKnee;
            leftAnkle = other.leftAnkle;
            rightAnkle = other.rightAnkle;
        }
        return *this;
    }
    void Reset()
    {
        nose = cv::Point3f(0, 0, 0);
        leftEye = cv::Point3f(0, 0, 0);
        rightEye = cv::Point3f(0, 0, 0);
        leftEar = cv::Point3f(0, 0, 0);
        rightEar = cv::Point3f(0, 0, 0);
        leftShoulder = cv::Point3f(0, 0, 0);
        rightShoulder = cv::Point3f(0, 0, 0);
        leftElbow = cv::Point3f(0, 0, 0);
        rightElbow = cv::Point3f(0, 0, 0);
        leftWrist = cv::Point3f(0, 0, 0);
        rightWrist = cv::Point3f(0, 0, 0);
        leftHip = cv::Point3f(0, 0, 0);
        rightHip = cv::Point3f(0, 0, 0);
        leftKnee = cv::Point3f(0, 0, 0);
        rightKnee = cv::Point3f(0, 0, 0);
        leftAnkle = cv::Point3f(0, 0, 0);
        rightAnkle = cv::Point3f(0, 0, 0);
    }
};

class Yolov8PoseDetectionInfo : public KeyPointDetectionInfo {
public:
    ObjectInfo objectInfo;
    KeyPoint keyPoint;
    Yolov8PoseDetectionInfo &operator=(const KeyPointDetectionInfo &other) {
        scoreMap = other.scoreMap;
        keyPointMap = other.keyPointMap;
        score = other.score;
        return *this;
    }
};

class Yolov8PosePostProcess : public KeypointPostProcessBase {
public:
    Yolov8PosePostProcess();

    ~Yolov8PosePostProcess() = default;

    Yolov8PosePostProcess(const Yolov8PosePostProcess &other);

    Yolov8PosePostProcess &operator=(const Yolov8PosePostProcess &other);

    APP_ERROR Init(const std::map<std::string, std::string> &postConfig) override;

    APP_ERROR DeInit() override;

    APP_ERROR Process(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<KeyPointDetectionInfo>>& keyPointInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos = {},
                            const std::map<std::string, std::shared_ptr<void>> &configParamMap = {});
    APP_ERROR Process(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<Yolov8PoseDetectionInfo>>& keyPointInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos = {},
                            const std::map<std::string, std::shared_ptr<void>> &configParamMap = {});
    uint64_t GetCurrentVersion() override;

private:
    void ConstructBoxFromOutput(float *output, size_t offset, std::vector<Yolov8PoseDetectionInfo> &objectInfo, const ResizedImageInfo &resizedImageInfo);
    void LogObjectInfo(std::vector<std::vector<Yolov8PoseDetectionInfo>> &objectInfos);

    void ParseOutput(const std::vector<TensorBase>& tensors,
                            std::vector<std::vector<Yolov8PoseDetectionInfo>>& batchYolov8PoseDetectionInfo,
                            std::vector<std::vector<KeyPointDetectionInfo>>& keyPointInfos,
                            const std::vector<ResizedImageInfo>& resizedImageInfos);
    APP_ERROR Check(const std::vector<TensorBase>& tensors, const std::vector<ResizedImageInfo>& resizedImageInfos);
    void NmsSort(std::vector<Yolov8PoseDetectionInfo>& detBoxes, float iouThresh, IOUMethod method = IOUMethod::UNION);
    // float CalculateIOU(const Yolov8PoseDetectionInfo& a, const Yolov8PoseDetectionInfo& b, IOUMethod method);
    
    int numKeyPoints_ = 17;
    int paddingType_ = 1;
    float iouThresh_ = 0.45;
};

#ifdef ENABLE_POST_PROCESS_INSTANCE
extern "C" {
std::shared_ptr<MxBase::Yolov8PosePostProcess> GetKeypointInstance();
}
#endif
}
#endif