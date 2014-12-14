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

    static const size_t RECV_BUFFER_SIZE = 4096; 

    enum SocketState
    {
        SOCK_STATE_RECEIVE,
        SOCK_STATE_SEND
    };

    enum ConnState 
    {
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
                        : 
                            socket_(sock), 
                            server_(server), 
                            connState_(CONN_READ_REQUEST),
                            socketState_(SOCK_STATE_RECEIVE),
                            event_(NULL),
                            eventFlags_(0),
                            readBuffer_(),
                            readBufferOffset_(0),
                            writeBuffer_(),
                            writeBufferOffset_(0)
                    {
                        setRead();
                    }
                    ~NConnection()
                    {
                        if(event_)
                        {
                            event_free(event_);
                        }
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
                                closeConnection();
                                return;
                            default:
                                LogFatal("Unexpected.");
                                assert(false);
                        }
                    }

                    void closeConnection()
                    {
                        setIdle();
                        close(socket_);
                        server_->returnConnection(this);
                    }

                    void setRead()
                    {
                        setFlags(EV_READ|EV_PERSIST);
                    }
                    void setWrite()
                    {
                        setFlags(EV_WRITE|EV_PERSIST);
                    }
                    void setIdle()
                    {
                        setFlags(0);
                    }

                    void setFlags(short flags)
                    {
                        if(flags == eventFlags_) 
                        {
                            return ;
                        }
                        if(event_ != NULL)
                        {
                            if(-1 == event_del(event_))
                            {
                                LogError("event_del failed.");
                                return;
                            }
                            event_free(event_);
                            event_ = NULL;
                        }

                        eventFlags_ = flags;

                        // Do not call event_set if there are no flags
                        if(eventFlags_ == 0)
                        {
                            return;
                        }
                        LIMONP_CHECK(event_ == NULL);
                        event_ = event_new(
                                    server_->getEventBase(), 
                                    socket_,
                                    eventFlags_,
                                    eventHandler,
                                    this
                        );
                        event_add(event_, NULL);
                    }

                private:
                    static void eventHandler(SocketFd fd, short, void * ctx)
                    {
                        LogDebug("eventHandler");
                        NConnection *self = (NConnection*)ctx;
                        LIMONP_CHECK(fd == self->getSocketFd());
                        self->workSocket();
                    }

                    void workSocket()
                    {
                        int ret;
                        switch(socketState_)
                        {
                            case SOCK_STATE_RECEIVE:
                                char buffer[RECV_BUFFER_SIZE];
                                ret = ::recv(socket_, buffer, sizeof(buffer), 0);
                                if(ret > 0)
                                {
                                    cout << __FILE__ << __LINE__ << endl;
                                    readBuffer_.append(buffer, ret);
                                    cout << readBuffer_.size() << endl;
                                    cout << readBuffer_ << endl;
                                }
                                else if(ret == 0) 
                                {
                                    LogDebug("socket close from remote");
                                    closeConnection();
                                    return;
                                }
                                else
                                {
                                    LogError(strerror(errno));
                                }
                                return;
                            case SOCK_STATE_SEND:
                                LIMONP_CHECK(writeBuffer_.size() >= writeBufferOffset_);
                                size_t sendsize;
                                const char* sendbuf;
                                sendsize = writeBuffer_.size() - writeBufferOffset_ ;
                                sendbuf = writeBuffer_.c_str() + writeBufferOffset_;
                                ret = ::send(socket_, sendbuf, sendsize, 0);
                                if(ret == -1)
                                {
                                    LogError(strerror(errno));
                                    return;
                                }
                                writeBufferOffset_ += sendsize;
                                return;
                            default:
                                LogFatal("unexpected.");
                                assert(0);
                        }
                    }

                    SocketFd getSocketFd() const {
                        return socket_;
                    }

                    SocketFd socket_;
                    NonblockingServer* server_;
                    ConnState connState_;
                    SocketState socketState_;
                    struct event* event_;
                    short eventFlags_;

                    string readBuffer_;
                    size_t readBufferOffset_;
                    string writeBuffer_;
                    size_t writeBufferOffset_;

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
            
            struct event_base* getEventBase() const 
            {
                return evbase_;
            }

            SocketFd listener_;
            const IRequestHandler& reqHandler_;

            struct event_base* evbase_;
    };
}

#endif
