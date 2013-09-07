#include "husky_demo.h"

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
	CXmlHttp xh;
	const char* end;
	const char* mid;
	end = strstr(strQuery.c_str(), " HTTP");
	mid = strstr(strQuery.c_str(), "/?");
	if (end&&mid)
	{
		strQuery = string(mid+2, end);
	}
	string dstrQuery(strQuery);
	strQuery = "";
	string str = "";
	strQuery = xh.URLDecode(dstrQuery, str);
	HSS   hssParse;
	HSSI  hssi;
	xh.ParseRequest(strQuery,hssParse);

	hssi = hssParse.find("title"); 
	if(hssi != hssParse.end() && hssi->second.length() > 0)
	{
		strOut = "<?xml version=\"1.0\" encoding=\"gbk\" ?><result><keyword><![CDATA[";
		//strOut += keywordStr;
		strOut += "]]></keyword></result>";
		return;
	}
	strOut = "<result>Hello Husky</result>";
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

