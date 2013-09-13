#ifndef HUSKY_UTIL_DEF_H
#define HUSKY_UTIL_DEF_H
#include <string>
#include <vector>
#include <set>
#include <string>
#include <stdlib.h>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#define INVALID_SOCKET  -1 
#define SOCKET_ERROR    -1 
#define closesocket     close
#define  RECV_BUFFER     10240
#define  LISEN_QUEUR_LEN 1024

namespace Husky
{
    using std::tr1::unordered_map;
    using std::tr1::unordered_set;

    using std::string;
    using std::vector;
    using std::set;
    using std::pair;

    const char* const MASTER_PID_FILE= "masterDaemon.pid.";//daemon master PID file 

    const char* const RESPONSE_CHARSET_UTF8 = "UTF-8";
    const char* const RESPONSE_CHARSET_GB2312 = "GB2312";

    const char* const RESPONSE_FORMAT = "HTTP/1.1 200 OK\nConnection: close\nServer: FrameServer/1.0.0\nContent-Type: text/xml; charset=%s\nContent-Length: %d\n\n";
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
    typedef unsigned short  u_short;
    typedef unsigned int    u_int;
    typedef	int             SOCKET;
    typedef pthread_mutex_t PM;
    typedef pthread_cond_t  PC;

    using std::vector;
    using std::string;
}

#endif
