#ifndef SEVKEYWORD_KEYWORDSERVER
#define SEVKEYWORD_KEYWORDSERVER

#include "ServerFrameLockAccept.h"
#include "FileOperation.h"
#include "UtilDef.h"
#include "XmlHttp.h"
#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>

class Server
{
	public:
		Server();
		~Server();
	public:
		bool init();
		bool dispose();

	public:
		int HandleRequest(string& strQuery,string& strOut);
};

#endif
