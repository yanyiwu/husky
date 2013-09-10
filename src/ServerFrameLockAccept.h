#ifndef SERVERFRAME_H 
#define SERVERFRAME_H 
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <vector>
#define INVALID_SOCKET  -1 
#define SOCKET_ERROR    -1 
#define closesocket     close
#define  RECV_BUFFER     10240
#define  LISEN_QUEUR_LEN 1024
namespace Husky
{
    typedef unsigned short  u_short;
    typedef unsigned int    u_int;
    typedef	int             SOCKET;
    typedef pthread_mutex_t PM;
    typedef pthread_cond_t  PC;

    using std::vector;
    using std::string;


    struct SRequestHandler 
    {
        virtual bool init(){return true;}
        virtual bool dispose(){return true;}
        virtual void operator()(string &strRec, string &strSnd)
        {
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

            bool BindToLocalHost(SOCKET &sock,u_short nPort);

            static void* ServerThread(void *lpParameter );


        private:
            u_short  m_nLsnPort;                            // 侦听端口
            u_short  m_nThreadCount;                     // 服务进程数量
            SOCKET   m_lsnSock;                           //侦听SOCKT


            SRequestHandler *m_pHandler;
            static bool m_bShutdown ;                 // Signals client/server threads to die
            static PM m_pmAccept;

    }; 


    struct SPara
    {
        SOCKET hSock;
        SRequestHandler * pHandler;
    };
}
#endif
