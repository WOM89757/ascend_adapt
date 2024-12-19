#include "ServerDH.h"

ServerDH::~ServerDH() { stop(); }

void ServerDH::init()
{
    std::string context_data = "AI TaskManager";
    
    taskManager.name = context_data;

    // 模拟测试算法仓接口
    add_get_route(GET_CONFIG_URI,
                  [this](http_request request) {
                      handleGetBehaviorTask(request, (void*)&taskManager);
                  });
    // add_post_route(client_->SERVER_REPORT_URI,
    //                [this](http_request request) {
    //                    handlePostFaceExtract(request, (void*)&taskManager);
    //                });
    add_post_route(SAVE_IMG_URI,
                   [this](http_request request) {
                       handleUploadImgPost(request, (void*)&taskManager);
                   });
    add_post_route(GET_NODE_INFO,
                   [this](http_request request) {
                       handleNodeInfo(request, (void*)&taskManager);
                   });
}

void ServerDH::handleUploadImgPost(http_request request, void* customInfo)
{
    request.extract_json()
        .then([request, customInfo](json::value body) mutable {
            std::cout << "handleUploadImgPost Receive: " << body.serialize()
                      << std::endl;

            json::value response;
            response["code"] = json::value::string("1");
            response["message"] = json::value::string("success");
            // Test for SAVE_IMG_URI
            response["data"]["imageUrl"] = json::value::string("/efs_1ur8/12ha.jpg");
            request.reply(status_codes::OK, response);
        })
        .wait();
}
void ServerDH::handleNodeInfo(http_request request, void* customInfo)
{
    request.extract_json()
        .then([request, customInfo](json::value body) mutable {
            std::cout << "handleNodeInfo Receive: " << body.serialize()
                      << std::endl;

            json::value response;
            response["code"] = json::value::string("1");
            response["message"] = json::value::string("success");

            // Test for GET_NODE_INFO
            for (int i = 0; i < 1; i++)
            {
                response["data"][i]["serviceName"] =
                    json::value::string("CVEngine-Feature");
                // response["data"][i]["serviceNodes"][0]["host"] =
                //     json::value::string("10.31.17.222");
                // response["data"][i]["serviceNodes"][0]["port"] =
                //     json::value::string("444" + std::to_string((rand() % 9)));
                
                response["data"][i]["serviceNodes"][0]["host"] =
                    json::value::string("0.0.0.0");
                response["data"][i]["serviceNodes"][0]["port"] =
                    json::value::string("8081");
            }
            request.reply(status_codes::OK, response);
        })
        .wait();
}


void ServerDH::handlePostBehaviorTask(http_request request, void* customInfo)
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
    // request.extract_json()
    //     .then([request, customInfo, this](json::value body) mutable {
    //         std::cout << "handlePostBehaviorTask Receive: " << body.serialize()
    //                   << std::endl;
    //         json::value response;
    //         if (!(body.has_field("taskId") && body.has_field("channelId")))
    //         {
    //             response["code"] = json::value::string("-1");
    //             response["message"] = json::value::string("Field Is Not Exist");
    //             request.reply(status_codes::OK, response);
    //         }
    //         std::string taskId = body["taskId"].as_string();
    //         std::string channelId = body["channelId"].as_string();
    //         std::string userChannelCode = body["userChannelCode"].as_string();
    //         std::string url = body["url"].as_string();
    //         std::string saasExtParam = body["saasExtParam"].as_string();
    //         std::string uid = body["uid"].as_string();
    //         auto algorithms = body["algorithms"].as_array();

    //         // std::cout << "taskId: " << taskId << std::endl;
    //         // std::cout << "uid: "<< uid << std::endl;
    //         // std::cout << algorithms.size() << std::endl;

    //         // TODO 解析算法字段 启动对应算法 若启动失败则返回对应字段
    //         std::unique_ptr<Task> task =  std::make_unique<Task>(viasAddr_ ,taskId, channelId, userChannelCode, url, saasExtParam, uid, 3);
    //         if (!addTask(task))
    //         {
    //             response["code"] = json::value::string("-1");
    //             response["message"] = json::value::string("add task faild!");
    //         }
    //         else
    //         {
    //             response["code"] = json::value::string("0");
    //             response["message"] = json::value::string("success");
    //         }
    //         request.reply(status_codes::OK, response);
    //     })
    //     .wait();
}

void ServerDH::handleDelBehaviorTask(http_request request, void* customInfo)
{
    // auto path = uri::decode(request.relative_uri().path());
    // auto segments = uri::split_path(path);

    // json::value response;
    // if (segments.size() >= 3 && segments[0] == "VIID" &&
    //     segments[1] == "Behavior" && segments[2] == "Task")
    // {
    //     std::string taskId = segments[3];
    //     // 删除逻辑
    //     if (delTask(taskId))
    //     {
    //         response["code"] = json::value::string("0");
    //         response["message"] = json::value::string("success");
    //         Logger::log("Deleting task with ID: " + taskId);
    //     }
    //     else
    //     {
    //         response["code"] = json::value::string("-1");
    //         response["message"] = json::value::string("del task faild");
    //         Logger::log("Deleting task with ID: " + taskId + " Faild!");
    //     }
    // }
    // else
    // {
    //     response["code"] = json::value::string("-1");
    //     response["message"] = json::value::string("Invalid Task ID");
    // }
    // request.reply(status_codes::OK, response);
}

void ServerDH::handleGetBehaviorTask(http_request request, void* customInfo)
{
    // json::value response = getTaskList();
    // std::cout << "handleGetBehaviorTask: " << response.serialize() << std::endl;
    // request.reply(status_codes::OK, response);
}