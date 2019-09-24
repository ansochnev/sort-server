#ifndef HANDLER_H
#define HANDLER_H

#include "proto/proto.h"

class Handler : public proto::IHandler
{
public:
    Handler() = default;
    virtual proto::Response handle(const proto::Request& r) override;
};

#endif // HANDLER_H