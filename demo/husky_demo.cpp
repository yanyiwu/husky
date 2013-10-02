#include "husky_demo.h"
#include <unistd.h>

bool ServerDemo::init()
{
	return true;
}

bool ServerDemo::dispose()
{
	return false;
}

void ServerDemo::operator()(string &strQuery, string &strOut) 
{
	//CXmlHttp xh;
	const char* end;
	const char* mid;
    strOut = strQuery;
    return;
} 

bool ServerDemo::do_GET(const HttpReqInfo& httpReq, string& strSnd)
{
    HttpReqInfo info = httpReq;
    strSnd = info.toString();
    return true;
}

int main(int argc,char* argv[])
{
	if(argc<3)
	{
		printf("usage: %s -n THREAD_NUMBER -p  PLISTEN_PORT -k start|STOP\n",argv[0]);
		return -1;
	}
	ServerDemo s;
	CWorkerEx worker(&s);
	CDaemon daemon(&worker);
	return daemon.Run(argc,argv)!=true;
}

