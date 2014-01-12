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

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

namespace Husky
{
    using namespace Limonp;
    class EpollServer
    {
        private:
            typedef int SOCKET;
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
                LogInfo("listen port[%u]", port);
                return true;
            }
    };
}
#endif
