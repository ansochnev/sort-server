#ifndef TCP_H
#define TCP_H

#include "proto.h"
#include "event_listener.h"

namespace tcp {
    class Client;
    class Server;
}


class tcp::Client : public proto::IClient
{
    std::string m_host;
    short       m_port;
    int         m_socket;

public:
    Client(const std::string& addr, short port);
    ~Client();
    proto::Response get(const proto::Request& req) override;

private:
    void connect();
    std::string receiveMessage();
};


class tcp::Server
{
    std::string      m_host;
    short            m_port;
    proto::IHandler* m_handler;

    EventListener m_eventListener;
    int           m_listener;

public:
    Server(const std::string& host, short port, proto::IHandler* h); 
    ~Server();   
    void run();

private:
    void do_accept();
};


#endif // TCP_H

