#include <iostream>
#include <thread>

#include "handler.h"
#include "proto/udp.h"
#include "proto/tcp.h"
#include "exception.h"

template<typename T>
void runServer(T&& server)
{
    try {
        server.run();
    }    
    catch (Exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "error " << e.errcode() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


int main()
{
    std::string host = "localhost";
    int port = 8000;
    Handler handler;

    tcp::Server tcpServer(host, port, &handler);
    udp::Server udpServer(host, port, &handler);

    std::thread th1([&udpServer]() { runServer(udpServer); });
    std::thread th2([&tcpServer]() { runServer(tcpServer); });

    th1.join();
    th2.join();

    return 0;
}
