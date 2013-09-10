#ifndef UTIL_DEF_H
#define UTIL_DEF_H
#include <string>
#include <vector>
#include <set>
#include <string>
#include <stdlib.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
namespace Husky
{
    using std::tr1::unordered_map;
    using std::tr1::unordered_set;

    using std::string;
    using std::vector;
    using std::set;
    using std::pair;


    const char* const RESPONSE_CHARSET_UTF8 = "UTF-8";
    const char* const RESPONSE_CHARSET_GB2312 = "GB2312";

    //-----------------------TYPE_DEFINE--------------------------------//
    typedef vector<string>         VS;
    typedef vector<int>            VI;
    typedef set<int>               SI;
    typedef set<string>            SETS;
    typedef VS::iterator           VSI;
    typedef VI::iterator           VII;
    typedef unordered_map<int,int>      HII;
    typedef unordered_map<string,int>        HSI;
    typedef unordered_map<int,string>        HIS;
    typedef unordered_map<string,string>          HSS;
    typedef pair<int,int>          PII;
    typedef pair<int,string>       PIS;
    typedef pair<string,string>    PSS;
    typedef pair<string,int>       PSI;
    typedef HII::iterator          HIII;
    typedef HSI::iterator          HSII;
    typedef HIS::iterator          HISI;
    typedef HSS::iterator          HSSI;
}

#endif
