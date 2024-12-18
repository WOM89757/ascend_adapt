#include "Task.h"

bool Task::start()
{
    //TODO init decode
    //TODO init alg infer

    // start infer thread
    startThr();
    alarmFCG.start(viasAddr_);
    return true;
}


bool Task::stop()
{
    // stop infer thread
    stopThr();
    alarmFCG.stop();
    return true;
}

void Task::run()
{
    while (isRunning())
    {
        std::cout << "Running in thread: " << std::this_thread::get_id() << " taskId: " << taskId_  << " url: " << url_ << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        //TODO infer image, get alter result
        //TODO push alter to queue
        std::string label = "test";
        // alarmQueue.push(AlarmInfo(0.5, label));
        // alarmFCG.dataQueue.push(AlarmInfo(0.5, label));
        ReportInfo reportInfo;
        reportInfo.uid = uid_;
        reportInfo.channelId = channelId_;
        reportInfo.userChannelCode = userChannelCode_;
        reportInfo.saasExtParam = saasExtParam_;
        reportInfo.taskId = taskId_;
        alarmFCG.dataQueue.push(reportInfo);
        // std::cout << "alarmQueue size " << alarmFCG.dataQueue.size() << std::endl;
        

    }
    std::cout << "Thread " << std::this_thread::get_id() << " stopped."
              << std::endl;
}