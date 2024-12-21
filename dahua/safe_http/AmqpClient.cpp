#include "AmqpClient.h"

AmqpClient::AmqpClient(const std::string &host, int port, const std::string &vhost,
                       const std::string &login, const std::string &password)
    : host_(host), port_(port), vhost_(vhost), login_(login), password_(password)
{
    loop_ = ev_loop_new(0);
    ev_async_init(&async_watcher_, asyncHandler);
    ev_async_start(loop_, &async_watcher_);

    handler_ = std::make_unique<AMQP::LibEvHandler>(loop_);
    connection_ = std::make_unique<AMQP::TcpConnection>(handler_.get(),
                                                        AMQP::Address(host, port, AMQP::Login(login, password), vhost));
    channel_ = std::make_unique<AMQP::TcpChannel>(connection_.get());

    // onMessageReceived = [](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
    //     std::cout << "Defaut Message Handling: " << std::string(message.body(), message.body() + message.bodySize()) << std::endl;
    // };

    loop_thread_ = std::thread(&AmqpClient::runEventLoop, this);
}

AmqpClient::~AmqpClient()
{
    if (connection_)
    {
        connection_->close();
        connection_.reset();
        std::cout << " connection close ---------" << std::endl;
    }
    stopEventLoop();
    if (loop_thread_.joinable())
    {
        loop_thread_.join();
    }
    // ev_loop_destroy(loop_);
}

void AmqpClient::declareComponents(const std::string &exchange, const std::string &queue,
                                   const std::string &routing_key, AMQP::ExchangeType exchange_type)
{
    channel_->declareExchange(exchange, exchange_type)
        .onError([exchange](const char *message)
                 {
            std::cerr << "Failed to declare exchange '" << exchange << "': " << message << std::endl;
            })
        .onSuccess([exchange]()
                   { std::cout << "Exchange '" << exchange << "' declared successfully." << std::endl;});

    channel_->declareQueue(queue, 3)
    // channel_->declareQueue(queue)
        .onError([queue](const char *message)
                 {
            std::cerr << "Failed to declare queue '" << queue << "': " << message << std::endl;
             })
        .onSuccess([queue]()
                   { std::cout << "Queue '" << queue << "' declared successfully." << std::endl; });

    channel_->bindQueue(exchange, queue, routing_key)
        .onError([exchange, queue](const char *message)
                 {
            std::cerr << "Failed to bind queue '" << queue << "' to exchange '" << exchange << "': " << message << std::endl;
             })
        .onSuccess([exchange, queue, routing_key]()
                   { std::cout << "Queue '" << queue << "' bound to exchange '" << exchange << "' with routing key '" << routing_key << "' successfully." << std::endl; });
}

void AmqpClient::publish(const std::string &exchange, const std::string &routing_key, const std::string &message)
{
    channel_->publish(exchange, routing_key, message);
}

void AmqpClient::subscribe(const std::string &queue)
{
    channel_->consume(queue)
        .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                    { 
                    try {
                        // std::string body(message.body(), message.body() + message.bodySize());
                        // std::cout << "Processing Message: " << body << std::endl;
                        // handler(std::string(message.body(), message.bodySize()));
                        // // 手动确认消息
                        // channel_->ack(deliveryTag);

                        handleMessage(message, deliveryTag, redelivered);

                    } catch (const std::exception &e) {
                        // 处理异常，拒绝消息
                        channel_->reject(deliveryTag);
                        std::cerr << "Error processing message: " << e.what() << std::endl;
                    } })
        .onError([queue](const char *message)
                 { std::cerr << "Failed to consume queue '" << queue << "': " << message << std::endl; });
}

void AmqpClient::handleMessage(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
{
    // 打印消息内容
    std::cout << "Received Message: " << std::string(message.body(), message.body() + message.bodySize()) << std::endl;

    // 扩展自定义逻辑
    if (onMessageReceived)
    {
        onMessageReceived(message, deliveryTag, redelivered);
    }

    // 手动确认
    channel_->ack(deliveryTag);
}

void AmqpClient::asyncHandler(struct ev_loop *loop, ev_async *w, int revents)
{
    // std::cout << "fd = " << w->fd << std::endl;
    ev_break(loop, EVBREAK_ALL);
}

void AmqpClient::runEventLoop()
{
    ev_run(loop_, 0);
}

void AmqpClient::stopEventLoop()
{
    // ev_async_init(&async_watcher_, asyncHandler);
    // ev_async_start(loop_, &async_watcher_);
    // ev_async_send(loop_, &async_watcher_);

    ev_async_send(loop_, &async_watcher_);
}

void AmqpClient::onError(const std::string &message)
{
    std::cerr << "[AMQP Error]: " << message << std::endl;
}

void AmqpClient::onConnectionReady()
{
    std::cout << "[AMQP Info]: Connection is ready." << std::endl;
}
