#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception : public std::exception
{
    int m_err;
    std::string m_message;
public:
    Exception(int err, std::string&& message) 
        : m_err(err), m_message(message) {}

    int errcode() const noexcept {
        return m_err;
    }

    const char* what() const noexcept override {
        return m_message.c_str();
    }
};

#endif // EXCEPTION_H