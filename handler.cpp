#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string_view>

#include "handler.h"

using namespace proto;


std::vector<std::string> findDigitTokens(std::string_view str)
{
    std::vector<std::string> tokens;

    std::string startsWith = "123456789";
    std::string endsWith   = "0123456789";

    auto begin = str.begin();
    auto end   = str.end();

    decltype(begin) tokenBegin, tokenEnd;
    while (begin != end)
    {
        begin = std::find_first_of(begin, end,
                                        startsWith.begin(), startsWith.end());
        if (begin == end) break;

        tokenBegin = begin;
        tokenEnd = std::find_if(begin, end, [endsWith](auto c) -> bool {
            return endsWith.find(c) == std::string::npos;
        });
        begin = tokenEnd;
        tokens.emplace_back(tokenBegin, tokenEnd);
    }

    return tokens;
}


uint32_t stou32(const std::string& s)
{
    unsigned long n = std::stoul(s);
    if constexpr (sizeof(n) > sizeof(uint32_t)) {
        if (n > uint32_t(-1)) throw std::out_of_range("stou32: overflow");
    } 
    return n; 
}


Response Handler::handle(const Request& req)
{
    auto tokens = findDigitTokens(std::string_view(req.data, req.size));

    std::vector<uint32_t> numbers;
    numbers.reserve(tokens.size());

    try {
        for (auto& t : tokens) {
            numbers.push_back(stou32(t));
        }  
    }
    catch (std::out_of_range& e) {
        return Response{"overflow", "0"};
    }
    
    std::sort(numbers.begin(), numbers.end());

    uint64_t sum = 0;
    for (uint32_t n : numbers) {
        sum += n;
    }

    Response resp;
    resp.sum = std::to_string(sum);

    if (!numbers.empty()) 
    {
        std::ostringstream ss;
        ss << numbers.front();
        for (std::size_t i = 1; i < numbers.size(); ++i) {
            ss << " " << numbers[i];
        }
        resp.numbers = ss.str();
    }

    return resp;
}