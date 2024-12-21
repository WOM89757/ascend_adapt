#ifndef __AMQPCLIENT_H__
#define __AMQPCLIENT_H__

#include <amqpcpp.h>
#include <amqpcpp/libev.h>
// #include <amqpcpp/callbacks.h>
#include <ev.h>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <iostream>

class AmqpClient
{
public:
    using MessageHandler = std::function<void(const std::string &message)>;
    using onMessageReceivedHandler = std::function<void(const AMQP::Message &message)>;

    AmqpClient(const std::string &host, int port, const std::string &vhost = "/",
               const std::string &login = "guest", const std::string &password = "guest");
    ~AmqpClient();

    void declareComponents(const std::string &exchange, const std::string &queue,
                           const std::string &routing_key = "routing_key",
                           AMQP::ExchangeType exchange_type = AMQP::ExchangeType::direct);
    void publish(const std::string &exchange, const std::string &routing_key, const std::string &message);
    void subscribe(const std::string &queue);
    // void subscribe(const std::string &queue, const MessageHandler &handler);
    void handleMessage(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered);

    AMQP::MessageCallback onMessageReceived;
private:
    static void asyncHandler(struct ev_loop *loop, ev_async *w, int revents);
    void runEventLoop();
    void stopEventLoop();
    void onError(const std::string &message);
    void onConnectionReady();

    struct ev_loop *loop_;
    ev_async async_watcher_;
    std::unique_ptr<AMQP::LibEvHandler> handler_;
    std::unique_ptr<AMQP::TcpConnection> connection_;
    std::unique_ptr<AMQP::TcpChannel> channel_;
    std::thread loop_thread_;

    std::string host_;
    int port_;
    std::string vhost_;
    std::string login_;
    std::string password_;
};

#endif /* __AMQPCLIENT_H__ */