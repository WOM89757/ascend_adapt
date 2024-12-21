#ifndef COMMON_HEADERS_H
#define COMMON_HEADERS_H

#include <iostream>
#include <map>
#include <fstream>
#include <memory>
#include <queue>
#include <thread>
#include "unistd.h"
#include "boost/filesystem.hpp"

#include "MxBase/DeviceManager/DeviceManager.h"
#include "MxBase/DvppWrapper/DvppWrapper.h"
#include "MxBase/MemoryHelper/MemoryHelper.h"
#include "MxBase/E2eInfer/ImageProcessor/ImageProcessor.h"
#include "MxBase/E2eInfer/VideoDecoder/VideoDecoder.h"
#include "MxBase/E2eInfer/VideoEncoder/VideoEncoder.h"
#include "MxBase/E2eInfer/DataType.h"
#include "MxBase/MxBase.h"
#include "MxBase/Log/Log.h"

extern "C"
{
#include <libavformat/avformat.h>
}

// Namespace for all common definitions
class ModuleManager;

namespace MFNameSpace
{

    // // Constants
    // const uint32_t SRC_WIDTH = 1920;
    // const uint32_t SRC_HEIGHT = 1080;
    // const uint32_t MAX_WIDTH = 3840;
    // const uint32_t MAX_HEIGHT = 2160;
    // const uint32_t MAX_FRAME_COUNT = 5000;
    // const uint32_t TARGET_FRAME_COUNT = 400;
    // const uint32_t DEVICE_ID = 0;
    // const uint32_t CHANNEL_ID = 1;
    // const uint32_t TIME_OUT = 3000; // ms
    // const uint32_t EOS_WAITE = 5;
    // const int DEFAULT_SRC_RATE = 60;
    // const int DEFAULT_MAX_BIT_RATE = 6000;

    struct AlgInitParams
    {
        std::string deviceName;
        uint32_t channelId;
        uint32_t targetFrameCount;
        std::string pushUrl;
        std::string saveUrl;
        std::string deviceId;
        std::string strModelPath;               // 部署配置所在路径
        std::vector<std::string> vAlgModelType; // 算法枚举标识
        std::vector<int> vGpuId;                // gpuid

        AlgInitParams()
        {
            Reset();
        }
        AlgInitParams(const AlgInitParams &temparam)
        {
            deviceName = temparam.deviceName;
            channelId = temparam.channelId;
            targetFrameCount = temparam.targetFrameCount;
            pushUrl = temparam.pushUrl;
            saveUrl = temparam.saveUrl;
            deviceId = temparam.deviceId;
            strModelPath = temparam.strModelPath;
            vAlgModelType.assign(temparam.vAlgModelType.begin(), temparam.vAlgModelType.end());
            vGpuId.assign(temparam.vGpuId.begin(), temparam.vGpuId.end());
        }
        AlgInitParams &operator=(const AlgInitParams &temparam)
        {

            deviceName = temparam.deviceName;
            channelId = temparam.channelId;
            targetFrameCount = temparam.targetFrameCount;
            pushUrl = temparam.pushUrl;
            saveUrl = temparam.saveUrl;
            deviceId = temparam.deviceId;
            strModelPath = temparam.strModelPath;
            vAlgModelType.assign(temparam.vAlgModelType.begin(), temparam.vAlgModelType.end());
            vGpuId.assign(temparam.vGpuId.begin(), temparam.vGpuId.end());
            return *this;
        }
        void Reset()
        {
            deviceName.clear();
            channelId = 0;
            targetFrameCount = 0;
            pushUrl.clear();
            saveUrl.clear();
            deviceId.clear();
            strModelPath.clear();
            vAlgModelType.clear();
            vGpuId.clear();
        }
    };

    enum MFStatus
    {
        MF_EXECUTE_SUCCESS = 1,           // 执行成功。
        MF_INIT_FAIL = 2,                 // 初始化失败
        MF_UNINITIALIZED = 3,             // 模型未初始化
        MF_NET_NOT_SUPPORTED = 4,         // 网络模型不支持
        MF_LOAD_MODEL_FAILED = 5,         // 加载模型失败
        MF_INVALID_ARGUMENT = 6,          // 参数无效
        MF_OUT_OF_MEMORY = 7,             // 显存或者内存不足
        MF_OUT_OF_DETECT_RANGE = 8,       // 超出检测范围
        MF_STATUS_CHIP_NOT_SUPPORTED = 9, // 当前要调用的模块 加密芯片不支持
        MF_DLL_NOT_MATCH_MODEL = 10,      // 调用的模型与DLL不匹配
        MF_DLLS_INCOMPATIBLE = 11,        // 调用的DLL不一致
        MF_YML_ERROR = 12,                // 配置解析失败
        MF_INFERENCE_FAIL = 13,           // 推理失败
        MF_NET_INPUTUOTPUT_ERROR = 14,    // 输入输出无效
        MF_PREPROCESS_FAIL = 15,          // 预处理失败,图像为空
        MF_FREE_FAIL = 16,                // 释放内存失败
    };

    // Alias for filesystem
    namespace fs = boost::filesystem;

    // FrameImage structure definition
    struct FrameImage
    {
        MxBase::Image image; // video Image Class
        uint32_t frameId = 0;
        uint32_t channelId = 0;
    };

    struct FrameInfo
    {
        int height;
        int width;
        std::string videoSavedPath;
    };

    struct InitParam
    {
        std::string pushUrlPath;
        std::string popUrlPath; // 新增参数
        uint32_t deviceId;
        uint32_t channelId;
        uint32_t targetFrameCount;

        // 默认构造函数
        InitParam()
            : pushUrlPath(""), popUrlPath(""), deviceId(0), channelId(0), targetFrameCount(0) {}

        // 带参构造函数
        InitParam(const std::string &pushUrl, const std::string &popUrl, uint32_t device, uint32_t channel, uint32_t frameCount)
            : pushUrlPath(pushUrl), popUrlPath(popUrl), deviceId(device), channelId(channel), targetFrameCount(frameCount) {}

        // 拷贝构造函数
        InitParam(const InitParam &other)
            : pushUrlPath(other.pushUrlPath),
              popUrlPath(other.popUrlPath),
              deviceId(other.deviceId),
              channelId(other.channelId),
              targetFrameCount(other.targetFrameCount) {}

        // 拷贝赋值操作符
        InitParam &operator=(const InitParam &other)
        {
            if (this != &other)
            {
                pushUrlPath = other.pushUrlPath;
                popUrlPath = other.popUrlPath;
                deviceId = other.deviceId;
                channelId = other.channelId;
                targetFrameCount = other.targetFrameCount;
            }
            return *this;
        }

        // 移动构造函数
        InitParam(InitParam &&other) noexcept
            : pushUrlPath(std::move(other.pushUrlPath)),
              popUrlPath(std::move(other.popUrlPath)),
              deviceId(other.deviceId),
              channelId(other.channelId),
              targetFrameCount(other.targetFrameCount)
        {
            // 将other置为默认状态
            other.deviceId = 0;
            other.channelId = 0;
            other.targetFrameCount = 0;
        }

        // 移动赋值操作符
        InitParam &operator=(InitParam &&other) noexcept
        {
            if (this != &other)
            {
                pushUrlPath = std::move(other.pushUrlPath);
                popUrlPath = std::move(other.popUrlPath);
                deviceId = other.deviceId;
                channelId = other.channelId;
                targetFrameCount = other.targetFrameCount;

                // 将other置为默认状态
                other.deviceId = 0;
                other.channelId = 0;
                other.targetFrameCount = 0;
            }
            return *this;
        }
    };

}
#endif // COMMON_HEADERS_H
