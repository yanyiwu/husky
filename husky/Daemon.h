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
#include "ServerFrameLockAccept.h"
#include "../cppcommon/headers.h"

namespace Husky
{
    using namespace CPPCOMMON;

    class CWorker
    {
        public:
            CWorker(IRequestHandler* pHandler);
            virtual bool Init(HIS&his);
            virtual bool Run();
            virtual bool Dispose();
            virtual bool close();

        private:
            CServerFrame     m_server;
            IRequestHandler* m_pHandler;

    };

    class CDaemon
    {
        public:
            CDaemon(){};
            virtual ~CDaemon(){};
            CDaemon(CWorker *pWorker);

            int ParseCmdLine(int argc,char** argv);
            bool Start();
            bool Stop();
            static void initAsDaemon();
            static void sigMasterHandler(int sig);
            static void sigChildHandler(int sig);
            bool Run(int argc,char** argv);
        private:
            HIS      m_hisOptVal;
            static	CWorker *m_pWorker;
            static	int m_nChildPid;
    };
}
#endif
