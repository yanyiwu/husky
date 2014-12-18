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
#include "HttpReqInfo.hpp"
#include "IRequestHandler.hpp"
#include "NWorkerThread.hpp"
#include "Limonp/Logger.hpp"

namespace Husky
{
    using namespace Limonp;

    static const size_t RECV_BUFFER_SIZE = 4096; 
    static const size_t TASK_QUEUE_MAX_SIZE = 1024;
    static const size_t WORKER_NUMBER = 4;

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
                                server_->addTask(httpReq_);
                                cout << httpReq_ << endl;
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
                        LogDebug("closeConnection");
                        setIdle();
                        close(socket_);
                        server_->returnConnection(this);
                    }

                    void setRead()
                    {
                        socketState_ = SOCK_STATE_RECEIVE;
                        setFlags(EV_READ|EV_PERSIST);
                    }
                    void setWrite()
                    {
                        socketState_ = SOCK_STATE_SEND;
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
                                    if(httpReq_.parseHeader(readBuffer_))
                                    {
                                        connState_ = CONN_READ_REQUEST;
                                        transition();
                                        return;
                                    }
                                }
                                else if(ret == 0) 
                                {
                                    LogDebug("socket close from remote");
                                    connState_ = CONN_CLOSE;
                                    transition();
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

                    HttpReqInfo httpReq_;

            };

            NonblockingServer(
                        int port, 
                        const IRequestHandler& handler, 
                        size_t thread_number = WORKER_NUMBER, 
                        size_t queue_max_size = TASK_QUEUE_MAX_SIZE)
                : 
                    listener_(-1), 
                    reqHandler_(handler),
                    pool_(thread_number, queue_max_size),
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

                pool_.start();
                LogInfo("workers start.");

                LogInfo("server start. using libevent %s method %s", event_get_version(), event_base_get_method(evbase_));
                event_base_dispatch(evbase_);
            }

            void addTask(const HttpReqInfo& req)
            {
                pool_.add(CreateTask<NWorkerThread, const HttpReqInfo&, const IRequestHandler>(req, reqHandler_));
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
            ThreadPool pool_;

            struct event_base* evbase_;
    };
}

#endif
