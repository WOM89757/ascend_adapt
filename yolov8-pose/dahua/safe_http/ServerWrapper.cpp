#include "ServerWrapper.h"
ServerWrapper::ServerWrapper(const std::string &uri) : listener(uri)
{
    listener.support(methods::GET, std::bind(&ServerWrapper::handle_get, this, std::placeholders::_1));
    listener.support(methods::POST, std::bind(&ServerWrapper::handle_post, this, std::placeholders::_1));
}

// 添加 GET 方法的路由处理器
void ServerWrapper::add_get_route(const std::string &path, const std::function<void(http_request)> &handler)
{
    get_routes[path] = handler;
}

// 添加 POST 方法的路由处理器
void ServerWrapper::add_post_route(const std::string &path, const std::function<void(http_request)> &handler)
{
    post_routes[path] = handler;
}

void ServerWrapper::start()
{
    listener.open().wait();
    Logger::log("Server started at: " + listener.uri().to_string());
}

void ServerWrapper::stop()
{
    listener.close().wait();
    Logger::log("Server stopped.");
}

void ServerWrapper::handle_get(http_request request)
{
    auto path = request.relative_uri().path();
    if (get_routes.count(path))
    {
        get_routes[path](request);
    }
    else
    {
        request.reply(status_codes::NotFound, "GET Path not found");
    }
}

void ServerWrapper::handle_post(http_request request)
{
    auto path = request.relative_uri().path();
    if (post_routes.count(path))
    {
        post_routes[path](request);
    }
    else
    {
        request.reply(status_codes::NotFound, "POST Path not found");
    }
}
