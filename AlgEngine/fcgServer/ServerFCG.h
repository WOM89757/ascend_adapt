#ifndef __SERVERFCG_H__
#define __SERVERFCG_H__

#include <ServerWrapper.h>
#include "ClientFCG.h"
#include "commonDataType.h"
#include "Task.h"

class ServerFCG : public ServerWrapper
{
public:
    ServerFCG(const std::string& uri, const std::string& viasAddr)
        : ServerWrapper(uri)
    {
        viasAddr_ = viasAddr;
    }
    ~ServerFCG();

    void init();

private:
    // 人脸图片特征提取接口
    void handlePostFaceExtract(http_request request, void* customInfo);
    // 下发行为智能任务
    void handlePostBehaviorTask(http_request request, void* customInfo);
    // 删除行为智能任务
    void handleDelBehaviorTask(http_request request, void* customInfo);
    // 查询行为智能任务
    void handleGetBehaviorTask(http_request request, void* customInfo);
    bool addTask(std::unique_ptr<Task>& task);
    json::value getTaskList();
    bool delTask(std::string& taskId);


    std::string viasAddr_;
    std::shared_ptr<ClientFCG> client_;
    CustomInfo taskManager;
    std::map<std::string, std::unique_ptr<Task>> taskManagerMap;
};

#endif /* __SERVERFCG_H__ */