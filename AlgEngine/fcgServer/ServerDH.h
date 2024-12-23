#ifndef __SERVERDH_H__
#define __SERVERDH_H__

#include <ServerWrapper.h>
#include "ClientFCG.h"
#include "commonDataType.h"

class ServerDH : public ServerWrapper
{
public:
    ServerDH(const std::string& uri)
        : ServerWrapper(uri)
    {
    }
    ~ServerDH();

    void init();

private:
    void handleUploadImgPost(http_request request, void* customInfo);
    void handleNodeInfo(http_request request, void* customInfo);
    // 下发行为智能任务
    void handlePostBehaviorTask(http_request request, void* customInfo);
    // 删除行为智能任务
    void handleDelBehaviorTask(http_request request, void* customInfo);
    // 查询行为智能任务
    void handleGetBehaviorTask(http_request request, void* customInfo);


    CustomInfo taskManager;
    const std::string SERVER_REPORT_URI = "/VIAS/System/Node";
    // 3. 算子存储报警图片
    const std::string SAVE_IMG_URI = "/eagle-pic/picture/upload";
    // 4. 算子获取存储节点
    const std::string GET_NODE_INFO = "/VIAS/System/Node/Subscribe";
    // 请求方法: GET
    // URL:/VIAS/System/Configs/{configName}/
    // 2. 获取配置文件
    const std::string GET_CONFIG_URI = "/VIAS/System/Configs";
};

#endif /* __SERVERFCG_H__ */