#ifndef USER_PROTO_H
#define USER_PROTO_H

#include <string>

namespace proto 
{
    const int MAX_DATA_SIZE    = 1024;

    struct Request;
    struct Response;
    class IClient;
    class IHandler;
}

struct proto::Request
{
    char data[MAX_DATA_SIZE];
    std::size_t size;
};

struct proto::Response
{
    std::string numbers;
    std::string sum;
};


class proto::IClient
{
public:
    virtual ~IClient() {}
    virtual Response get(const Request& r) = 0;
};


class proto::IHandler
{
public:
    virtual ~IHandler() {} 
    virtual Response handle(const Request& r) = 0;
};

#endif // USER_PROTO_H