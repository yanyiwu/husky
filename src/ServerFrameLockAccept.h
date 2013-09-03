#ifndef SERVERFRAME_H 
#define SERVERFRAME_H 
#ifdef _WIN32
	#include <WinSock2.h>	
	typedef  CRITICAL_SECTION CS;
#else
	#include <pthread.h>
	#include <string.h>
	#include <errno.h>
	#include <unistd.h>
	typedef unsigned short  u_short;
	typedef unsigned int    u_int;
	typedef	int             SOCKET;
	typedef pthread_mutex_t PM;
	typedef pthread_cond_t  PC;
	#define INVALID_SOCKET  -1 
	#define SOCKET_ERROR    -1 
	#define closesocket     close
#endif

#include <string>
#include <vector>
using std::vector;
using std::string;

#define  RECV_BUFFER     10240
#define  LISEN_QUEUR_LEN 1024

struct SRequestHandler 
{
	virtual void operator()(string &strRec, string &strSnd)
	{
		;
	}

};


class CServerFrame
{
public:
	
	CServerFrame(void);
	bool CreateServer(u_short nPort,u_short nThreadCount,SRequestHandler *pHandler);
	bool CloseServer();
	bool RunServer();
	// virtual  void HandleRequest(const string &strReceive,string& strSend){;}
	
public:
	~CServerFrame(void);

protected:

#ifdef _WIN32
	bool InitSocketLib(void);
#endif

	bool BindToLocalHost(SOCKET &sock,u_short nPort);

#ifdef _WIN32
	static unsigned __stdcall ServerThread(void *lpParameter );
#else
	static void* ServerThread(void *lpParameter );
#endif

	
private:
	u_short  m_nLsnPort;                            // 侦听端口
	u_short  m_nThreadCount;                     // 服务进程数量
	SOCKET   m_lsnSock;                           //侦听SOCKT


	SRequestHandler *m_pHandler;
	static bool m_bShutdown ;                 // Signals client/server threads to die
#ifdef _WIN32
	static CS m_csAccept;              // 接收锁
#else		
	static PM m_pmAccept;
#endif
			
}; 


struct SPara
{
	SOCKET hSock;
	SRequestHandler * pHandler;
};
#endif
