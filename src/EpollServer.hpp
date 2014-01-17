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
#include <sys/epoll.h>
#include <fcntl.h>
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
            static const size_t LISTEN_QUEUE_LEN = 1024;
            static const size_t RECV_BUFFER_SIZE = 1024 * 8;
            static const int MAXEPOLLSIZE = 512;

        private:
            const IRequestHandler* _reqHandler;
            int _host_socket;
            int _epoll_fd;
            bool _isInited;
            bool _isShutDown;
            bool _getInitFlag() const {return _isInited;}
            bool _setInitFlag(bool flag) {return _isInited = flag;} 
        public:
            explicit EpollServer(uint port, const IRequestHandler* pHandler): _reqHandler(pHandler), _host_socket(-1), _isShutDown(false)
            {
                assert(_reqHandler);
                _setInitFlag(_create_socket(port) && _create_epoll());
            };
            ~EpollServer(){};// unfinished;
        public:
            operator bool() const
            {
                return _getInitFlag();
            }
        private:
            bool _create_epoll()
            {
                _epoll_fd = epoll_create(MAXEPOLLSIZE);
                if(-1 == _epoll_fd)
                {
                    LogError(strerror(errno));
                    return false;
                }
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = _host_socket;

                /*set nonblocking*/
                if (-1 == fcntl(_host_socket, F_SETFL, fcntl(_host_socket, F_GETFD, 0)|O_NONBLOCK)) 
                {
                    LogError(strerror(errno));
                    return false;
                }
                if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _host_socket, &ev) < 0)
                {
                    LogError("epoll set insertion error: fd=%d", _host_socket);
                    return false;
                }
                LogInfo("socket added into epoll ok.");
                
                return true;
            }
            bool _create_socket(uint port)
            { 
                _host_socket = socket(AF_INET, SOCK_STREAM, 0);
                if(-1 == _host_socket)
                {
                    LogError(strerror(errno));
                    return false;
                }

                int nRet = 1;
                if(-1 == setsockopt(_host_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&nRet, sizeof(nRet)))
                {
                    LogError(strerror(errno));
                    return false;
                }

                struct sockaddr_in addrSock;
                addrSock.sin_family = AF_INET;
                addrSock.sin_port = htons(port);
                addrSock.sin_addr.s_addr = htonl(INADDR_ANY);
                if(-1 == ::bind(_host_socket, (sockaddr*)&addrSock, sizeof(sockaddr)))
                {
                    LogError(strerror(errno));
                    close(_host_socket);
                    return false;
                }
                if(-1 == listen(_host_socket, LISTEN_QUEUE_LEN))
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
                int clientSock;
                sockaddr_in clientaddr;
                socklen_t nSize = sizeof(clientaddr);
                //char recvBuf[RECV_BUFFER_SIZE];
                struct epoll_event events[MAXEPOLLSIZE];
                int nfds, new_fd;
                
                int curfds = 1;
                while(!_isShutDown)
                {
                    HttpReqInfo httpReq;
                    nfds = epoll_wait(_epoll_fd, events, curfds, -1);
                    if(-1 == nfds)
                    {
                        LogError(strerror(errno));
                        break;
                    }
                    for(int i = 0; i < nfds; i++)
                    {
                        if(events[i].data.fd == _host_socket)
                        {
                            new_fd = accept(_host_socket, (struct sockaddr*) &clientaddr, &nSize);
                            if(new_fd < 0)
                            {
                                LogError(strerror(errno));
                                continue;
                            }
                            LogInfo("有连接来自于： %d:%d， 分配的 socket 为:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), new_fd);
                            setnonblocking(new_fd);
                            ev.events = EPOLLIN | EPOLLET;
                            ev.data.fd = new_fd;
                            if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, new_fd, &ev) < 0)
                            {
                                LogError("把 socket '%d' 加入 epoll 失败！%s\n", new_fd, strerror(errno));
                                return false;
                            }
                            curfds++;
                        }
                        else
                        {
                            //ret = 
                            if(ret < 1 && errno != 11) // 11 errno means [Resource temporarily unavailable]
                            {
                                epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                                curdfs --;
                            }
                        }
                    }
                    clientSock = accept(_host_socket, (sockaddr *)&clientaddr, &nSize);
                    if(-1 == clientSock)
                    {
                        LogError(strerror(errno));
                        continue;
                    }
                    
                    httpReq[CLIENT_IP_K] = inet_ntoa(clientaddr.sin_addr);// inet_ntoa is not thread safety at some version 
                    if(-1 == setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (const char*)&LNG, sizeof(LNG)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    if(-1 == setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    if(-1 == setsockopt(clientSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT)))
                    {
                        LogError(strerror(errno));
                        close(clientSock);
                        continue;
                    }
                    string strRec, strSnd, strRetByHandler;
                    strRec.resize(RECV_BUFFER_SIZE);
                    int nRetCode = recv(clientSock, (char*)strRec.c_str(), strRec.size(), 0);

                    if(-1 == nRetCode)
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
                    if(-1 == send(clientSock, strSnd.c_str(), strSnd.length(), 0))
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
                if(-1 == close(_host_socket))
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
