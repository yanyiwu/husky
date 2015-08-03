#include "HttpReqInfo.hpp"
#include "gtest/gtest.h"

using namespace Husky;
TEST(HttpReqInfoTest, Test1) {
  //string url("http://127.0.0.1/?k1=v1&k2=v2 hh");
  //HashMap<string, string> mp;
  //parseUrl(url, mp);
  //cout<<HashMapToString(mp)<<endl;
  const char * header = "GET /?hehek1=v1&htt HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  reqinfo.set("CLIENT_IP", "11.1.1.1");
  string s;
  ASSERT_TRUE(reqinfo.find("ACCEPT", s));
  ASSERT_EQ("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8", s);
  ASSERT_EQ(reqinfo.getPath(), "/");
}

TEST(HttpReqInfoTest, Test2) {
  const char * header = "POST / HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nContent-Length: 12\r\n\r\nhello world.";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  reqinfo.set("CLIENT_IP", "11.1.1.1");
  string s;
  ASSERT_TRUE(reqinfo.find("ACCEPT", s));
  ASSERT_EQ(reqinfo.getBody(), "hello world.");
  ASSERT_TRUE(reqinfo.isPOST());
  ASSERT_EQ(reqinfo.getPath(), "/");
}

TEST(HttpReqInfoTest, Test3) {
  const char * header = "POST /123 123 HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\nhello world.\r\n";
  HttpReqInfo reqinfo;
  ASSERT_FALSE(reqinfo.parseHeader(header, strlen(header)));
}

TEST(HttpReqInfoTest, Chinese) {
  const char * header = "GET /?wd=%E4%BD%A0%E5%A5%BD HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  string s;
  ASSERT_TRUE(reqinfo.GET("wd", s));
  ASSERT_EQ("你好", s);
}

TEST(HttpReqInfoTest, GETint) {
  const char * header = "GET /?number=1 HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  int num;
  ASSERT_TRUE(reqinfo.GET("number", num));
  ASSERT_EQ(num, 1);
}

TEST(HttpReqInfoTest, GETsize_t1) {
  const char * header = "GET /?number=1 HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  size_t num;
  ASSERT_TRUE(reqinfo.GET("number", num));
  ASSERT_EQ(num, 1u);
}
TEST(HttpReqInfoTest, GETsize_t2) {
  const char * header = "GET /?number=-1 HTTP/1.1\r\nHost: 10.109.245.13:11256\r\nConnection: keep-alive\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.66 Safari/537.36\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\n\r\n";
  HttpReqInfo reqinfo;
  ASSERT_TRUE(reqinfo.parseHeader(header, strlen(header)));
  size_t num;
  ASSERT_FALSE(reqinfo.GET("number", num));
}
