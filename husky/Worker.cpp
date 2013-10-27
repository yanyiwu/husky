#include "Worker.h"

namespace Husky
{
    Worker::Worker(IRequestHandler* pHandler):m_pHandler(pHandler){};

    bool Worker::Init(unsigned int port, unsigned int threadNum)
    {
        if(!m_pHandler->init())
        {
            return false;
        }
        return m_server.CreateServer(port, threadNum, m_pHandler);
    }

    bool Worker::Run()
    {
        return m_server.RunServer();
    }

    bool Worker::Dispose()
    {
        return m_pHandler->dispose();
    } 

    bool Worker::CloseServer()
    {
        return m_server.CloseServer();
    }
}
