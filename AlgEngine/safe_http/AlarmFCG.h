#ifndef __ALARMFCG_H__
#define __ALARMFCG_H__

#include "ClientFCG.h"
#include "ThreadSafeQueue.h"
#include "ThreadBase.h"
#include "AmqpClient.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class AlarmFCG : ThreadBase
{
public:
    AlarmFCG() {}
    ~AlarmFCG() noexcept { stop(); }

    void run() override;
    void start(std::string& viasAddr);
    void stop();
   
   ThreadSafeQueue<ReportInfo> dataQueue;
    
private:
    bool uploadImage(std::string& uid, std::string base64, std::string& imageUrl);
    boost::uuids::random_generator generator;
    std::vector<ServiceNode> serviceNodes_;
    std::shared_ptr<ClientFCG> clientVias;
    std::shared_ptr<ClientFCG> clientImgServer;
    std::shared_ptr<AmqpClient> clientMq;
    std::string viasAddr_;
};

#endif /* __ALARMFCG_H__ */