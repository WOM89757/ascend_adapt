#include "AlarmFCG.h"

void AlarmFCG::start(std::string& viasAddr)
{
    viasAddr_ = viasAddr;
    clientVias = std::make_shared<ClientFCG>("http://" + viasAddr_);
    clientVias->updateNodeInfo(serviceNodes_);
    startThr();
}

void AlarmFCG::stop()
{
    stopThr();
}

void AlarmFCG::run()
{
    // while (isRunning())
    while (isRunning() || !dataQueue.empty())
    {
        std::cout << "Alarm Running in thread: " << std::this_thread::get_id() << " queue size: " << dataQueue.size() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto data = dataQueue.pop();
        // 图片转码  图片上传 告警上报
        

        //TODO report by rabbitmq
        json::value reportInfo;
        reportInfo["uid"] = json::value::string(data.uid);
        reportInfo["channelId"] = json::value::string(data.channelId);
        reportInfo["deviceId"] = json::value::string(data.deviceId);
        reportInfo["event"] = json::value::string(data.event);
        reportInfo["deviceCode"] = json::value::string(data.deviceCode);

        //imageList
        std::string ImageUrl;
        std::string base64;
        if (!uploadImage(data.uid, base64, ImageUrl))
        {
            continue;
        }

        // rabbitmq.send(reportInfo);

            // {
//     "uid": "8",
//     "channelId": "123456",
//     "deviceId": "xxxx",
//     "event": "behaviorAlarm",
//     "deviceCode": "xxxx",
//     "userDeviceCode": "xxx",
//     "channelCode": "xxxx",
//     "userChannelCode": "xxx",
//     "saasExtParam": "xxx",
//     "taskId": "xxx",
//     "recordId": "xxxx",
//     "paasId": "1",
//     "alarmTime": 1506731150010,
//     "alarmType": "302",
//     "alarmAction": "start",
//     "alarmName": "东门区域入侵报警",
//     "imgList": [
//         {
//             "imgType": 1,
//             "imgWidth": 1920,
//             "imgHeight": 1080,
//             "capTime": 1506731150010,
//             "objectRect": [
//                 {
//                     "objLeft": 0,
//                     "objTop": 0,
//                     "objRight": 512,
//                     "objBottom": 512,
//                     "absoluteObjLeft": 0,
//                     "absoluteObjTop": 0,
//                     "absoluteObjRight": 512,
//                     "absoluteObjBottom": 512
//                 }
//             ],
//             "imgSize": 1024,
//             "imgUrl": "/efs_1ur8/12ha.jpg"
//         }
//     ],
//     "classType": "Normal",
//     "info": {
//         "detectRegion": [
//             [
//                 100,
//                 100
//             ],
//             [
//                 200,
//                 100
//             ]
//         ],
//         "track": [
//             [
//                 100,
//                 100
//             ],
//             [
//                 200,
//                 100
//             ]
//         ],
//         "uniformStyle": 1
//     }
// }


    }
    std::cout << "dataQueue size " << dataQueue.size() << std::endl;
    std::cout << "Alarm Thread " << std::this_thread::get_id() << " stopped. " << std::endl;
}

bool AlarmFCG::uploadImage(std::string& uid, std::string base64, std::string& imageUrl)
{
    /**
     * 1.  imagPath to base64
     * 2. image data ptr in memery to base64
     * 3. base64 data ptr
    */
    // TODO image to base64
    boost::uuids::uuid uuid = generator();
    std::cout << "Generated UUID: " << uuid << std::endl;
    std::string uuidStr = boost::uuids::to_string(uuid);
    if (!clientImgServer)
    {
        for(auto tmp : serviceNodes_)
        {
           auto uri =  "http://" + tmp.getUri();
            //   std::cout << "imageserver uri: "<< uri << std::endl;
            if (!clientImgServer)
            {
                std::cout << "startserver create  image uri: "<< uri << std::endl;
                clientImgServer = std::make_shared<ClientFCG>(uri);
                break;
            }
        }
    }
    if (!clientImgServer)
    {
        std::cout << "uploadImag(): clientImgServer is Null" << std::endl;
        return false;
    }
    std::string errorResult;
    imageUrl = clientImgServer->uploadImg(uuidStr, uid, base64, errorResult);
    std::cout << "imageUrl :  " << imageUrl << std::endl;
    if(imageUrl.empty())
    {
        std::cerr << "uploadImg failed, " << errorResult << " at uri: " << clientImgServer->uri_ << std::endl;
        for(auto tmp : serviceNodes_)
        {
           auto uri =  "http://" + tmp.getUri();
            if (clientImgServer && clientImgServer->uri_ != uri)
            {
                // std::cout << "startserver new create  image uri: "<< uri << std::endl;
                clientImgServer.reset();
                clientImgServer = std::make_shared<ClientFCG>(uri);
                break;
            }
        }
        return false;
    }
    return true;
}