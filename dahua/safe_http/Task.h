#ifndef __TASK_H__
#define __TASK_H__

#include "commonDataType.h"
#include <vector>

class Task
{
public:
    Task() {}
    Task(const std::string& taskId, const std::string& channelId,
         const std::string& userChannelCode, const std::string& url,
         const std::string& saasExtParam, const std::string& uid,
         const std::string& status)
        : taskId_(taskId),
          channelId_(channelId),
          userChannelCode_(userChannelCode),
          url_(url),
          saasExtParam_(saasExtParam),
          uid_(uid),
          status_(status)
    {
    }
    bool start();
    bool stop();

    std::string taskId_;
    std::string channelId_;
    std::string userChannelCode_;
    std::string url_;
    std::string saasExtParam_;
    std::string uid_;
    std::string status_;
    std::vector<AlgInfo> algList;
};

#endif /* __TASK_H__ */