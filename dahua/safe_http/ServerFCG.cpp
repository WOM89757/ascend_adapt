#include "ServerFCG.h"

ServerFCG::~ServerFCG() { stop(); }

void ServerFCG::init()
{
    std::string context_data = "AI TaskManager";
    
    taskManager.name = context_data;
    add_post_route("/VIID/Face/Extract",
                   [this](http_request request) {
                       handlePostFaceExtract(request, (void*)&taskManager);
                   });
    add_post_route("/VIID/Behavior/Task",
                   [this](http_request request) {
                       handlePostBehaviorTask(request, (void*)&taskManager);
                   });
    add_delete_route("/VIID/Behavior/Task",
                     [this](http_request request) {
                         handleDelBehaviorTask(request, (void*)&taskManager);
                     });
    add_get_route("/VIID/Behavior/Task",
                  [this](http_request request) {
                      handleGetBehaviorTask(request, (void*)&taskManager);
                  });

    // 启动定时任务
    client_ = std::make_shared<ClientFCG>("http://" + viasAddr_);
    // // 模拟测试算法仓接口
    // add_get_route(client_->GET_CONFIG_URI,
    //               [this](http_request request) {
    //                   handleGetBehaviorTask(request, (void*)&taskManager);
    //               });
    // add_post_route(client_->SERVER_REPORT_URI,
    //                [this](http_request request) {
    //                    handlePostFaceExtract(request, (void*)&taskManager);
    //                });
    // add_post_route(client_->SAVE_IMG_URI,
    //                [this](http_request request) {
    //                    handlePostFaceExtract(request, (void*)&taskManager);
    //                });
    // add_post_route(client_->GET_NODE_INFO,
    //                [this](http_request request) {
    //                    handlePostFaceExtract(request, (void*)&taskManager);
    //                });
    // // 上报服务器状态，部分动态参数需进一步传入
    // client_->reportStatus();
    // // 更新存储节点信息
    // client_->updateNodeInfo(serviceNodes_);
    // // 获取配置文件信息
    // client_->getConfigFile();
    // TODO 图片转码及上传 test
    // client_->uploadImg();
}

void ServerFCG::handlePostFaceExtract(http_request request, void* customInfo)
{
    if (customInfo != nullptr)
    {
        CustomInfo* custom = static_cast<CustomInfo*>(customInfo);
        std::cout << "custom: " << custom->name << std::endl;
    }

    // std::cout << "resquest:  " << request.body() << " " <<
    // request.to_string() << std::endl;
    request.extract_json()
        .then([request, customInfo](json::value body) mutable {
            std::cout << "handlePostFaceExtract Receive: " << body.serialize()
                      << std::endl;
            // std::cout << "handlePostFaceExtract Receive: " <<
            // body.to_string() << std::endl;

            // TODO 解析算法字段 启动对应算法 若启动失败则返回对应字段

            json::value response;
            response["code"] = json::value::string("-1");
            response["message"] = json::value::string("接口未实现");
            // Test for GET_NODE_INFO
            response["data"][0]["serviceName"] =
                json::value::string("CVEngine-Feature");
            response["data"][0]["serviceNodes"][0]["host"] =
                json::value::string("10.31.17.222");
            response["data"][0]["serviceNodes"][0]["port"] =
                json::value::string("4444");
            request.reply(status_codes::OK, response);
        })
        .wait();
}


void ServerFCG::handlePostBehaviorTask(http_request request, void* customInfo)
{
    // {
    // "taskId":"1"
    // "channelId":"12334566",
    // "userChannelCode": "",
    // "url":"rtsp://10.31.53.238:8319/dss/monitor/param?substream=1&trackID=0&cameraid=6520638774511808",
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
    request.extract_json()
        .then([request, customInfo, this](json::value body) mutable {
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
            Task task(taskId, channelId, userChannelCode, url, saasExtParam, uid, 3);
            if (!addTask(task))
            {
                response["code"] = json::value::string("-1");
                response["message"] = json::value::string("add task faild!");
            }
            else
            {
                response["code"] = json::value::string("0");
                response["message"] = json::value::string("success");
            }
            request.reply(status_codes::OK, response);
        })
        .wait();
}

void ServerFCG::handleDelBehaviorTask(http_request request, void* customInfo)
{
    auto path = uri::decode(request.relative_uri().path());
    auto segments = uri::split_path(path);

    json::value response;
    if (segments.size() >= 3 && segments[0] == "VIID" &&
        segments[1] == "Behavior" && segments[2] == "Task")
    {
        std::string taskId = segments[3];
        Logger::log("Deleting task with ID: " + taskId);
        // TODO 删除逻辑
        if (delTask(taskId))
        {
            response["code"] = json::value::string("0");
            response["message"] = json::value::string("success");
        }
        else
        {
            response["code"] = json::value::string("-1");
            response["message"] = json::value::string("del task faild");
        }
    }
    else
    {
        response["code"] = json::value::string("-1");
        response["message"] = json::value::string("Invalid Task ID");
    }
    request.reply(status_codes::OK, response);
}

void ServerFCG::handleGetBehaviorTask(http_request request, void* customInfo)
{
    json::value response = getTaskList();
    std::cout << "handleGetBehaviorTask: " << response.serialize() << std::endl;
    request.reply(status_codes::OK, response);
}

bool ServerFCG::addTask(Task& task)
{
    auto iter = taskManagerMap.find(task.taskId_);
    if (iter != taskManagerMap.end())
    {
        return false;
    }
    //TODO start task
    task.start();
    taskManagerMap[task.taskId_] = task;
    std::cout << "taskMap size: " << taskManagerMap.size() << std::endl;
    return true;
}

json::value ServerFCG::getTaskList()
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
    // 根据算法任务运行情况，构造算法任务列表
    int i = 0;
    for(auto iter : taskManagerMap)
    {
        auto taskTmp = iter.second;

        json::value task;
        task["taskId"] = json::value::string(taskTmp.taskId_);
        task["channelId"] = json::value::string(taskTmp.channelId_);
        task["state"] = taskTmp.state_;
        task["code"] = json::value::string(taskTmp.code_);
        task["message"] = json::value::string(taskTmp.message_);
        response["data"][i] = task;
        i++;
    }
    return response;
}
bool ServerFCG::delTask(std::string& taskId)
{
    auto iter = taskManagerMap.find(taskId);
    if (iter == taskManagerMap.end())
    {
        return false;
    }
    taskManagerMap[taskId].stop();
    taskManagerMap.erase(taskId);
    return true;
}