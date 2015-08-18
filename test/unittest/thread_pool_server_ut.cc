#include "thread_pool_server.h"
#include "gtest/gtest.h"

using namespace husky;
class ReqHandler: public IRequestHandler {
 public:
  virtual ~ReqHandler() {
  }

  bool DoGET(const HttpReqInfo& httpReq, string& res) {
    res << httpReq;
    return true;
  }
  bool DoPOST(const HttpReqInfo& httpReq, string& res) {
    res << httpReq;
    return true;
  }
};


TEST(ThreadPoolServerTest, Test1) {
  ReqHandler handler;
  ThreadPoolServer server(4, 256, 11257, handler);
  ASSERT_TRUE(server.Start());
}

