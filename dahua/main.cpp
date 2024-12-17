#include <atomic>
#include <thread>
#include <csignal>
#include "safe_http/ServerWrapper.h"
#include "safe_http/ClientWrapper.h"
#include "safe_http/Timer.h"

// Global atomic flag to control server running state
std::atomic<bool> keep_running(true);

// Signal handler for clean shutdown
void signal_handler(int signal)
{
    Logger::log("Received signal to stop server.");
    keep_running.store(false);
}

struct CustomInfo
{
    std::string name;
};

struct ServiceNode
{
    std::string host_;
    std::string port_;
    std::string serviceName_;
    
    ServiceNode() {} ;
    ServiceNode(std::string host, std::string port, std::string serviceName) : host_(host), port_(port), serviceName_(serviceName) {};

    std::string getUri()
    {
        return (host_ + port_);
    }
};

void handle_post_hello(http_request request)
{
    request.extract_json().then([request](json::value body) mutable
                                {
        json::value response;
        response["received"] = body;
        response["message"] = json::value::string("Hello, POST Handler!");
        request.reply(status_codes::OK, response); })
        .wait();
}

void handlePostFaceExtract(http_request request, void* customInfo)
{
    if (customInfo != nullptr)
    {
        CustomInfo* custom = static_cast<CustomInfo*>(customInfo);    
        std::cout << "custom: " << custom->name << std::endl;
    }

    // std::cout << "resquest:  " << request.body() << " " << request.to_string() << std::endl;
    request.extract_json().then([request, customInfo](json::value body) mutable
                                {

        std::cout << "handlePostFaceExtract Receive: " << body.serialize() << std::endl;
        // std::cout << "handlePostFaceExtract Receive: " << body.to_string() << std::endl;
        
        //TODO 解析算法字段 启动对应算法 若启动失败则返回对应字段

        json::value response;
        response["code"] = json::value::string("-1");
        response["message"] = json::value::string("接口未实现");
        // Test for GET_NODE_INFO
        response["data"][0]["serviceName"] = json::value::string("CVEngine-Feature");
        response["data"][0]["serviceNodes"][0]["host"] = json::value::string("10.31.17.222");
        response["data"][0]["serviceNodes"][0]["port"] = json::value::string("4444");
        request.reply(status_codes::OK, response); })
        .wait();
}

// {
// "taskId":"1"
// "channelId":"12334566",
// "userChannelCode": "",
// "url": "rtsp://10.31.53.238:8319/dss/monitor/param?substream=1&trackID=0&cameraid=6520638774511808",
// "saasExtParam":"xxxx",
// "uid":"1"
// "algorithms":
//         [
//                 {
//                 "algorithmCode": "xxx",
//                 "version":"V1",
//                 "config": {}
//                 }
//         ]
// }
void handlePostBehaviorTask(http_request request, void* customInfo)
{
    request.extract_json()
        .then([request, customInfo](json::value body) mutable {
            std::cout << "handlePostBehaviorTask Receive: " << body.serialize()
                      << std::endl;
            json::value response;
            if (!(body.has_field("taskId") && body.has_field("channelId")))
            {
                response["code"] = json::value::string("-1");
                response["message"] = json::value::string("Field Is Not Exist");
                request.reply(status_codes::OK, response);
            }
            std::string taskId = body["taskId"].as_string();
            std::string channelId = body["channelId"].as_string();
            std::string userChannelCode = body["userChannelCode"].as_string();
            std::string url = body["url"].as_string();
            std::string saasExtParam = body["saasExtParam"].as_string();
            std::string uid = body["uid"].as_string();
            auto algorithms = body["algorithms"].as_array();

            // std::cout << "taskId: " << taskId << std::endl;
            // std::cout << "uid: "<< uid << std::endl;
            // std::cout << algorithms.size() << std::endl;

            // TODO 解析算法字段 启动对应算法 若启动失败则返回对应字段

            response["code"] = json::value::string("0");
            response["message"] = json::value::string("success");
            request.reply(status_codes::OK, response);
        })
        .wait();
}

void handleDelBehaviorTask(http_request request,void* customInfo)
{

    auto path = uri::decode(request.relative_uri().path());
    auto segments = uri::split_path(path);

    json::value response;
    if (segments.size() >= 3 && segments[0] == "VIID" && segments[1] == "Behavior" && segments[2] == "Task")
    {
        std::string taskId = segments[3];
        Logger::log("Deleting task with ID: " + taskId);
        // TODO 删除逻辑
        response["code"] = json::value::string("0");
        response["message"] = json::value::string("success");
    }
    else
    {
        response["code"] = json::value::string("-1");
        response["message"] = json::value::string("Invalid Task ID");
    }
    request.reply(status_codes::OK, response);
}

void handleGetBehaviorTask(http_request request, void* customInfo)
{
    json::value response;
    //     {
    // "code": "0",
    // "message": "success"
    // "data":[
    // "taskId":"1",
    // "channelId":"12334566",
    // "state":3,
    // "code":"authError",
    // "message":"auth timeout",
    // ]
    // }
    response["code"] = json::value::string("0");
    response["message"] = json::value::string("success");
    // TODO 根据算法任务运行情况，构造算法任务列表
    json::value task;
    task["taskId"] = json::value::string("1");
    task["channelId"] = json::value::string("12321");
    task["state"] = json::value::string("3");
    task["code"] = json::value::string("authError");
    task["message"] = json::value::string("auth timeout");
    response["data"][0] = task;
    response["data"][1] = task;
    std::cout << "handleGetBehaviorTask: " << response.serialize() << std::endl;

    request.reply(status_codes::OK, response);
}

#include <iomanip>
std::string get_time() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t（从1970年开始的秒数）
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 转换为 tm 结构
    std::tm utc_tm = *std::gmtime(&now_time_t); // 转为 UTC 时间结构
    utc_tm.tm_hour += 8;                        // 加上8小时偏移，转换为北京时间

    // 跨天处理
    if (utc_tm.tm_hour >= 24) {
        utc_tm.tm_hour -= 24;
        utc_tm.tm_mday += 1;
        // 注意处理跨月和跨年的情况
        if (utc_tm.tm_mday > 31) {
            utc_tm.tm_mday = 1;
            utc_tm.tm_mon += 1;
            if (utc_tm.tm_mon >= 12) {
                utc_tm.tm_mon = 0;
                utc_tm.tm_year += 1;
            }
        }
    }

    // 使用字符串流格式化时间
    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}


void on_success(json::value response, void* custom)
{
    std::cout << "[get] Response: " << response.serialize()
              << std::endl;
    if (custom != nullptr)
    {
        CustomInfo* customInfo = static_cast<CustomInfo*>(custom);
        customInfo->name = response["message"].as_string();
    }
}

const std::string SERVER_REPORT_URI = "/VIAS/System/Node";
const std::string SAVE_IMG_URI = "/eagle-pic/picture/upload";
const std::string GET_NODE_INFO = "/VIAS/System/Node/Subscribe";
// 请求方法: GET
// URL:/VIAS/System/Configs/{configName}/
const std::string GET_CONFIG_URI = "/VIAS/System/Configs";
// 1. 服务上报
// 3. 算子存储报警图片
// 4. 算子获取存储节点
// 2. 获取配置文件

int main()
{
    // Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string ipAddress = "0.0.0.0";
    std::string port = "8080";
    std::string serverUri = ipAddress + ":" + port;
    // Server
    ServerWrapper server("http://" + serverUri);

    server.add_get_route("/hello", handle_post_hello);

    std::string context_data = "AI TaskManager";
    CustomInfo taskManager;
    taskManager.name = context_data;
    server.add_post_route("/VIID/Face/Extract", [taskManager](http_request request)
                        { handlePostFaceExtract(request, (void*)&taskManager); });
    server.add_post_route("/VIID/Behavior/Task", [taskManager](http_request request)
                        { handlePostBehaviorTask(request, (void*)&taskManager); });
    server.add_delete_route("/VIID/Behavior/Task", [taskManager](http_request request)
                            { handleDelBehaviorTask(request, (void*)&taskManager); });
    server.add_get_route("/VIID/Behavior/Task", [taskManager](http_request request)
                        { handleGetBehaviorTask(request, (void*)&taskManager); });

    //模拟测试算法仓接口
    server.add_get_route(GET_CONFIG_URI, [taskManager](http_request request)
                        { handleGetBehaviorTask(request, (void*)&taskManager); });
    server.add_post_route(SERVER_REPORT_URI, [taskManager](http_request request)
                        { handlePostFaceExtract(request, (void*)&taskManager); });
    server.add_post_route(SAVE_IMG_URI, [taskManager](http_request request)
                        { handlePostFaceExtract(request, (void*)&taskManager); });
    server.add_post_route(GET_NODE_INFO, [taskManager](http_request request)
                        { handlePostFaceExtract(request, (void*)&taskManager); });

    server.start();

    // Client
    ClientWrapper client("http://" + serverUri);
    json::value body;
    body["taskId"] = 1;
    // body = json::value::string("ssssssss");
    // client.get("/hello");

    // test GET_CONFIG_URI
    CustomInfo customInfo;
    client.get(GET_CONFIG_URI,  &customInfo, on_success);
    std::cout << "customInfo : " << customInfo.name << std::endl;
    assert(customInfo.name == "success");

    client.post(SERVER_REPORT_URI, body);
    client.post(SAVE_IMG_URI, body);
    client.post(GET_NODE_INFO, body);

    Timer serverReportTimer;
    auto serverReportTask = [&]() {
        std::cout << "serverReportTask Task executed at: " << get_time() << std::endl;
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
        bodyInfo["vendor"] = json::value::string("019");//大华定义
        bodyInfo["version"] = json::value::string("v0.0.1");
        bodyInfo["capabilitySet"];
        json::value capabilitySet;
        capabilitySet["videoCapacity"]["max"] = 5;
        capabilitySet["videoCapacity"]["free"] = 4;
        json::value supportAlgorithm;
        {
            json::value algorithm;
            algorithm["algorithmCode"] =  json::value::string("018_fs");
            algorithm["version"] =  json::value::string("v0.0.1");
            supportAlgorithm[0] = algorithm;
        }
        capabilitySet["supportAlgorithm"] =supportAlgorithm;
        capabilitySet["loadRate"] = 0.6;
        bodyInfo["capabilitySet"] = capabilitySet;
        bodyInfo["serialNumber"] = json::value::string("env number");


        client.post(SERVER_REPORT_URI, bodyInfo);
    };
    serverReportTimer.start(serverReportTask , std::chrono::milliseconds(5 * 1000));
    //TODO 请求构建及结果解析
    Timer getNodeInfoTimer;
    std::vector<ServiceNode> serviceNodes;
    auto getNodeInfoTask = [&]( ) {
        std::cout << "getNodeInfoTask Task executed at: " << get_time() << std::endl;
        json::value bodyInfo;

        // {
        // "serviceNames": ["CVEngine-Feature"],
        // }
        bodyInfo["serviceNames"][0] = json::value::string("CVEngine-Feature");

        client.post(
            GET_NODE_INFO, bodyInfo, &serviceNodes,
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
    getNodeInfoTimer.start(getNodeInfoTask , std::chrono::milliseconds(5 * 1000));
    
    Timer getConfigFileTimer;
    CustomInfo customInfoForGetConfigFileTask;
    auto getConfigFileTask = [&]() {
        std::cout << "getConfigFileTask Task executed at: " << get_time()
                  << std::endl;

        client.get(
            GET_CONFIG_URI, &customInfoForGetConfigFileTask,
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
    getConfigFileTimer.start(getConfigFileTask , std::chrono::milliseconds(2 * 1000));


    //TODO 图片转码及上传

    while (keep_running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    serverReportTimer.stop();
    getNodeInfoTimer.stop();
    getConfigFileTimer.stop();
    server.stop();


    return 0;
}