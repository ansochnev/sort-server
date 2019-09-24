#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../exception.h"
#include "udp.h"
#include "common.h"

using namespace proto;
using namespace udp;


Client::Client(const std::string& host, short port)
    : m_host(host), m_port(port), m_socket(ERROR) {}


Client::~Client() {
    if (m_socket != ERROR) close(m_socket);
}


void Client::connect()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == ERROR) {
        throw Exception(errno, "udp: client: socket");
    }

    sockaddr_in addr = makeAddr(m_host, m_port);
    if (::connect(m_socket, (sockaddr*) &addr, sizeof(addr)) == ERROR) {
        close(m_socket);
        m_socket = ERROR;
        throw Exception(errno, "udp: client: connect");
    };  
}


Response Client::get(const Request& req)
{
    if (req.size > MAX_DATA_SIZE) {
        throw Exception(-1, "too big request data");
    }

    if (!connected()) {
        connect();
    }

    Serializer s;
    s.serialize(req);
    int n = send(m_socket, s.data(), s.size(), 0);
    if (n == ERROR) {
        throw Exception(errno, "udp: client: send");
    }

    // Need timeout here.
    char buf[MAX_DATA_SIZE];
    n = recv(m_socket, buf, MAX_DATA_SIZE, 0);
    if (n == ERROR) {
        throw Exception(errno, "udp: client: send");
    }

    return s.deserializeResponse(std::string(buf, n));
}


Server::Server(const std::string& host, short port, IHandler* h)
    : m_host(host), m_port(port), m_handler(h) {}


void Server::run()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket == ERROR) {
        throw Exception(errno, "udp: server: socket");
    }

    sockaddr_in addr = makeAddr(m_host, m_port);
    if (bind(m_socket, (sockaddr*) &addr, sizeof(addr)) == ERROR) {
        close(m_socket);
        throw Exception(errno, "udp: server: bind");
    }

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    char buf[MAX_MESG_SIZE];
    while (true)
    {
        int n = recvfrom(m_socket, buf, MAX_MESG_SIZE, 0, 
                         (struct sockaddr*) &clientAddr, &addrlen);
        if (n == ERROR) {
            throw Exception(errno, "udp: server: recvfrom");
        }
        if (n == 0) {
            continue;
        }
        
        Serializer s;
        Request req = s.deserializeRequest(std::string(buf, n));
        Response resp = m_handler->handle(req);
        s.serialize(resp);

        n = sendto(m_socket, s.data(), s.size(), 0, 
                   (struct sockaddr*) &clientAddr, addrlen);
        if (n == ERROR) {
            throw Exception(errno, "udp: server: sendto");
        }
    }
}
