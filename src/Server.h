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
//#include "globals.h"
//#include "cppjieba/KeyWordExt.h"
//#include "include/segdef.h"
//#include "OrthmSegment.h"

	//using namespace CppJieba;
	class Server
	{
		private:
			//KeyWordExt _kwext;
			//OrthmSegment _seg;
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
