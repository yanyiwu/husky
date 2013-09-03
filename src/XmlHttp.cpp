// XmlHttp.cpp: implementation of the CXmlHttp class.
//
//////////////////////////////////////////////////////////////////////


#include "XmlHttp.h"
#include "Character.h"


//extern const char*g_ppIndexSource[FILE_CNT];
//extern const char*g_ppIndexKind[FILE_CNT];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static CharacterConverter gs_cc;

CXmlHttp::CXmlHttp()
{

}

CXmlHttp::~CXmlHttp()
{

}

//将URL_DECODE 后参数值原封不动的存入HASH_MAP中
int CXmlHttp::ParseRequest(string strRecvPara,hash_map<string,string>& hmStrStr)
{
	if(strRecvPara.length()==0)
		return 0;
	//strRecvPara=URLDecode(strRecvPara.c_str());

	string::iterator itStr;
	string::iterator itStrT;
	string m_strEncodedKeyWord;
	string strName,strValue;
	// Parses & decodes the string in pairs name-value.
	// ONLY standard requests are handled properly.
	

	for (itStr=strRecvPara.begin();itStr!=strRecvPara.end();++itStr)
	{
		itStrT=itStr;
		while(itStr!=strRecvPara.end()&&*itStr!='=')
		{
			++itStr;
		}
		

		if (itStr==strRecvPara.end())
		{
			break;			
		}
		else			
		{
			strName.assign(itStrT,itStr);
			if (strValue.empty())
			{
				strValue="";
			}
			//cout<<endl<<"strName"<<strName.c_str()<<endl;
			itStrT=itStr;
		}
		
		while (itStr!=strRecvPara.end()&&*itStr!='&')
		{
			++itStr;
		}
		
		strValue.assign(++itStrT,itStr);
		if (strValue.empty())
		{
			strValue="";
		}
		//cout<<endl<<"strValue"<<strValue.c_str()<<endl;
		
		string ret;
		URLDecode(strValue,ret);
		gs_cc.Convert_t2s((char*)ret.c_str());
		gs_cc.ConvertDigitAlpha((char*)ret.c_str());
		ret=ret.c_str();

		hmStrStr[strName]=ret;	
		if (itStr==strRecvPara.end())
		{
			break;
		}
	}
	return 0;

}
string& CXmlHttp::XmlEncode(const char *str, string& s)
{
	if(!str) return s;
	while(*str)
	{
		switch(*str)
		{
		case '<':
			s+="&lt;";
			break;
		case '>':
			s+="&gt;";
			break;
		case '&':
			s+="&amp;";
			break;
		case '\'':
			s+="&apos;";
			break;
		case '\"':
			s+="&quot;";
			break;
		default:
			if(*str>=0)
			{
				if((*str>=0x00&&*str<=0x08)||(*str==0x0c)||(*str==0x0b)||(*str>=0x0e&&*str<=0x1f))
					;
				else
					s+=*str;
			}
			else
			{
				if(*(str+1)==0)
			       return s;;
				s+= *str++;				
				s+=*str;
			}	
		}
		str++;
	}
	return s;
}

string& CXmlHttp::XmlPackText(const char *str, string& s)
{
	if(!str) return s;
	s+="<![CDATA[ ";
	while(*str)
	{
		if(*str>=0)
		{
			if((*str>=0x00&&*str<=0x08)||(*str==0x0c)||(*str==0x0b)||(*str>=0x0e&&*str<=0x1f))
				;
			else
				s+=*str;
		}
		else
		{
			if(*(str+1)==0)
				break;
			s+= *str++;				
			s+=*str;
		}	
		str++;
	}
	s+=" ]]>";
	return s;
}






// Translates the urlencoded string.


// Translates a two-char hex representation in his corresponding value.



string CXmlHttp::URLEncode(string sIn)

{

	string sOut;

	const int nLen = sIn.length();

	const char* p=sIn.c_str();



	unsigned char ch;

	for(int i=0;i<nLen;i++)

	{

		ch=sIn[i];

		if(isalnum(ch))

			sOut+=ch;

		else if(isspace(ch))

			sOut+="%20";

		else{

			sOut+="%";

			sOut+=toHex(ch>>4);

			sOut+=toHex(ch%16);

		}

	}

	return sOut;

}




string CXmlHttp::URLDecode(const string& in,string& ret)

{

	
	string hex;

	string::const_iterator it = in.begin();

	while(it != in.end())
	{

		switch(*it) 
		{

		case '+':

			ret.push_back(' ');

			++it;

			break;

		case '%':

			hex = "";

			++it;

			if(it != in.end()) 
			{

				hex.push_back(*it);

				++it;

				if(it != in.end()) 
				{

					hex.push_back(*it);

					long r = strtol(hex.c_str(), NULL, 16);

					if((0 < r) && (r < 256))

						ret.push_back(r);

					++it;

				}

			}

			break;

		default:

			ret.push_back(*it);

			++it;

		}

	}

	return ret;

}
