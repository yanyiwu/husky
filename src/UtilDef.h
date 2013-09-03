#ifndef UTIL_DEF_H
#define UTIL_DEF_H
#include <string>
#include <vector>
#include <set>
#include "hash_wrap.h"

using std::string;
using std::vector;
using std::set;
using std::pair;

//-----------------------TYPE_DEFINE--------------------------------//
typedef string   S;
typedef vector<string>         VS;
typedef vector<int>            VI;
typedef set<int>               SI;
typedef set<string>            SETS;
typedef VS::iterator           VSI;
typedef VI::iterator           VII;
typedef hash_map<int,int>      HII;
typedef hash_map<S,int>        HSI;
typedef hash_map<int,S>        HIS;
typedef hash_map<S,S>          HSS;
typedef pair<int,int>          PII;
typedef pair<int,string>       PIS;
typedef pair<string,string>    PSS;
typedef pair<string,int>       PSI;
typedef HII::iterator          HIII;
typedef HSI::iterator          HSII;
typedef HIS::iterator          HISI;
typedef HSS::iterator          HSSI;

//--------------------MACRO_DEFINE----------------------------------//
#define FOR_EACH(iter,container)          for(iter=container.begin();iter!=container.end();++iter)
#define FOR_EACH_POS(iter,container)      for(iter=0;iter!=container.size();++iter)
#define FOR_EACH_CLASSIC(iter,start,end)  for(iter=start;iter<end;++iter)


//#define FALSE_RETURN(clause)              if(clause) return false;
#define TRUE_RETURN(clause)               if(clause) return true;
#define FALSE_RETURN_STRERROR(clause)     if(clause) {fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));return false;}
#define FALSE_RETURN(clause)     if(clause) {fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));return false;}

#define MINUS1_RETURN(clause)     if(clause) {fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));return -1;}



#endif
