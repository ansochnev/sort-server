#include <cassert>
#include <cstring>
#include <sstream>

#include "common.h"

using namespace proto;


void testRequestRW(const std::string& str)
{
    Serializer s;
    Request rs;
    memcpy(rs.data, str.c_str(), str.size());
    rs.size = str.size();
    s.serialize(rs);
    Request rd = s.deserializeRequest(std::string(s.data(), s.size()));

    assert(rs.size == rd.size);
    assert(std::string(rs.data, rs.size) == std::string(rd.data, rd.size));
}

void testResponseRW(const std::string& nums, const std::string& sum)
{
    Serializer s;
    Response rs{nums, sum};
    s.serialize(rs);
    Response rd = s.deserializeResponse(std::string(s.data(), s.size()));

    std::stringstream ssrs, ssrd;
    ssrs << rs.numbers << " " << rs.sum;
    ssrd << rd.numbers << " " << rd.sum;
    assert(rs.numbers == rd.numbers);
    assert(rs.sum == rd.sum);
}


void testSerializer()
{
    testRequestRW("hello, world");
    testRequestRW("1");
    testRequestRW(std::string(MAX_DATA_SIZE, 'a'));

    testResponseRW("123", "sdf");
    testResponseRW("1", "2");
}


int main()
{
    makeAddr("localhost", 8000); // Avoid compiler warning.
    testSerializer();
}