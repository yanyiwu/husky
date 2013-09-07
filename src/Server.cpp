#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

bool Server::init()
{
	return true;
}

bool Server::dispose()
{
	return true;
}

int Server::HandleRequest(string& strQuery,string& strOut)
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
		return 0;
	}
	strOut = "<result>EMPTY</result>";
	return 0;
}



#ifdef KEYWORDSERVER_UT

int main()
{
	return 0;
}

#endif
