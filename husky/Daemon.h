#ifndef HUSKY_DAEMON_H_
#define HUSKY_DAEMON_H_
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <logger.hpp>
#include "ServerFrame.h"


namespace Husky
{
    using namespace Limonp;

    class CWorker
    {
        public:
            CWorker(IRequestHandler* pHandler);
            virtual bool Init(unsigned int port, unsigned int threadNum);
            virtual bool Run();
            virtual bool Dispose();
            virtual bool CloseServer();

        private:
            CServerFrame     m_server;
            IRequestHandler* m_pHandler;

    };

    class CDaemon
    {
        public:
            CDaemon(CWorker *pWorker);
            ~CDaemon(){};
        public:
            bool Start(unsigned int port, unsigned int threadNum);
            bool Stop();
        public:
            static void initAsDaemon();
            static void sigMasterHandler(int sig);
            static void sigChildHandler(int sig);
            static bool isAbnormalExit(int pid, int status);
        private:
            static	CWorker *m_pWorker;
            static	int m_nChildPid;
    };
}
#endif
