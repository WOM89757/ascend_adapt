#ifndef __SERVERWRAPPER_H__
#define __SERVERWRAPPER_H__

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <map>
#include <functional>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Logger Utility
class Logger
{
public:
    static void log(const std::string &message)
    {
        std::cout << "[LOG]: " << message << std::endl;
    }
};

class ServerWrapper
{
public:
    explicit ServerWrapper(const std::string &uri);

    void add_get_route(const std::string &path, const std::function<void(http_request)> &handler);
    void add_post_route(const std::string &path, const std::function<void(http_request)> &handler);
    void add_delete_route(const std::string &path, const std::function<void(http_request)> &handler);
    void start();
    void stop();

private:

    static void default_not_found_handler(http_request request);
    void handle_request(http_request request, const method& httpMethod);
    void handle_exception(http_request request, const std::string& errorMessage);
    const std::map<std::string, std::function<void(http_request)>>& get_route_map(const method& httpMethod);


    http_listener listener;
    std::map<std::string, std::function<void(http_request)>> get_routes;
    std::map<std::string, std::function<void(http_request)>> post_routes;
    std::map<std::string, std::function<void(http_request)>> delete_routes;
};

#endif /* __SERVERWRAPPER_H__ */