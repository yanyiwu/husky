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
#include "HttpReqInfo.hpp"


namespace Husky
{
    using namespace Limonp;

    const char* const HTTP_FORMAT = "HTTP/1.1 200 OK\r\nConnection: close\r\nServer: HuskyServer/1.0.0\r\nContent-Type: text/json; charset=%s\r\nContent-Length: %d\r\n\r\n%s";
    const char* const CHARSET_UTF8 = "UTF-8";
    const char* const CLIENT_IP_K = "CLIENT_IP"; 

    const struct linger LNG = {1, 1};
    const struct timeval SOCKET_TIMEOUT = {5, 0};

    class IRequestHandler
    {
        public:
            virtual ~IRequestHandler(){};
        public:
            virtual bool do_GET(const HttpReqInfo& httpReq, string& res) const = 0;
    };

    class EpollServer
    {
        private:
            typedef int SOCKET;
            static const int INVALID_SOCKET = -1;
            static const int SOCKET_ERROR = -1;
            static const size_t LISTEN_QUEUE_LEN = 1024;
            static const size_t RECV_BUFFER_SIZE = 1024 * 8;
        private:
            const IRequestHandler* _reqHandler;
            SOCKET _host_socket;
            bool _isInited;
            bool _isShutDown;
            bool _getInitFlag() const {return _isInited;}
            bool _setInitFlag(bool flag) {return _isInited = flag;} 
        public:
            explicit EpollServer(uint port, const IRequestHandler* pHandler): _reqHandler(pHandler), _host_socket(-1), _isShutDown(false)
            {
                assert(_reqHandler);
                _setInitFlag(_bind(port));
            };
            ~EpollServer(){};// unfinished;
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
            bool start()
            {
                SOCKET clientSock;
                sockaddr_in clientaddr;
                socklen_t nSize = sizeof(clientaddr);
                //char recvBuf[RECV_BUFFER_SIZE];
                while(!_isShutDown)
                {
                    HttpReqInfo httpReq;
                    clientSock = accept(_host_socket, (sockaddr *)&clientaddr, &nSize);
                    if(SOCKET_ERROR == clientSock)
                    {
                        LogError(strerror(errno));
                        continue;
                    }
                    
                    httpReq[CLIENT_IP_K] = inet_ntoa(clientaddr.sin_addr);// inet_ntoa is not thread safety at some version 
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (const char*)&LNG, sizeof(LNG)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    if(SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    string strRec, strSnd, strRetByHandler;
                    strRec.resize(RECV_BUFFER_SIZE);
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

                    httpReq.load(strRec);
                    _reqHandler->do_GET(httpReq, strRetByHandler);
                    string_format(strSnd, HTTP_FORMAT, CHARSET_UTF8, strRetByHandler.length(), strRetByHandler.c_str());
                    if(SOCKET_ERROR == send(clientSock, strSnd.c_str(), strSnd.length(), 0))
                    {
                        LogError(strerror(errno));
                    }
                    close(clientSock);
                }
                return true;
            }
            void stop()
            {
                _isShutDown = true;
                if(SOCKET_ERROR == close(_host_socket))
                {
                    LogError(strerror(errno));
                    return;
                }
                int sockfd;
                struct sockaddr_in dest;
                if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    LogError(strerror(errno));
                    return;
                }
                bzero(&dest, sizeof(dest));
                dest.sin_family = AF_INET;
                dest.sin_port = htons(_host_socket);
                if(0 == inet_aton("127.0.0.1", (struct in_addr *) &dest.sin_addr.s_addr))
                {
                    LogError(strerror(errno));
                    return;
                }
                if(connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) < 0)
                {
                    LogError(strerror(errno));
                }
                close(sockfd);
            }
    };
}
#endif
