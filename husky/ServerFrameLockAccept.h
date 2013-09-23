#ifndef HUSKY_SERVERFRAME_H 
#define HUSKY_SERVERFRAME_H 

#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "UtilDef.h"
#include "../cppcommon/headers.h"


namespace Husky
{

    using namespace CPPCOMMON;

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
