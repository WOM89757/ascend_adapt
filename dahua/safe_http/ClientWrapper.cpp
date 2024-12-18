#include "ClientWrapper.h"

// 请求方法
void ClientWrapper::request(
    const method& httpMethod, const std::string& path, const json::value& body,
    void* customInfo, std::string content_type,
    const std::function<void(json::value, void*)>& onSuccess,
    const std::function<void(const std::string&)>& onError)
{
    client.request(httpMethod, path, body.serialize(), content_type)
        .then([onSuccess, onError](http_response response) {
            try
            {
                // 检查状态码
                if (response.status_code() != status_codes::OK)
                {
                    throw std::runtime_error(
                        "Server returned error status: " +
                        std::to_string(response.status_code()));
                }

                // 提取 JSON
                return response.extract_json();
            }
            catch (const std::exception& e)
            {
                if (onError)
                {
                    onError(e.what());
                }
                throw;
            }
        })
        .then([onSuccess, onError, customInfo](json::value jsonResponse) {
            try
            {
                if (onSuccess)
                {
                    onSuccess(jsonResponse, customInfo);
                }
            }
            catch (const std::exception& e)
            {
                if (onError)
                {
                    onError(e.what());
                }
            }
        })
        .wait(); // 等待异步任务完成
}

// GET 请求封装
void ClientWrapper::get(
    const std::string& path, void* customInfo,
    const std::function<void(json::value, void*)>& onSuccess,
    const std::function<void(const std::string&)>& onError)
{
    try
    {
        request(methods::GET, path, json::value::null(), customInfo,
                "text/plain; charset=utf-8", onSuccess, onError);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Client Error: " << e.what() << " at uri: " << uri_  << path
                  << std::endl;
    }
}

void ClientWrapper::post(
    const std::string& path, const json::value& body, void* customInfo,
    const std::function<void(json::value, void*)>& onSuccess,
    const std::function<void(const std::string&)>& onError)
{
    try
    {
        request(methods::POST, path, body, customInfo,
                "application/json; charset=utf-8", onSuccess, onError);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Client Error: " << e.what() << " at uri: " << uri_ << path
                  << " body: " << body.serialize() << std::endl;
    }
}

// 默认成功回调函数
void ClientWrapper::default_on_success(json::value response, void* customInfo)
{
    std::cout << "[Client Default Success] Response: " << response.serialize()
              << std::endl;
}

// 默认错误回调函数
void ClientWrapper::default_on_error(const std::string& error)
{
    std::cerr << "[Clinet Default Error] Error: " << error << std::endl;
}