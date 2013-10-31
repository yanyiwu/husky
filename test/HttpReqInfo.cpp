
#ifdef UT

using namespace Limonp;
int main()
{
    //string url("http://127.0.0.1/?k1=v1&k2=v2 hh");
    //HashMap<string, string> mp;
    //parseUrl(url, mp);
    //cout<<HashMapToString(mp)<<endl;
    const char * header = "GET /?hehek1=v1&htt HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
    HttpReqInfo reqinfo;
    reqinfo.load(header);
    reqinfo["CLIENT_IP"] = "11.1.1.1";
    string s;
    //cout<<reqinfo.GET("%20k")
    cout<<reqinfo.toString()<<endl;
    return 0;
}

#endif
