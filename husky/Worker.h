#ifndef HUKSY_WORKER_H
#define HUKSY_WORKER_H

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

    class Worker
    {
        public:
            Worker(IRequestHandler* pHandler);
            virtual bool Init(unsigned int port, unsigned int threadNum);
            virtual bool Run();
            virtual bool Dispose();
            virtual bool CloseServer();

        private:
            ServerFrame     m_server;
            IRequestHandler* m_pHandler;

    };

}

#endif
