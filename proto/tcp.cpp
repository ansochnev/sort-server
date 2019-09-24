#include <cstring>
#include <algorithm>
#include <memory>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../exception.h"
#include "tcp.h"
#include "common.h"

using namespace proto;
using namespace tcp;


Client::Client(const std::string& host, short port)
    : m_host(host), m_port(port), m_socket(ERROR) {}


Client::~Client() {
    if (m_socket != ERROR) close(m_socket);
}


void Client::connect()
{
    sockaddr_in addr = makeAddr(m_host, m_port);
    
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == ERROR) {
        throw Exception(errno, "tcp: client: socket");
    }

    if (::connect(m_socket, (sockaddr*) &addr, sizeof(addr)) == ERROR) {
        close(m_socket);
        m_socket = ERROR;
        throw Exception(errno, "tcp: client: connect");
    }  
}


std::string Client::receiveMessage()
{
    std::string message;
    std::size_t endmsg = std::string::npos;

    char buf[MAX_MESG_SIZE];
    while (endmsg == std::string::npos)
    {
        int n = recv(m_socket, buf, MAX_MESG_SIZE, 0);
        if (n == ERROR) {
            throw Exception(errno, "tcp: client: recv");
        }
        if (n == 0) {
            shutdown(m_socket, SHUT_RDWR);
            throw Exception(errno, "tcp: client: recv");
        }

        message.append(buf, n);
        if (message.size() > MAX_MESG_SIZE) {
            throw Exception(errno, "tcp: client: too long message");
        }
        endmsg = message.find(MESG_DELIM) + 1;
    }

    return message.substr(0, endmsg);
}


Response Client::get(const Request& req)
{
    if (m_socket == ERROR) {
        this->connect();
    }

    Serializer s;
    s.serialize(req);

    try 
    {
        int n = send(m_socket, s.data(), s.size(), 0);
        if (n == ERROR) {
            throw Exception(errno, "tcp: client: send");
        }
        
        std::string message = receiveMessage();
        return s.deserializeResponse(message);
    }
    catch (Exception&) 
    {
        close(m_socket);
        m_socket = ERROR;
        throw;
    }
}


class Session : public std::enable_shared_from_this<Session>
{
    IHandler* m_handler;

    EventListener *m_eventListener;
    int  m_sock;

    std::string m_message;

public:
    Session(IHandler* h, EventListener* evl, int sock) 
        : m_handler(h), m_eventListener(evl), m_sock(sock) {}

    ~Session() {
        if (m_sock != ERROR) close(m_sock);
    }
    void start() { do_read(); }

private:
    void handle(std::string&& message);
    void do_read();
    void do_write(const char* buf, std::size_t size);
};


void Session::handle(std::string&& message)
{
    Serializer s;
    Request req = s.deserializeRequest(message);
    Response resp = m_handler->handle(req);
    s.serialize(resp);

    do_write(s.data(), s.size());   
}


void Session::do_read()
{
    auto self(shared_from_this());
    m_eventListener->addReadEvent(m_sock, [self, this](int) 
    {
        char buf[MAX_MESG_SIZE];

        int n = recv(m_sock, buf, MAX_MESG_SIZE, 0);
        if (n == ERROR) {
            perror("tcp: session: recv:");
            return;
        }
        if (n == 0) {
            shutdown(m_sock, SHUT_RDWR);
            return;
        }

        m_message.append(buf, n);

        if (m_message.size() > MAX_MESG_SIZE) {
            m_message.clear();
            std::string msg = "error: message too long";
            do_write(msg.c_str(), msg.size());
        }

        auto endmsg = m_message.find(MESG_DELIM) + 1;
        if (endmsg == std::string::npos) {
            do_read();
        }

        std::string message = m_message.substr(0, endmsg);
        m_message = m_message.substr(endmsg);

        handle(std::move(message));
    });
}


void Session::do_write(const char* buf, std::size_t size)
{
    auto self(shared_from_this());
    m_eventListener->addWriteEvent(m_sock, [self, this, buf, size](int)
    {
        int n = send(m_sock, buf, size, 0);
        if (n == ERROR) {
            perror("send");
            close(m_sock);
            return;
        }
        if (n == 0) {
            close(m_sock);
            return;
        }
        do_read();
    });
}


Server::Server(const std::string& host, short port, proto::IHandler* h)
    : m_host(host), m_port(port), m_handler(h) {}


Server::~Server() {
    if (m_listener != ERROR) close(m_listener);
}


void Server::run()
{
    sockaddr_in addr = makeAddr(m_host, m_port);

    m_listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (m_listener == ERROR) {
        throw Exception(errno, "tcp: server: socket");
    }

    if (bind(m_listener, (sockaddr*) &addr, sizeof(addr)) == ERROR) {
        close(m_listener);
        m_listener = ERROR;
        throw Exception(errno, "tcp: server: bind");
    }

    int backlog = 16;
    if (listen(m_listener, backlog) == ERROR) {
        close(m_listener);
        m_listener = ERROR;
        throw Exception(errno, "server: listen");
    } 

    do_accept();
    m_eventListener.run();
}


void Server::do_accept()
{
    m_eventListener.addReadEvent(m_listener, [this](int) 
    {
        int sock = accept4(m_listener, nullptr, nullptr, SOCK_NONBLOCK);
        if (sock == ERROR) {
            perror("tcp: server: accept");
        } else {
            std::make_shared<Session>(
                m_handler, &m_eventListener, sock)->start();
        }
        do_accept();
    });
}
