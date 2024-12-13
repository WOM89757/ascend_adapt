#ifndef __CLIENTWRAPPER_H__
#define __CLIENTWRAPPER_H__

#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;

class ClientWrapper {
public:
    explicit ClientWrapper(const std::string& uri) : client(uri) {}

    pplx::task<http_response> get(const std::string& path) {
        return client.request(methods::GET, path);
    }

    pplx::task<http_response> post(const std::string& path, const json::value& body) {
        return client.request(methods::POST, path, body);
    }

private:
    http_client client;
};

#endif /* __CLIENTWRAPPER_H__ */