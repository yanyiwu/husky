// XmlHttp.h: interface for the CXmlHttp class.
//
//////////////////////////////////////////////////////////////////////



#ifndef XMLHTTP_H
#define XMLHTTP_H

#include <string>
#include "UtilDef.h"

#define ADD_INT_XMLNODE(xml,chBuf,nodeName,value) {sprintf(chBuf, "<%s>%d</%s>\n", nodeName,value,nodeName);xml += chBuf;}


namespace Husky
{

    using namespace std;

    class CXmlHttp  
    {
        public:

            string& XmlEncode(const char *str, string& s);
            string& XmlPackText(const char *str, string& s)	;
            //int ParseRequest(string strRecvPara,unordered_map<string,string>& hmStrStr);
            // 15=>F 9=>9



            //URLEncode


            string URLEncode(string sIn);
            string URLDecode(const string& in,string& ret);


            CXmlHttp();
            virtual ~CXmlHttp();
        private:

            // b => 0xb

            char char2hex(char ch)

            {
                if(ch>='A' && ch<='Z') 
                  return ch-'A'+10;
                if(ch>='a' && ch<='z')
                  return ch-'a'+10;
                if(ch>='0' && ch<='9')
                  return ch-'0';
                return 0;

            }

            // BA => 0xBA

            inline char hex2char(char c1,char c2)
            {
                return (char2hex(c1)<<4) + char2hex(c2); 
            }
            inline unsigned char toHex(const unsigned char &x)
            {
                return  x > 9 ? x + 55: x + 48;
            }



    };

}
#endif
