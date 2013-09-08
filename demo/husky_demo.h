#ifndef HUSKY_DEMO_H
#define HUSKY_DEMO_H

#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "../src/headers.h"
using namespace CPPCOMMON;

class ServerDemo: public SRequestHandler
{
	public:
		virtual bool init();
		virtual bool dispose();
	public:
		//int HandleRequest(string& strQuery,string& strOut);
		virtual void operator()(string &strRec, string &strSnd);
};

#endif
