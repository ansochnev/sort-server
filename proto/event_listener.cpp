#include <unistd.h>
#include <fcntl.h>

#include "event_listener.h"

namespace {
    const int ERROR = -1;    
    const int TIMEOUT_INF = -1;

    const int INITIAL_CAP = 2;
}


struct EventListener::HandlerList
{
    EventHandler readHandler;
    EventHandler writeHandler;
};


struct EventListener::Command
{
    enum class Code {EXIT, ADD_NEW_EVENT};

    Code  code;
    int   fd;
    short events;
};


EventListener::EventListener(std::shared_ptr<IExecutor> ex) 
    : m_executor(ex)
{
    int pipe[2];
    int res = pipe2(&pipe[0], O_DIRECT | O_NONBLOCK);
    if (res == ERROR) {
        throw errno;
    }
    m_pipeCommandReader = pipe[0];
    m_pipeCommandWriter = pipe[1];

    m_events.reserve(INITIAL_CAP);
    addEvent(Event{m_pipeCommandReader, POLLIN, 0});

    if (!m_executor) {
        m_executor = std::make_shared<SingleThreadingExecutor>();
    }
}


EventListener::~EventListener()
{
    close(m_pipeCommandReader);
    close(m_pipeCommandWriter);
}


void EventListener::sendCommand(Command cmd)
{
    int n = write(m_pipeCommandWriter, &cmd, sizeof(cmd));
    if (n == ERROR) {
        throw Exception(errno, "event listener: pipe write error");
    }
    if (std::size_t(n) < sizeof(cmd)) {
        throw Exception(-1, "event listener: written incomplete command");
    }
}


bool EventListener::recvCommand(Command* cmd)
{
    int n = read(m_pipeCommandReader, cmd, sizeof(*cmd));
    if (n == ERROR && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return false;
    }
    if (n == ERROR) {
        throw Exception(errno, "event listener: pipe read error");
    }
    if (std::size_t(n) < sizeof(cmd)) {
        throw Exception(-1, "event listener: read incomplete command");
    }
    return true;
}


void EventListener::addReadEvent(int fd, EventHandler h) {
    m_handlers[fd].readHandler = h;    
    sendCommand(Command{Command::Code::ADD_NEW_EVENT, fd, POLLIN});
}


void EventListener::addWriteEvent(int fd, EventHandler h) {
    m_handlers[fd].writeHandler = h;
    sendCommand(Command{Command::Code::ADD_NEW_EVENT, fd, POLLOUT});
}


void EventListener::addEvent(Event ev)
{
    for (auto& pfd : m_events) {
        if (pfd.fd == ev.fd) {
            pfd.events |= ev.events;
            return;
        }
    }

    struct pollfd new_pfd;
    new_pfd.fd = ev.fd;
    new_pfd.events = ev.events;
    new_pfd.revents = 0;

    for (auto& pfd : m_events) {
        if (pfd.fd == -1) {
            pfd = new_pfd;
            return;
        }
    }
    m_events.push_back(new_pfd);
}


void EventListener::run()
{
    while (true) 
    {
        int ready_n = poll(&m_events[0], m_events.size(), TIMEOUT_INF);

        if (ready_n == ERROR) {
            perror("poll");
            throw errno;
        }
        
        if (m_events[0].revents & POLLIN) 
        {
            Command cmd;
            while(recvCommand(&cmd)) 
            {
                switch (cmd.code) 
                {
                case Command::Code::EXIT:
                    return; 

                case Command::Code::ADD_NEW_EVENT:
                    addEvent(Event{cmd.fd, cmd.events, 0});
                    break;
                }
            }
            m_events[0].revents = 0;
            continue;
        }

        executeHandlers();
    }
}


void EventListener::executeHandlers()
{
    for (auto& pfd : m_events) 
    {
        if (pfd.fd == m_pipeCommandReader) {
            continue;
        }

        auto handlers = m_handlers[pfd.fd];

        if (pfd.revents & (POLLIN | POLLERR)) {
            pfd.events ^= POLLIN;
            if (handlers.readHandler) {
                m_executor->execute(pfd.fd, handlers.readHandler);
            }
        }
        if (pfd.revents & (POLLOUT | POLLHUP | POLLERR)) {
            pfd.events ^= POLLOUT;
            if (handlers.writeHandler) {
                m_executor->execute(pfd.fd, handlers.writeHandler);
            }
        }
        pfd.revents = 0;

        if (pfd.events == 0) {
            pfd.fd = -1;
        }
    }
}


void EventListener::stop() {
    sendCommand(Command{Command::Code::EXIT, 0, 0});
}
