#ifndef __CLIENTWRAPPER_H__
#define __CLIENTWRAPPER_H__

#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;

class ClientWrapper
{
public:
    explicit ClientWrapper(const std::string& uri) : client(uri) {}

    // 请求方法
    void request(
        const method& httpMethod, const std::string& path,
        const json::value& body = json::value::null(),
        void* customInfo = nullptr,
        std::string content_type = "text/plain; charset=utf-8",
        const std::function<void(json::value, void*)>& onSuccess = default_on_success,
        const std::function<void(const std::string&)>& onError = default_on_error);

    // GET 请求封装
    void get(
        const std::string& path,
        void* customInfo = nullptr,
        const std::function<void(json::value, void*)>& onSuccess = default_on_success,
        const std::function<void(const std::string&)>& onError = default_on_error);

    void post(
        const std::string& path, const json::value& body,
        void* customInfo = nullptr,
        const std::function<void(json::value, void*)>& onSuccess = default_on_success,
        const std::function<void(const std::string&)>& onError = default_on_error);

    // 默认成功回调函数
    static void default_on_success(json::value response, void* customInfo = nullptr);
    // 默认错误回调函数
    static void default_on_error(const std::string& error);

private:
    http_client client;
};

#endif /* __CLIENTWRAPPER_H__ */