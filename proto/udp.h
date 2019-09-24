#ifndef UDP_H
#define UDP_H

#include "proto.h"

namespace udp {
    class Server;
    class Client;
}


class udp::Client : public proto::IClient
{
    std::string m_host;
    short m_port;

    int m_socket;

public:
    Client(const std::string& host, short port);
    ~Client();
    proto::Response get(const proto::Request& req) override;

private:
    void connect();
    bool connected() const noexcept { return m_socket != -1; }
};


class udp::Server
{
    std::string      m_host;
    short            m_port;
    proto::IHandler* m_handler;
    int              m_socket;

public:
    Server(const std::string& host, short port, proto::IHandler* h); 
    void run();
};

#endif // UDP_H