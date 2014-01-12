#ifndef HUSKY_EPOLLSERVER_H 
#define HUSKY_EPOLLSERVER_H 

#include <stdio.h>
#include <string.h>

#include <cassert>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include "logger.hpp"


namespace Husky
{
    using namespace Limonp;

    const struct linger LNG = {1, 1};
    const struct timeval SOCKET_TIMEOUT = {2, 0};
    class EpollServer
    {
        private:
            typedef int SOCKET;
            static const int INVALID_SOCKET = -1;
            static const int SOCKET_ERROR = -1;
            static const size_t LISTEN_QUEUE_LEN = 1024;
            static const size_t RECV_BUFFER_SIZE = 1024 * 8;
        private:
            SOCKET _host_socket;
            bool _isInited;
            bool _getInitFlag() const {return _isInited;}
            bool _setInitFlag(bool flag) {return _isInited = flag;} 
        public:
            EpollServer()
            {
                _host_socket = -1;
                _setInitFlag(false);
            }
            explicit EpollServer(uint port)
            {
                EpollServer();
                _setInitFlag(_bind(port));
            };
            ~EpollServer(){};
        public:
            operator bool() const
            {
                return _getInitFlag();
            }
        public:
            bool _bind(uint port)
            { 
                _host_socket = socket(AF_INET, SOCK_STREAM, 0);
                if(INVALID_SOCKET == _host_socket)
                {
                    LogError(strerror(errno));
                    return false;
                }

                int nRet = 1;
                if(SOCKET_ERROR == setsockopt(_host_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&nRet, sizeof(nRet)))
                {
                    LogError(strerror(errno));
                    return false;
                }

                struct sockaddr_in addrSock;
                addrSock.sin_family = AF_INET;
                addrSock.sin_port = htons(port);
                addrSock.sin_addr.s_addr = htonl(INADDR_ANY);
                if(SOCKET_ERROR == ::bind(_host_socket, (sockaddr*)&addrSock, sizeof(sockaddr)))
                {
                    LogError(strerror(errno));
                    close(_host_socket);
                    return false;
                }
                if(SOCKET_ERROR == listen(_host_socket, LISTEN_QUEUE_LEN))
                {
                    LogError(strerror(errno));
                    return false;
                }
                LogInfo("listen port[%u]", port);
                return true;
            }
        public:
            bool run()
            {
                SOCKET clientSock;
                sockaddr_in clientaddr;
                socklen_t nSize = sizeof(clientaddr);
                //char recvBuf[RECV_BUFFER_SIZE];
                while(true)
                {
                    clientSock = accept(_host_socket, (sockaddr *)&clientaddr, &nSize);
                    if(SOCKET_ERROR == clientSock)
                    {
                        LogError(strerror(errno));
                        continue;
                    }
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (const char*)&LNG, sizeof(LNG)))
                    {
                        LogError(strerror(errno));
                    }
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                    }
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                    }
                    string strRec, strSnd;
                    strRec.resize(RECV_BUFFER_SIZE);
                    //memset(recvBuf, 0, sizeof(recvBuf));
                    int nRetCode = recv(clientSock, (char*)strRec.c_str(), strRec.size(), 0);

                    if(SOCKET_ERROR == nRetCode)
                    {
                        LogDebug(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    if(0 == nRetCode)
                    {
                        LogDebug("client socket closed gracefully.");
                        close(clientSock);
                        continue;
                    }
                    
                }
                return true;
            }
    };
}
#endif
