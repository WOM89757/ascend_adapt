#ifndef __COMMONDATATYPE_H__
#define __COMMONDATATYPE_H__

#include <string>

struct ServiceNode
{
    std::string host_;
    std::string port_;
    std::string serviceName_;

    ServiceNode() {};
    ServiceNode(std::string host, std::string port, std::string serviceName)
        : host_(host), port_(port), serviceName_(serviceName) {};

    std::string getUri() { return (host_ + port_); }
};

struct CustomInfo
{
    std::string name;
};

struct AlgInfo
{
    std::string algorithmCode;
    std::string version;
};

#endif /* __COMMONDATATYPE_H__ */