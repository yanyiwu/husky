#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

bool Server::init()
{
	//bool res = true;

	//res = _seg.init();
	//if(!res)
	//{
	//	LogFatal("_seg.init Error");
	//	return false;
	//}

	//res = _kwext.init();
	//if(!res)
	//{
	//	LogFatal("_kwext.init Error");
	//	return false;
	//}

	//res = _kwext.loadSegDict(SEGDICT_FILEPATH);
	//if(!res)
	//{
	//	LogFatal("_kwext.loadSegDict Error");
	//	return false;
	//}
	//res = _kwext.loadStopWords(STOPWORDS_FILEPATH);
	//if(!res)
	//{
	//	LogFatal("_kwext.loadStopWords Error");
	//	return false;
	//}
	return true;
}

bool Server::dispose()
{
	//_kwext.dispose();
	//_seg.dispose();
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
		//string url(hssParse["url"]);
		//string title(upperStr(hssParse["title"]));

		//vector<string> words;
		//vector<string> keywords;
		//
		//_seg.cut(title, words);
		//_kwext.extract(words, keywords, 10);
		//string keywordStr = joinStr(keywords, ",");

		//response
		strOut = "<?xml version=\"1.0\" encoding=\"gbk\" ?><result><keyword><![CDATA[";
		//strOut += keywordStr;
		strOut += "]]></keyword></result>";

		//log
		//LogInfo(string_format("{url:\"%s\", title:\"%s\", result:\"%s\"}", url.c_str(), title.c_str(), keywordStr.c_str()).c_str());
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
