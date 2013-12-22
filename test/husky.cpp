#include <unistd.h>
#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "src/ServerFrame.hpp"
#include "src/Limonp/ArgvContext.hpp"

using namespace Husky;

class ReqHandle: public IRequestHandler
{
	public:
        ReqHandle(){};
        virtual ~ReqHandle(){};
		virtual bool init(){return true;};
		virtual bool dispose(){return true;};
	public:
        virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd)
        {
            strSnd << httpReq;
            LogInfo(strSnd);
            return true;
        }
};

int main(int argc,char* argv[])
{
	//if(argc < 5)
	//{
	//	printf("usage: %s -n THREAD_NUMBER -p LISTEN_PORT \n",argv[0]);
	//	return -1;
	//}
    //ArgvContext arg(argc, argv);
    unsigned int port = 11257, threadNum = 8;
    //port = atoi(arg["-p"].c_str());
    //threadNum = atoi(arg["-n"].c_str());
    

    ReqHandle reqh;
    ServerFrame sf(port, threadNum, &reqh);
    return !(sf.init() && sf.run());
}

