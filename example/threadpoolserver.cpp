#include "ThreadPoolServer.hpp"

using namespace husky;

class ReqHandler: public IRequestHandler {
 public:
  virtual ~ReqHandler() {
  }
 public:
  virtual bool DoGET(const HttpReqInfo& httpReq, string& response) {
    const unordered_map<string, string>& mp = httpReq.GetMethodGetMap();
    string mpStr;
    mpStr << mp;
    response = string_format("{method:GET, arguments:%s}", mpStr.c_str());
    return true;
  }
  virtual bool DoPOST(const HttpReqInfo& httpReq, string& response) {
    response = string_format("{body:%s}", httpReq.GetBody().c_str());
    return true;
  }
};

int main(int argc, char** argv) {
  if(argc < 3) {
    printf("usage: %s --port 11257 \n", argv[0]);
    return EXIT_FAILURE;
  }
  size_t threadNumber = 4;
  size_t queueMaxSize = 256;
  int port = atoi(argv[2]);
  ReqHandler reqHandler;
  ThreadPoolServer server(threadNumber, queueMaxSize, port, reqHandler);
  server.Start();
  return EXIT_SUCCESS;
}

