#include "ServerWrapper.h"
ServerWrapper::ServerWrapper(const std::string &uri) : listener(uri)
{
    listener.support(methods::GET, std::bind(&ServerWrapper::handle_request, this, std::placeholders::_1, methods::GET));
    listener.support(methods::POST, std::bind(&ServerWrapper::handle_request, this, std::placeholders::_1, methods::POST));
    listener.support(methods::DEL, std::bind(&ServerWrapper::handle_request, this, std::placeholders::_1, methods::DEL));
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

void ServerWrapper::add_delete_route(const std::string &path, const std::function<void(http_request)> &handler)
{
    delete_routes[path] = handler;
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

void ServerWrapper::default_not_found_handler(http_request request) {
    json::value response;
    response["error"] = json::value::string("Path not found");
    request.reply(status_codes::NotFound, response);
}

void ServerWrapper::handle_request(http_request request, const method& httpMethod) {
    try {
        const auto path = request.relative_uri().path();
        const auto& route_map = get_route_map(httpMethod);

        if (route_map.count(path)) {
            route_map.at(path)(request);
        } else {
            // 动态路由匹配逻辑
            if (httpMethod == methods::DEL)
            {
                auto segments = uri::split_path(path);
                std::string realPath = path.substr(0, path.find_last_of("/"));
                if (route_map.count(realPath))
                {
                    route_map.at(realPath)(request);
                }
            }
            else
            {
                default_not_found_handler(request);
            }
        }
    } catch (const std::exception& e) {
        handle_exception(request, e.what());
    } catch (...) {
        handle_exception(request, "Unknown error occurred");
    }
}

void ServerWrapper::handle_exception(http_request request, const std::string& errorMessage) {
    std::cerr << "[Server Error] " << errorMessage << " request: " << request.body() << std::endl;
    json::value response;
    response["error"] = json::value::string(errorMessage);
    request.reply(status_codes::InternalError, response);
}

const std::map<std::string, std::function<void(http_request)>>& ServerWrapper::get_route_map(const method& httpMethod) {
    if (httpMethod == methods::GET) {
        return get_routes;
    } else if (httpMethod == methods::POST) {
        return post_routes;
    } else if (httpMethod == methods::DEL) {
        return delete_routes;
    } else {
        throw std::invalid_argument("Unsupported HTTP method");
    }
}
