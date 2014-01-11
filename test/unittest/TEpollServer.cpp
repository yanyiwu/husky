#include "src/EpollServer.hpp"
#include "gtest/gtest.h"

using namespace Husky;
TEST(EpollServerTest, Test1)
{
    EpollServer reqinfo;
    reqinfo.load(header);
    reqinfo["CLIENT_IP"] = "11.1.1.1";
    string s;
    ASSERT_TRUE(reqinfo.find("ACCEPT", s));
}

