#ifndef HUSKY_DAEMON_H_
#define HUSKY_DAEMON_H_
#include "UtilDef.h"
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
            virtual bool Init(HIS&his){return true;};
            virtual bool Run(){return true;};
            virtual bool Dispose(){return true;};
            virtual bool close(){return true;};


    };

    class CWorkerEx:public CWorker
    {
        public:
            CWorkerEx(SRequestHandler* pHandler);
            virtual bool Init(HIS&his);
            virtual bool Run();
            virtual bool Dispose();
            virtual bool close();

        private:
            CServerFrame     m_server;
            SRequestHandler* m_pHandler;

    };

    class CDaemon
    {
        public:
            CDaemon(CWorker *pWorker){m_pWorker=pWorker;}

            int ParseCmdLine(int argc,char** argv);
            bool Start();
            bool Stop();
            static void initAsDaemon();
            static void sigMasterHandler(int sig);
            static void sigChildHandler(int sig);

            bool Run(int argc,char** argv);
            CDaemon(void);
            virtual ~CDaemon(void);
        private:
            HIS      m_hisOptVal;
            string   m_runPath;
            char*    m_pName;
            static	CWorker *m_pWorker;
            static	int m_nChildPid;


    };
}
#endif
