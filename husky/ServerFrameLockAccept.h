#ifndef HUSKY_SERVERFRAME_H 
#define HUSKY_SERVERFRAME_H 

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include "UtilDef.h"
#include "../cppcommon/headers.h"
#include "SimpleThread.h"
#include "UtilDef.h"


namespace Husky
{

    using namespace CPPCOMMON;
    using namespace simpleThread;

    class IRequestHandler 
    {
        public:
            virtual ~IRequestHandler() =0;
        public:
            virtual bool init() = 0;
            virtual bool dispose() = 0;
            virtual void operator()(string& strRec, string &strSnd) = 0;

            virtual bool do_GET(const HttpReqInfo& httpReq, string& res) = 0;

    };
    

    class CServerFrame
    {
        public:

            CServerFrame(void);
            bool CreateServer(u_short nPort,u_short nThreadCount,IRequestHandler *pHandler);
            bool CloseServer();
            bool RunServer();
            // virtual  void HandleRequest(const string &strReceive,string& strSend){;}

        public:
            ~CServerFrame(void);

        protected:

            bool BindToLocalHost(SOCKET &sock,u_short nPort);

            static void* ServerThread(void *lpParameter );


        private:
            u_short  m_nLsnPort;
            u_short  m_nThreadCount;
            SOCKET   m_lsnSock;
            IRequestHandler *m_pHandler;
            static bool m_bShutdown ;                 // Signals client/server threads to die
            static PM m_pmAccept;

    }; 


    struct SPara
    {
        SOCKET hSock;
        IRequestHandler * pHandler;
    };
}
#endif
