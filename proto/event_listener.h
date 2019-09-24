#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

#include <functional>
#include <map>
#include <vector>
#include <memory>

#include <poll.h>

#include "../exception.h"

using EventHandler = std::function<void (int fd)>;


class IExecutor
{
public:
    virtual ~IExecutor() {}
    virtual void execute(int fd, EventHandler h) = 0;
};


class EventListener
{
    using Event = pollfd;
    struct HandlerList;

    std::map<int, HandlerList>  m_handlers;
    std::vector<Event>          m_events;

    int m_pipeCommandWriter;
    int m_pipeCommandReader;

    std::shared_ptr<IExecutor> m_executor;

public:
    EventListener(std::shared_ptr<IExecutor> ex = nullptr);
    ~EventListener();

    void addReadEvent(int fd, EventHandler h);
    void addWriteEvent(int fd, EventHandler h);

    void run();
    void stop();

private:
    struct Command;
    void sendCommand(Command cmd);
    bool recvCommand(Command* cmd);
    
    void addEvent(Event ev);
    void removeEvent(int fd);

    void executeHandlers();
};


class SingleThreadingExecutor : public IExecutor
{
public:
    SingleThreadingExecutor() = default;
    void execute(int fd, EventHandler h) override { h(fd); }
};

#endif // EVENT_LISTENER_H