#include "ServerFrameLockAccept.h"
#include "SimpleThread.h"
#include "UtilDef.h"
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
#endif
#include <stdlib.h>
//#include "ErroLog.h"
using namespace simpleThread;

bool CServerFrame::m_bShutdown = false;



#ifdef _WIN32
	CS CServerFrame::m_csAccept; 
#else
	PM CServerFrame::m_pmAccept; 
#endif


	

CServerFrame::CServerFrame(void)
{

}

CServerFrame::~CServerFrame(void)
{
#ifdef _WIN32
	WSACleanup();
	DeleteCriticalSection(&m_csAccept);
#else
	pthread_mutex_destroy(&m_pmAccept);
#endif

}

bool CServerFrame::CloseServer()
{
	m_bShutdown=true;
#ifdef _WIN32
	if (SOCKET_ERROR==closesocket(m_lsnSock))
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}
	char buf[128];
	sprintf(buf,"telnet 127.0.0.1 %d",m_nLsnPort);
	system(buf);
#else
	if (SOCKET_ERROR==closesocket(m_lsnSock))
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}


	int sockfd;
	struct sockaddr_in dest;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}

	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(m_nLsnPort);
	if (inet_aton("127.0.0.1", (struct in_addr *) &dest.sin_addr.s_addr) == 0)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}

	if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) < 0)
	{
		fprintf(stdout,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
	}
	close(sockfd);
#endif
	return true;

}

bool CServerFrame::RunServer()
{
	if(SOCKET_ERROR==listen(m_lsnSock,LISEN_QUEUR_LEN))
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}	
	threadManager thrMngr;	
	int i;
	SPara para;
	para.hSock=m_lsnSock;
	para.pHandler=m_pHandler;
	for (i=0;i<m_nThreadCount;i++)
	{		
#ifdef _WIN32
		if (!thrMngr.CreateThread(ServerThread, &para))
#else
		if (0!=thrMngr.CreateThread(ServerThread, &para))
#endif
		{
			break;	
		}		
	}	
	printf("expect thread count %d, real count %d\n",m_nThreadCount,i);
	if(i==0)	
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}

	printf("server start to run.........\n");

#ifdef _WIN32
	if (thrMngr.WaitMultipleThread()!=WAIT_OBJECT_0)
#else
	if (thrMngr.WaitMultipleThread()!=0)
#endif
	{
		return false;//等待所有线程退出
	}
	printf("server shutdown ok............\n");
	return true;

}

#ifdef _WIN32
unsigned __stdcall CServerFrame::ServerThread(void *lpParameter )
#else
void* CServerFrame::ServerThread(void *lpParameter )
#endif
{
	SPara *pPara=(SPara*)lpParameter;
	SOCKET hSockLsn=pPara->hSock;
	SRequestHandler *pHandler=pPara->pHandler;
	int nRetCode;
	linger lng;
	char chRecvBuf[RECV_BUFFER];

	SOCKET hClientSock;
	string strHttpXml;
	while(!m_bShutdown)
	{		
#ifdef _WIN32
		EnterCriticalSection(&m_csAccept);
		hClientSock=accept(hSockLsn,NULL,NULL);
		LeaveCriticalSection(&m_csAccept);
#else
		pthread_mutex_lock(&m_pmAccept);
		hClientSock=accept(hSockLsn,NULL,NULL);
		pthread_mutex_unlock(&m_pmAccept);
#endif
		if(hClientSock==SOCKET_ERROR)
		{
			if(!m_bShutdown)
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			continue;
		}
		//printf("start to serve:id = %d\n",nSockNum);

		//printf("server thread id = %d,socket id = %d",nSockNum,hClientSock);

#ifdef _WIN32
		bool noDelay=1;
		if(SOCKET_ERROR==setsockopt(hClientSock,IPPROTO_TCP,TCP_NODELAY,(char*)&noDelay,sizeof(noDelay)))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		}
#endif

		lng.l_linger=1;
		lng.l_onoff=1;				
		if(SOCKET_ERROR==setsockopt(hClientSock,SOL_SOCKET,SO_LINGER,(char*)&lng,sizeof(lng)))			
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		}

		struct timeval to;
		to.tv_sec=5;
		to.tv_usec=0;

		struct timeval in;
		in.tv_sec=5;
		in.tv_usec=0;

		if(SOCKET_ERROR==setsockopt(hClientSock,SOL_SOCKET,SO_RCVTIMEO,(char*)&in,sizeof(in)))
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		if(SOCKET_ERROR==setsockopt(hClientSock,SOL_SOCKET,SO_SNDTIMEO,(char*)&to,sizeof(to)))
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));


		string strRec;  //接收数据缓冲区
		string strSnd;            //发送数据缓冲区
		while(true)
		{
			memset(chRecvBuf,0,sizeof(chRecvBuf));//每次初始化字串		
			nRetCode=recv(hClientSock,chRecvBuf,RECV_BUFFER-1,0);	
			if(nRetCode>0)
			{
				strRec+=chRecvBuf;
				if(strstr(chRecvBuf," HTTP")!=NULL)
					break;
			}
			else
				break;
		}


		if(SOCKET_ERROR==nRetCode)
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			closesocket(hClientSock);
			continue;
		}
		if(0==nRetCode)
		{
			printf("****connection has been gracefully closed \n");/*comment by msdn*/
			closesocket(hClientSock);
			continue;
		}

		(*pHandler)(strRec,strSnd);     
		char chHttpHeader[1024];


		sprintf(chHttpHeader,   "HTTP/1.1 200 OK\r\n"
			"Connection: close\r\n"
			"Server: FrameServer/1.0.0\r\n"  //**改成SELF
			"Content-Type: text/xml; charset=%s\r\n"
			"Content-Length: %d\r\n\r\n",RESPONSE_CHARSET_UTF8,strSnd.length());
		strHttpXml=chHttpHeader;
		strHttpXml+=strSnd;

		if (SOCKET_ERROR==send(hClientSock,strHttpXml.c_str(),strHttpXml.length(),0))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		}
		closesocket(hClientSock);
	}

	//			g_threadStatusArray[threadNum]=CLIENT_SOCKET_CLOSED;//关闭连接
	return 0;

}//while(!g_fShutdown)

bool CServerFrame::CreateServer(u_short nPort,u_short nThreadCount,SRequestHandler *pHandler)
{

	m_nLsnPort=nPort;
	m_nThreadCount=nThreadCount;
	m_pHandler=pHandler;

#ifdef _WIN32
	if(!InitSocketLib())
		return false;
#endif

	if (!BindToLocalHost(m_lsnSock,m_nLsnPort))
	{
		return false;
	}
#ifdef _WIN32
	InitializeCriticalSection(&m_csAccept);
#else
	pthread_mutex_init(&m_pmAccept,NULL);
#endif

	return true;
}



#ifdef _WIN32
bool  CServerFrame::InitSocketLib(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {	
		printf("Failed to load WinSock!\n");
		return false;
	}	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {	
			printf("Socket version is not 2.2\n");
			WSACleanup( );
			return false; 
	}
	return true;

}
#endif

bool  CServerFrame::BindToLocalHost(SOCKET &sock,u_short nPort)
{
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(INVALID_SOCKET==sock)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;
	}

 /* 使地址马上可以重用 */
    int nRet = 1;
    if(SOCKET_ERROR==setsockopt(m_lsnSock, SOL_SOCKET, SO_REUSEADDR, (char*)&nRet, sizeof(nRet)))
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
	}

	struct sockaddr_in addrSock;
	addrSock.sin_family=AF_INET;
	addrSock.sin_port=htons(nPort);
#ifdef _WIN32
	addrSock.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
#else
	addrSock.sin_addr.s_addr=htonl(INADDR_ANY);
#endif
	int retval;
	retval=bind(sock,(sockaddr*)&addrSock,sizeof(sockaddr));
	if(SOCKET_ERROR==retval)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		closesocket(sock);
		return false;
	}

	return true;

}
