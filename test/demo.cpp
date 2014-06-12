#include "EpollServer.hpp"

using namespace Husky;

class ReqHandler: public IRequestHandler
{
	public:
        virtual ~ReqHandler(){};
	public:
        virtual bool do_GET(const HttpReqInfo& httpReq, string& response) const
        {
            const unordered_map<string, string>& mp = httpReq.getMethodGetMap();
            string mpStr;
            mpStr << mp;
            string_format(response, "{method:GET, arguments:%s}", mpStr.c_str());
            return true;
        }
        virtual bool do_POST(const HttpReqInfo& httpReq, string& response) const
        {
            string_format(response, "{body:%s}", httpReq.getBody().c_str());
            return true;
        }
};

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: %s --port 11257 \n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[2]);
    ReqHandler reqHandler;
    EpollServer server(port, reqHandler);
    server.start();
    return EXIT_SUCCESS;
}

