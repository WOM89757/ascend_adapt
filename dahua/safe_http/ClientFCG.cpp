#include "ClientFCG.h"

ClientFCG::ClientFCG(const std::string& uri) : ClientWrapper(uri)
{
    timerInterval = 3;
}

ClientFCG::~ClientFCG()
{
    reportStatusTimer.stop();
    updateNodeInfoTimer.stop();
    getConfigFileTimer.stop();
}

void ClientFCG::reportStatus()
{
    auto serverReportTask = [&]() {
        std::cout << "serverReportTask Task executed" << std::endl;
        json::value bodyInfo;
        // {
        //     "serviceName": "xxx",
        //     "host": "127.0.0.1",
        //     "port": 3435,
        //     "vendor": "014",
        //     "version": "3",
        //     "capabilitySet": {
        //         "pictureCapacity": {"max": 10, "free": 5},
        //         "videoCapacity": {"max": 10, "free": 5},
        //         "supportAlgorithm":
        //             [{"algorithmCode": "001_traffic", "version": "V1.0"}],
        //         "loadRate": 0.22
        //     },
        //     "serialNumber": "11"
        // }
        std::string ip = getenv("ip");
        std::string ports = getenv("ports"); //[1,2,3]
        if (ip.empty())
        {
            std::cout << "ip is empty from env!" << std::endl;
        }

        bodyInfo["serviceName"] = json::value::string("CommonScene");
        bodyInfo["host"] = json::value::string(ip);
        bodyInfo["port"] = json::value::string(ports);
        bodyInfo["vendor"] = json::value::string("019"); // 大华定义
        bodyInfo["version"] = json::value::string("v0.0.1");
        bodyInfo["capabilitySet"];
        json::value capabilitySet;
        capabilitySet["videoCapacity"]["max"] = 5;
        capabilitySet["videoCapacity"]["free"] = 4;
        json::value supportAlgorithm;
        {
            json::value algorithm;
            algorithm["algorithmCode"] = json::value::string("018_fs");
            algorithm["version"] = json::value::string("v0.0.1");
            supportAlgorithm[0] = algorithm;
        }
        capabilitySet["supportAlgorithm"] = supportAlgorithm;
        capabilitySet["loadRate"] = 0.6;
        bodyInfo["capabilitySet"] = capabilitySet;
        bodyInfo["serialNumber"] = json::value::string("env number");

        post(SERVER_REPORT_URI, bodyInfo);
    };
    reportStatusTimer.start(serverReportTask,
                            std::chrono::milliseconds(timerInterval * 1000));
    return;
}

void ClientFCG::updateNodeInfo(std::vector<ServiceNode>& serviceNodes)
{
    // TODO 请求构建及结果解析
    auto updateNodeInfoTask = [&]() {
        std::cout << "updateNodeInfoTask Task executed" << std::endl;
        json::value bodyInfo;

        // {
        // "serviceNames": ["CVEngine-Feature"],
        // }
        bodyInfo["serviceNames"][0] = json::value::string("CVEngine-Feature");

        post(GET_NODE_INFO, bodyInfo, &serviceNodes,
             [](json::value response, void* customInfo) {
                 std::cout << "[GET_NODE_INFO] Response: "
                           << response.serialize() << std::endl;
                 if (customInfo != nullptr)
                 {
                     std::vector<ServiceNode>* serviceNodes =
                         static_cast<std::vector<ServiceNode>*>(customInfo);

                     if (!response.has_field("data")) return;
                     auto data = response["data"].as_array();
                     for (auto service : data)
                     {
                         for (auto node : service["serviceNodes"].as_array())
                         {
                             serviceNodes->emplace_back(
                                 node["host"].as_string(),
                                 node["port"].as_string(),
                                 service["serviceName"].as_string());
                         }
                     }
                 }
                 else
                 {
                     std::cerr << "customInfo is null!" << std::endl;
                 }
             });

        // {
        //     "code" : "0", "message" : "success", "data" : [
        // {
        //         "serviceName": "CVEngine-Feature",
        //         "serviceNodes": [
        // {"host": "10.31.17.222", "port": 4444}
        // ]
        //     }]
        // }
    };
    updateNodeInfoTimer.start(updateNodeInfoTask,
                              std::chrono::milliseconds(timerInterval * 1000));
    return;
}

void ClientFCG::getConfigFile()
{
    CustomInfo customInfoForGetConfigFileTask;
    auto getConfigFileTask = [&]() {
        std::cout << "getConfigFileTask Task executed" << std::endl;

        get(GET_CONFIG_URI, &customInfoForGetConfigFileTask,
            [](json::value response, void* customInfo) {
                std::cout << "[GET_CONFIG_FILE] Response: "
                          << response.serialize() << std::endl;
                // {
                //     "code" : "0", "message" : "success", "data":
                //     {
                //         "configContent" : "{......}"
                //     }
                // }
                if (customInfo != nullptr)
                {
                    std::vector<CustomInfo>* serviceNodes =
                        static_cast<std::vector<CustomInfo>*>(customInfo);
                    // TODO parse config file
                }
            });
    };
    getConfigFileTimer.start(getConfigFileTask,
                             std::chrono::milliseconds(timerInterval * 1000));
}

std::string ClientFCG::uploadImg(std::string& uuid, std::string& uid, std::string& base64)
{
    std::cout << "uploadImg Task executed" << std::endl;
    json::value bodyInfo;

    // {
    // "fileName": "12345678",
    // "image": "base64",
    // "type": "capture",
    // "uid": "1",
    // "fileType": "alarm"
    // }

    bodyInfo["fileName"] = json::value::string(uuid);
    bodyInfo["image"] = json::value::string(base64);
    bodyInfo["type"] = json::value::string("capture");
    bodyInfo["uid"] = json::value::string(uid);
    bodyInfo["fileType"] = json::value::string("alarm");
    
    std::string imageUrl;
    post(SAVE_IMG_URI, bodyInfo, nullptr,
            [&](json::value response, void* customInfo) {
                std::cout << "[SAVE_IMG_URI] Response: "
                        << response.serialize() << std::endl;
                if (!response.has_field("data")) return;
                imageUrl = response["data"]["imageUrl"].as_string();
            });

    // {
    //     "code" : "0", "message" : "success", "data": { "imageUrl" : "xxxxxxx" }
    // }
    return imageUrl;
}