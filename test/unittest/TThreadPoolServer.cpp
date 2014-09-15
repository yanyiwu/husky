#include "ThreadPoolServer.hpp"
#include "gtest/gtest.h"

using namespace Husky;
class ReqHandler: public IRequestHandler
{
    public:
        virtual ~ReqHandler(){}
    public:
        bool do_GET(const HttpReqInfo& httpReq, string& res) const
        {
            res << httpReq;
            return true;
        }
        bool do_POST(const HttpReqInfo& httpReq, string& res) const
        {
            res << httpReq;
            return true;
        }
};


TEST(ThreadPoolServerTest, Test1)
{
    ReqHandler handler;
    ThreadPoolServer server(4, 256, 11257, handler);
    ASSERT_TRUE(server.start());
}

