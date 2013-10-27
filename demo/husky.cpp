#include <unistd.h>
#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "../husky/headers.h"
#include <ArgvContext.hpp>

using namespace Husky;

class ServerDemo: public IRequestHandler
{
	public:
        ServerDemo(){};
        virtual ~ServerDemo(){};
		virtual bool init(){return true;};
		virtual bool dispose(){return true;};
	public:

        virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd)
        {
            HttpReqInfo info = httpReq;
            strSnd = info.toString();
            return true;
        }
};

int main(int argc,char* argv[])
{
	if(argc != 7)
	{
		printf("usage: %s -n THREAD_NUMBER -p LISTEN_PORT -k start|stop\n",argv[0]);
		return -1;
	}
    ArgvContext arg(argc, argv);
    unsigned int port = 0, threadNum = 0;
    port = atoi(arg["-p"].c_str());
    threadNum = atoi(arg["-n"].c_str());

    ServerDemo s;
    Worker worker(&s);
    Daemon daemon(&worker);
    if(arg["-k"] == "start")
    {
        return !daemon.Start(port, threadNum);
    }
    else
    {
        return !daemon.Stop();
    }
}

