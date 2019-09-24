#include <cstring>
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "../exception.h"
#include "proto.h"

namespace {

const int ERROR = -1;
const int MAX_MESG_SIZE = proto::MAX_DATA_SIZE + 1;
const int MESG_DELIM = 0;

sockaddr_in makeAddr(const std::string& host, short port)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (host == "localhost") {
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    } 
    else if (!inet_aton(host.c_str(), &addr.sin_addr)) {
        throw Exception(-1, "invalid host");
    }
    return addr;
}


class Serializer
{
    char m_buf[MAX_MESG_SIZE];
    int  m_len;

public:
    const char* data() const noexcept { return m_buf; }
    int size() const noexcept { return m_len; }

    void serialize(const proto::Request& req);
    void serialize(const proto::Response& resp);

    proto::Request deserializeRequest(const std::string& message);
    proto::Response deserializeResponse(const std::string& message);
};

void Serializer::serialize(const proto::Request& req) 
{
    memcpy(m_buf, req.data, req.size);
    m_buf[req.size] = MESG_DELIM;
    m_len = req.size + 1;
}


proto::Request Serializer::deserializeRequest(const std::string& message)
{
    proto::Request req;
    req.size = message.size() - 1;
    memcpy(req.data, message.c_str(), req.size);
    return req;
}


void Serializer::serialize(const proto::Response& resp) 
{
    int numlen = resp.numbers.size();
    memcpy(m_buf, resp.numbers.c_str(), numlen);
    m_buf[numlen] = '\n';

    int sumlen = resp.sum.size();
    memcpy(&m_buf[numlen + 1], resp.sum.c_str(), sumlen);
    m_buf[numlen + sumlen + 1] = '\n';

    m_buf[numlen + sumlen + 2] = MESG_DELIM;
    m_len = numlen + sumlen + 3;
}

proto::Response Serializer::deserializeResponse(const std::string& message)
{
    auto numlen = message.find('\n');
    auto sumlen = message.find('\n', numlen + 1) - numlen - 1;
    
    proto::Response resp;
    resp.numbers = message.substr(0, numlen);
    resp.sum     = message.substr(numlen + 1, sumlen);
    return resp;
}

} // namespace