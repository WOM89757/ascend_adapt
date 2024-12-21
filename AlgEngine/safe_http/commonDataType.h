#ifndef __COMMONDATATYPE_H__
#define __COMMONDATATYPE_H__

#include <string>
#include <vector>

struct ServiceNode
{
    std::string host_;
    std::string port_;
    std::string serviceName_;

    ServiceNode() {};
    ServiceNode(std::string host, std::string port, std::string serviceName)
        : host_(host), port_(port), serviceName_(serviceName) {};

    std::string getUri() { return (host_ +":" + port_); }
};

struct CustomInfo
{
    std::string name;
};

struct AlgInfo
{
    std::string algorithmCode;
    std::string version;
};

struct AlarmInfo
{
    // ARect rect;
    float score;
    std::string label;
    AlarmInfo() {}
    AlarmInfo(float score, std::string& label) : score(score), label(label) {}
};

struct ObjectRect
{
    // 报警目标在全景图中的相对位置:
    int objLeft;
    int objTop;
    int objRight;
    int objBottom;
    int absoluteObjLeft;
    int absoluteObjTop;
    int absoluteObjRight;
    int absoluteObjBottom;
};
struct ImageInfo
{
    // 图片类型,参见 附录15
    int imgType;
    // 报警抓图分辨率宽,单位:像素
    int imgWidth;
    // 报警抓图分辨率高,单位:像素
    int imgHeight;
    // 图片抓拍时间, utc时间,单位毫秒;有可能抓拍多张图后再发报警消息,类似取证
    double capTime;
    // 目标框,可能一张报警图片存在多个目标
    std::vector<ObjectRect> objectRects;
    // 图片大小
    int imgSize;
    // 图片地址
    std::string imgUrl;
};

struct ReportInfo
{
    AlarmInfo alarmInfo;
    // 用户id,算子透传
    std::string uid;
    // 通道唯一id,算子透传
    std::string channelId;
    // 事件类型 ,行为报警为: behaviorAlarm
    std::string event;
    std::string deviceId; // ?
    // 记录的唯一id,保证全局唯一,生成规则参见附录3 ,算子自己生成
    std::string recordId;
    std::string deviceCode;
    std::string userDeviceCode;
    std::string channelCode;
    std::string userChannelCode;
    // 平台透传字段,透传 下发行为智能任务 中带的saasExtParam
    std::string saasExtParam;
    // 任务id,透传 下发行为智能任务 中的任务id
    std::string taskId;
    std::string paasId;
    // 报警时间, utc 0时区的时间,单位为毫秒
    double alarmTime;

    // 报警类型,见 附录16
    std::string alarmType;
    // 报警动作,start-开始,stop结束,pluse-脉冲,固定填写 pluse 即可
    std::string alarmAction;
    // 报警名称,不能重复。
    std::string alarmName;
    // 报警图片列表,当报警消息中不存在图片则为空
    std::vector<ImageInfo> imgList;
    std::string classType;
    // 报警信息,不同的alarmType对应的info信息不同
    // std::string info;?
    // 算子透传字段,可根据需求自定义
    // std::string extData

    ReportInfo() {}
};



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

#endif /* __COMMONDATATYPE_H__ */