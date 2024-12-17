#include <atomic>
#include <thread>
#include <csignal>

#include "safe_http/ServerFCG.h"


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
    std::string port = "8080";
    std::string serverUri = ipAddress + ":" + port;
    std::string viasAddr = getenv("VIASAddr");
    viasAddr = serverUri;
    // Server
    ServerFCG server("http://" + serverUri, viasAddr);
    server.init();
    server.start();


    while (keep_running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // server.stop();
    return 0;
}