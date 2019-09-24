#ifndef TCP_INTERNAL_H
#define TCP_INTERNAL_H

#include "proto.h"
/*
namespace {
    const int MAX_MESG_SIZE = proto::MAX_DATA_SIZE + 1;
    const int MESG_DELIM = 0;
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
*/
#endif // TCP_INTERNAL_H