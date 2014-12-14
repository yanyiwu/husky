#ifndef HUSKY_NONBLOCKING_SERVER_H
#define HUSKY_NONBLOCKING_SERVER_H

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

//deps
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "NetUtils.hpp"
#include "IRequestHandler.hpp"
#include "Limonp/Logger.hpp"

namespace Husky
{
    using namespace Limonp;

    enum ConnState 
    {
        CONN_INIT,
        CONN_READ_REQUEST,
        CONN_WAIT_TASK,
        CONN_SEND_RESULT,
        CONN_CLOSE
    };

    class NonblockingServer
    {
        public:
            class NConnection 
            {
                public:
                    NConnection(SocketFd sock, NonblockingServer* server)
                        : sock_(sock), server_(server)
                    {
                        connState_ = CONN_CLOSE;
                    }
                    ~NConnection()
                    {
                    }

                    void transition()
                    {
                        LogDebug("transition");
                        switch(connState_)
                        {
                            case CONN_READ_REQUEST:
                                LogDebug("CONN_READ_REQUEST");
                                return;
                            case CONN_CLOSE:
                                LogDebug("CONN_CLOSE");
                                close(sock_);
                                server_->returnConnection(this);
                                return;
                            default:
                                LogFatal("Unexpected.");
                                assert(false);
                        }
                    }

                private:
                    SocketFd sock_;
                    NonblockingServer* server_;

                    char* readBuffer_;
                    size_t readBufferSize_;

                    char* writeBuffer_;
                    size_t writeBufferSize_;

                    ConnState connState_;
            };

            NonblockingServer(int port, const IRequestHandler& handler)
                : 
                    listener_(-1), 
                    reqHandler_(handler),
                    evbase_(NULL)
            {
                listener_ = CreateAndListenSocket(port);
                int ret = evutil_make_socket_nonblocking(listener_);
                LIMONP_CHECK(ret != -1);
                
                evbase_ = event_base_new();
                LIMONP_CHECK(evbase_);
            }
            ~NonblockingServer()
            {
                if(listener_ != -1)
                {
                    close(listener_);
                }
                if(evbase_)
                {
                    event_base_free(evbase_);
                }
            }
            void start()
            {
                registerEvents();
                LogInfo("server start. using libevent %s method %s", event_get_version(), event_base_get_method(evbase_));
                event_base_dispatch(evbase_);
            }
        private:
            static void listenHandler(SocketFd fd, short which, void *ctx)
            {
                ((NonblockingServer*)ctx)->handleEvent(fd, which);
            }

            void handleEvent(SocketFd fd, short which)
            {
                assert(fd == listener_);

                sockaddr_in addr;
                socklen_t addrlen;
                SocketFd clientSock;
                
                while((clientSock = ::accept(fd, (struct sockaddr*)&addr, &addrlen)) != -1)
                {
                    int ret = evutil_make_socket_nonblocking(clientSock);
                    LIMONP_CHECK(ret != -1);
                    
                    NConnection* clientConn = createConnection(clientSock);
                    //TODO delete
                    clientConn->transition();
                    //NConnection
                }
            }

            void registerEvents()
            {
                struct event* listener_event = event_new(
                    evbase_,
                    listener_,
                    EV_READ | EV_PERSIST,
                    listenHandler,
                    this
                );
                event_add(listener_event, NULL);
            }

            NConnection* createConnection(SocketFd sock)
            {
                LogDebug("new NConnection");
                NConnection* result;
                result = new NConnection(sock, this);
                return result;
            }
            void returnConnection(NConnection* conn)
            {
                LogDebug("del NConnection");
                delete conn;
            }

            SocketFd listener_;
            const IRequestHandler& reqHandler_;

            struct event_base* evbase_;
    };
}

#endif
