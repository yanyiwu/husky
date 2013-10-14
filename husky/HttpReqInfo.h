#ifndef CPPCOMMON_HTTP_REQINFO_H
#define CPPCOMMON_HTTP_REQINFO_H

#include <iostream>
#include <string>
#include "globals.h"
#include "../cppcommon/headers.h"

namespace Husky
{
    using namespace CPPCOMMON;

    class HttpReqInfo
    {
        public:
            static const char* const KEY_METHOD;
            static const char* const KEY_PATH;
            static const char* const KEY_PROTOCOL;
        public:
            bool load(const string& headerStr);
        private:
            //bool _parse(const string& headerStr, size_t lpos, size_t rpos, const char * const key);
        public:
            string& operator[] (const string& key)
            {
                return _headerMap[key];
            }
            bool find(const string& key, string& res)const
            {
                return _find(_headerMap, key, res);
            }
            bool GET(const string& argKey, string& res)const
            {
                return _find(_methodGetMap, argKey, res);
            }
            bool POST(const string& argKey, string& res)const
            {
                return _find(_methodPostMap, argKey, res);
            }
        private:
            HashMap<string, string> _headerMap;
            HashMap<string, string> _methodGetMap;
            HashMap<string, string> _methodPostMap;
        private:
            bool _find(const HashMap<string, string>& mp, const string& key, string& res) const;
        public:
            string toString() const;// function for debug because of heavy time consuming
        private:
            bool _parseUrl(const string& url, HashMap<string, string>& mp);
    };

}

#endif
