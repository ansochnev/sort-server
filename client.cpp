#include <iostream>
#include <string>
#include <cstring>
#include <memory>

#include <unistd.h>

#include "proto/proto.h"
#include "proto/tcp.h"
#include "proto/udp.h"
#include "exception.h"

using namespace proto;


void printUsage() {
    std::cout << "usage: client [-t transport]" << std::endl;
}


enum class Proto {TCP, UDP};


Proto parseProtocol(const std::string s)
{
    if (s == "udp") {
        return Proto::UDP;
    } 
    if (s == "tcp") {
        return Proto::TCP;
    }
    throw std::string("unknown protocol");
}


IClient* createClient(Proto p, const std::string host, short port)
{
    switch (p)
    {
    case Proto::TCP:
        return new tcp::Client(host, port);

    case Proto::UDP:
        return new udp::Client(host, port);

    default:
        throw std::string("not implemented");
    }
}


int main(int argc, char* argv[])
{
    Proto transport = Proto::TCP;
    std::string host = "localhost";
    int port = 8000;

    int opt;
    while((opt = getopt(argc, argv, "t:h")) != -1)
    {
        switch (opt) {
        case 'h':
            printUsage();
            return 0;
        case 't':
            try {
                transport = parseProtocol(optarg);
            }
            catch (std::string& e) {
                std::cerr << e << std::endl;
                return 1;
            }
            break;
        }
    }

    proto::IClient *c = createClient(transport, host, port);
    auto client = std::unique_ptr<IClient>(c);
    
    try 
    {
        const std::size_t size = 1024;
        char buf[size];
        while(std::cin.getline(buf, size))
        {
            if (std::string(buf) == "quit") {
                break;
            }

            Request req;
            req.size = strlen(buf);
            memcpy(req.data, buf, req.size);
            
            Response resp = client->get(req);
            std::cout << resp.numbers << std::endl;
            std::cout << resp.sum << std::endl;
        }
    }
    catch (Exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "error: " << e.errcode() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    if (!std::cin.eof()) {
        std::cout << "input error" << std::endl;
    }

    return 0;
}