#include <atomic>
#include <thread>
#include <csignal>

#include "fcgServer/ServerFCG.h"
#include "fcgServer/ServerDH.h"

std::atomic<bool> keep_running(true);

void signal_handler(int signal)
{
    Logger::log("Received signal to stop server.");
    keep_running.store(false);
}

int main()
{

    // Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string ipAddress = "0.0.0.0";
    std::string port = "18080";
    std::string serverUri = ipAddress + ":" + port;
    std::string viasAddr = getenv("VIASAddr");
    // Server
    ServerFCG server("http://" + serverUri, viasAddr);
    server.init();
    server.start();

    std::string serverUriDh = ipAddress + ":18081";
    ServerDH serverDh("http://" + serverUriDh);
    serverDh.init();
    serverDh.start();

    // AmqpClient client("192.168.31.115", 5673);
    // 模式: topic
    // exchange: tripartite_event
    // routeKey: event.tripartite.behaviorAlarm.[uid].[vendor]
    // client.declareComponents("tripartite_event", "example_queue-alarm", "event.tripartite.behaviorAlarm.[1].[020]", AMQP::ExchangeType::topic);
    // client.onMessageReceived = [](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    // {
    //     std::cout << "Custom Message Handling: " << std::string(message.body(), message.body() + message.bodySize()) << std::endl;
    // };
    // client.subscribe("example_queue-alarm");
    // client.publish("tripartite_event", "event.tripartite.behaviorAlarm.[1].[020]", "Alarm...!");

    // client.declareComponents("example_exchange", "example_queue", "example_routing_key");
    // client.onMessageReceived = [](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    // {
    //     std::cout << "Custom Message Handling: " << std::string(message.body(), message.body() + message.bodySize()) << std::endl;
    // };
    // client.subscribe("example_queue");
    // client.publish("example_exchange", "example_routing_key", "Hello, World!");

    while (keep_running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // server.stop();
    return 0;
}