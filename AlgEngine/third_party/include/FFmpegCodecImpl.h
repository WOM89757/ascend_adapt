#pragma once
#include "MFManagerDataType.h"
using namespace MFNameSpace;

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef _WIN32
#ifdef MF_FFMPEGCODEC_DLL_EXPORT
#define MF_FFMPEGCODEC_API __declspec(dllexport)
#else
#define MF_FFMPEGCODEC_API __declspec(dllimport)
#endif
#else
#define MF_FFMPEGCODEC_API
#endif

  /**************************************************************************
   * @InitModel									: 模型初始化
   * @AlgInitParams algInitParam[in]						: 算法初始化结构体
   * @UpdateParam updateParam[in]						: 更新参数结构体
   * @MFStatus *Status[out]						: 状态信息
   * @return MFPredictProjHandle* pMFProjHandle[Out]	: 预测句柄
   ***************************************************************************/
  MF_FFMPEGCODEC_API ModuleManager *InitCodec(const AlgInitParams &algInitParam, MFStatus *Status);

  /**************************************************************************
   * @MFReleaseProject                       : 释放项目预测工程
   * @MFTreePredictHandle nProjHandle[in]    : 项目预测句柄
   * @return MFStatus										: 状态信息
   ***************************************************************************/
  MF_FFMPEGCODEC_API MFStatus ReleaseCodec(ModuleManager *pMFProjHandle);

#ifdef __cplusplus
}
#endif
