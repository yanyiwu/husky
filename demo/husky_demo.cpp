#include <unistd.h>
#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "../husky/headers.h"
using namespace CPPCOMMON;
using namespace Husky;

class ServerDemo: public IRequestHandler
{
	public:
        ServerDemo(){};
        virtual ~ServerDemo(){};
		virtual bool init(){return true;};
		virtual bool dispose(){return true;};
	public:
		//int HandleRequest(string& strQuery,string& strOut);
		virtual void operator()(string &strRec, string &strSnd)
        {
            //CXmlHttp xh;
            const char* end;
            const char* mid;
            strSnd = strRec;
            return;
        }

        virtual bool do_GET(const HttpReqInfo& httpReq, string& strSnd)
        {
            HttpReqInfo info = httpReq;
            strSnd = info.toString();
            return true;
        }
};

int main(int argc,char* argv[])
{
	if(argc<3)
	{
		printf("usage: %s -n THREAD_NUMBER -p  PLISTEN_PORT -k start|STOP\n",argv[0]);
		return -1;
	}
	ServerDemo s;
	CWorker worker(&s);
	CDaemon daemon(&worker);
	return daemon.Run(argc,argv)!=true;
}

