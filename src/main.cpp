#include <algorithm>
#include <string>
#include <ctype.h>
#include <string.h>
#include "ServerFrameLockAccept.h"
#include "FileOperation.h"
#include "UtilDef.h"
#include "XmlHttp.h"
#include "Daemon.h"
#include "Server.h"

struct SDerivedHandler:public SRequestHandler 
{ 
	bool Init()
	{
		return m.init();
	}

	bool Dispose()
	{
		m.dispose();
		return true;
	}
	virtual void operator()(string &strRec, string &strSnd) 
	{
		m.HandleRequest(strRec,strSnd);
	} 
	Server m;
};


class CWorkerEx:public CWorker
{
	public:
		CWorkerEx(SDerivedHandler* pHandler):m_pHandler(pHandler){}
		virtual bool Init(HIS&his)
		{
			int nPort,nThreadNum;
			HISI i=his.find('n');   
			if(i!=his.end())
			  nThreadNum=atoi(i->second.c_str());
			i=his.find('p');   
			if(i!=his.end())
			  nPort=atoi(i->second.c_str());
			if(nThreadNum<=0||nPort<=0)
			  return false;
			if(!m_pHandler->Init())
			  return false;

			return m_server.CreateServer(nPort, nThreadNum, m_pHandler);
		}
		virtual bool Run(){return m_server.RunServer();}
		virtual bool Dispose(){return m_pHandler->Dispose();} 
		virtual bool close(){return m_server.CloseServer();}

	private:
		CServerFrame     m_server;
		SDerivedHandler* m_pHandler;

};

int main(int argc,char* argv[])
{
	if(argc<3)
	{
		printf("usage: %s -n THREAD_NUMBER -p  PLISTEN_PORT -k start|STOP\n",argv[0]);
		return -1;
	}
	SDerivedHandler s;
	CWorkerEx worker(&s);
	CDaemon daemon(&worker);
	return daemon.Run(argc,argv)!=true;
}

