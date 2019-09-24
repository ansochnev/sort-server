#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <cassert>
#include <sstream>
#include <cstring>

#include "proto/proto.h"
#include "handler.h"

using namespace proto;

std::vector<std::string> findDigitTokens(std::string_view str);


void testFindDigitTokens()
{
    std::string s = "123middle 1232 text3838dfds45";
    std::vector<std::string> v = {"123", "1232", "3838", "45"};
    assert(findDigitTokens(s) == v);

    s = "1234";
    v = {"1234"};
    assert(findDigitTokens(s) == v);

    s = "01230";
    v = {"1230"};
    assert(findDigitTokens(s) == v);

    s = "-1230";
    v = {"1230"};
    assert(findDigitTokens(s) == v);

    s = "asf";
    v = {};
    assert(findDigitTokens(s) == v);

    s = "";
    v = {};
    assert(findDigitTokens(s) == v);  
}

void testHandler(const std::string& input, const std::string& expect)
{
    Request req;
    memcpy(req.data, input.c_str(), input.size());
    req.size = input.size();

    Handler h;
    Response resp = h.handle(req);

    std::string have = resp.numbers + ": " + resp.sum;
    assert(have == expect);
}


int main()
{
    testFindDigitTokens();

    std::string input = "20 apples, 30 bananas, 15 peaches and 1 watermelon";
    std::string expect = "1 15 20 30: 66";
    testHandler(input, expect);

    return 0;
}