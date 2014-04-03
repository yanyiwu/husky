#include <unistd.h>
#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "src/EpollServer.hpp"
#include "src/Limonp/ArgvContext.hpp"

using namespace Husky;

class ReqHandler: public IRequestHandler
{
	public:
        virtual ~ReqHandler(){};
	public:
        virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd) const
        {
            strSnd << httpReq;
            LogInfo(strSnd);
            return true;
        }
        virtual bool do_POST(const HttpReqInfo& httpReq, string& strSnd) const
        {
            strSnd << httpReq;
            LogInfo(strSnd);
            return true;
        }
};

int main(int argc,char* argv[])
{
    unsigned int port = 11257;//, threadNum = 8;

    ReqHandler reqh;
    EpollServer sf(port, &reqh);
    //HuskyServer sf(port, threadNum, &reqh);
    return !(sf.start());
}

