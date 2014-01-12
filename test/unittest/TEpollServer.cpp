#include "src/EpollServer.hpp"
#include "gtest/gtest.h"

using namespace Husky;
TEST(EpollServerTest, Test1)
{
    EpollServer server(22222);
    ASSERT_TRUE(server);
}

