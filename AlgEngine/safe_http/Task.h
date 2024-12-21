#ifndef __TASK_H__
#define __TASK_H__

#include "commonDataType.h"
#include "ThreadBase.h"
#include "AlarmFCG.h"
#include <vector>
#include <queue>

#include "FFmpegCodecImpl.h"
// #include "MFManagerDataType.h"

class Task : public ThreadBase
{
public:
    Task() {}
    Task(const std::string& viasAddr, const std::string& taskId, const std::string& channelId,
         const std::string& userChannelCode, const std::string& url,
         const std::string& saasExtParam, const std::string& uid,
         const int& state)
        : viasAddr_(viasAddr),
          taskId_(taskId),
          channelId_(channelId),
          userChannelCode_(userChannelCode),
          url_(url),
          saasExtParam_(saasExtParam),
          uid_(uid),
          state_(state),
          code_(""),
          message_("")
    {
    }
    ~Task() { stop(); }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&&) = default;
    Task& operator=(Task&&) = default;

    bool start();
    bool stop();

    std::string viasAddr_;
    std::string taskId_;
    std::string channelId_;
    std::string userChannelCode_;
    std::string url_;
    std::string saasExtParam_;
    std::string uid_;
    int state_;
    std::string code_;
    std::string message_;
    AlarmFCG alarmFCG;
    // std::shared_ptr<ModuleManager> moduleManager;
   ModuleManager* moduleManager;

private:
    void run() override;
};

#endif /* __TASK_H__ */