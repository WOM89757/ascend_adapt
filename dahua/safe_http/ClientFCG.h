#ifndef __CLIENTFCG_H__
#define __CLIENTFCG_H__

#include "ClientWrapper.h"
#include "Timer.h"
#include "commonDataType.h"

class ClientFCG : public ClientWrapper
{
public:
    uint32_t timerInterval;

    explicit ClientFCG(const std::string& uri);
    ~ClientFCG();
    void reportStatus();
    void updateNodeInfo(std::vector<ServiceNode>& ServiceNodes);
    void getConfigFile();
    std::string uploadImg(std::string& uuid, std::string& uid, std::string& base64);

    // private:
    // 1. 服务上报
    const std::string SERVER_REPORT_URI = "/VIAS/System/Node";
    // 3. 算子存储报警图片
    const std::string SAVE_IMG_URI = "/eagle-pic/picture/upload";
    // 4. 算子获取存储节点
    const std::string GET_NODE_INFO = "/VIAS/System/Node/Subscribe";
    // 请求方法: GET
    // URL:/VIAS/System/Configs/{configName}/
    // 2. 获取配置文件
    const std::string GET_CONFIG_URI = "/VIAS/System/Configs";
    Timer reportStatusTimer;
    Timer updateNodeInfoTimer;
    Timer getConfigFileTimer;
};

#endif /* __CLIENTFCG_H__ */