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
#include "Limonp/Logger.hpp"
#include "Limonp/ThreadPool.hpp"

namespace Husky {
using namespace Limonp;

static const size_t RECV_BUFFER_SIZE = 4096;
static const size_t TASK_QUEUE_MAX_SIZE = 1024;
static const size_t WORKER_NUMBER = 4;

enum SocketState {
  SOCK_STATE_RECEIVE,
  SOCK_STATE_SEND,
  SOCK_STATE_CLOSE
};

enum ConnState {
  CONN_INIT,
  CONN_READ_REQUEST,
  CONN_WAIT_TASK,
  CONN_SEND_RESULT,
  CONN_CLOSE
};

class NonblockingServer {
 public:
  class NConnection {
   public:
    NConnection(SocketFd sock, NonblockingServer* server)
      :
      socket_(sock),
      server_(server),
      connState_(CONN_INIT),
      socketState_(SOCK_STATE_RECEIVE),
      event_(NULL),
      eventFlags_(0),
      readBuffer_(),
      readBufferOffset_(0),
      writeBuffer_(),
      writeBufferOffset_(0) {
      this->transition();
    }
    ~NConnection() {
      if(event_) {
        event_free(event_);
      }
    }

    void transition() {
      switch(connState_) {
      case CONN_INIT:
        setRead();
        return;
      case CONN_READ_REQUEST:
        setConnState(CONN_WAIT_TASK);
        transition();
        return;
      case CONN_WAIT_TASK:
        setIdle();
        server_->addTask(httpReq_, this);
        return;
      case CONN_SEND_RESULT:
        setWrite();
        return;
      case CONN_CLOSE:
        closeConnection();
        return;
      default:
        LogFatal("Unexpected.");
      }
    }

    void closeConnection() {
      setIdle();
      close(socket_);
      server_->returnConnection(this);
    }

    NonblockingServer* getServer() {
      return server_;
    }

    void setRead() {
      socketState_ = SOCK_STATE_RECEIVE;
      setFlags(EV_READ|EV_PERSIST);
    }
    void setWrite() {
      socketState_ = SOCK_STATE_SEND;
      setFlags(EV_WRITE|EV_PERSIST);
    }
    void setIdle() {
      setFlags(0);
    }
    void setTaskResult(const string& res) {
      writeBuffer_ = res;
    }
    void setConnState(ConnState state) {
      connState_ = state;
    }
   private:

    void setFlags(short flags) {
      if(flags == eventFlags_) {
        return ;
      }
      if(event_ != NULL) {
        if(-1 == event_del(event_)) {
          LogError("event_del failed.");
          return;
        }
        event_free(event_);
        event_ = NULL;
      }

      eventFlags_ = flags;

      // Do not call event_set if there are no flags
      if(eventFlags_ == 0) {
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

    static void eventHandler(SocketFd fd, short, void * ctx) {
      NConnection *self = (NConnection*)ctx;
      LIMONP_CHECK(fd == self->getSocketFd());
      self->workSocket();
    }

    void workSocket() {
      int ret;
      switch(socketState_) {
      case SOCK_STATE_RECEIVE:
        char buffer[RECV_BUFFER_SIZE];
        ret = ::recv(socket_, buffer, sizeof(buffer), 0);
        if(ret > 0) {
          readBuffer_.append(buffer, ret);
          if(httpReq_.parseHeader(readBuffer_)) {
            setConnState(CONN_READ_REQUEST);
            this->transition();
            return;
          }
        } else if(ret == 0) {
          LogDebug("socket close from remote");
          setConnState(CONN_CLOSE);
          this->transition();
          return;
        } else {
          LogError(strerror(errno));
        }
        return;
      case SOCK_STATE_SEND:
        LIMONP_CHECK(writeBuffer_.size() >= writeBufferOffset_);
        size_t sendsize;
        const char* sendbuf;
        sendsize = writeBuffer_.size() - writeBufferOffset_ ;
        sendbuf = writeBuffer_.c_str() + writeBufferOffset_;
        if(sendsize == 0) {
          socketState_ = SOCK_STATE_CLOSE;
          return;
        }
        ret = ::send(socket_, sendbuf, sendsize, 0);
        if(ret == -1) {
          LogError(strerror(errno));
          return;
        }
        writeBufferOffset_ += sendsize;
      case SOCK_STATE_CLOSE:
        server_->returnConnection(this);
        return;
      default:
        LogFatal("unexpected.");
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
  class NWorkerThread: public ITask {
   public:
    NWorkerThread(const HttpReqInfo& req, const IRequestHandler& reqHandler, NConnection* conn)
      : httpReq_(req), reqHandler_(reqHandler), conn_(conn) {
    }
    virtual ~NWorkerThread() {
    }

    void run() {
      string sendbuf;
      string result;
      if(httpReq_.isGET() && !reqHandler_.do_GET(httpReq_, result)) {
        LogError("do_GET failed.");
        return;
      }
      if(httpReq_.isPOST() && !reqHandler_.do_POST(httpReq_, result)) {
        LogError("do_POST failed.");
        return;
      }
      sendbuf = string_format(HTTP_FORMAT, CHARSET_UTF8, result.length(), result.c_str());
      conn_->setTaskResult(sendbuf);
      conn_->getServer()->notify(conn_);
    }
   private:
    HttpReqInfo httpReq_;
    const IRequestHandler& reqHandler_;
    NConnection* conn_;
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
    evbase_(NULL) {
    listener_ = CreateAndListenSocket(port);
    int ret = evutil_make_socket_nonblocking(listener_);
    LIMONP_CHECK(ret != -1);
    notificationPipeFDs_[0] = -1;
    notificationPipeFDs_[1] = -1;
    evbase_ = event_base_new();
    LIMONP_CHECK(evbase_);
  }
  ~NonblockingServer() {
    if(listener_ != -1) {
      close(listener_);
    }
    if(notificationPipeFDs_[0] > 0) {
      close(notificationPipeFDs_[0]);
    }
    if(notificationPipeFDs_[1] > 0) {
      close(notificationPipeFDs_[1]);
    }
    if(evbase_) {
      event_base_free(evbase_);
    }
  }
  void start() {
    createNotificationPipe();
    registerEvents();
    pool_.start();
    LogInfo("workers start.");

    LogInfo("server start. using libevent %s method %s", event_get_version(), event_base_get_method(evbase_));
    event_base_dispatch(evbase_);
  }

  void addTask(const HttpReqInfo& req, NConnection* conn) {
    pool_.add(CreateTask<NWorkerThread, const HttpReqInfo&, const IRequestHandler&, NConnection*>(req, reqHandler_, conn));
  }
 private:
  static void listenHandler(SocketFd fd, short which, void *ctx) {
    ((NonblockingServer*)ctx)->handleEvent(fd, which);
  }

  void handleEvent(SocketFd fd, short which) {
    assert(fd == listener_);

    sockaddr_in addr;
    socklen_t addrlen;
    SocketFd clientSock;

    while((clientSock = ::accept(fd, (struct sockaddr*)&addr, &addrlen)) != -1) {
      int ret = evutil_make_socket_nonblocking(clientSock);
      LIMONP_CHECK(ret != -1);

      NConnection* clientConn = createConnection(clientSock);
      clientConn->transition();
    }
  }

  void registerEvents() {
    struct event* listener_event = event_new(
                                     evbase_,
                                     listener_,
                                     EV_READ | EV_PERSIST,
                                     listenHandler,
                                     this
                                   );
    event_add(listener_event, NULL);

    struct event* notification_event = event_new(
                                         evbase_,
                                         getNotificationRecvFD(),
                                         EV_READ | EV_PERSIST,
                                         notifyHandler,
                                         this
                                       );
    event_add(notification_event, NULL);
  }

  NConnection* createConnection(SocketFd sock) {
    NConnection* result;
    result = new NConnection(sock, this);
    return result;
  }
  void returnConnection(NConnection* conn) {
    delete conn;
  }

  void createNotificationPipe() {
    int ret;
    ret = evutil_socketpair(AF_LOCAL, SOCK_STREAM, 0, notificationPipeFDs_);
    LIMONP_CHECK(ret != -1);
    ret = evutil_make_socket_nonblocking(notificationPipeFDs_[0]);
    LIMONP_CHECK(ret != -1);
    ret = evutil_make_socket_nonblocking(notificationPipeFDs_[1]);
    LIMONP_CHECK(ret != -1);
  }

  struct event_base* getEventBase() const {
    return evbase_;
  }

  void notify(NConnection* conn) {
    SocketFd fd = getNotificationSendFD();
    LIMONP_CHECK(fd);
    const int size = sizeof(conn);
    if(size != send(fd, &conn, size, 0)) {
      LogError("send failed.");
      return;
    }
    return;
  }

  static void notifyHandler(SocketFd fd, short which, void * ctx) {
    //NonblockingServer * self = (NonblockingServer*) ctx;
    while(true) {
      NConnection * conn;
      int n = recv(fd, &conn, sizeof(conn), 0);
      if(n == sizeof(conn)) {
        conn->setConnState(CONN_SEND_RESULT);
        conn->transition();
        continue;
      }
      LIMONP_CHECK(n <= 0); //TODO
      return;
    }
  }

  SocketFd getNotificationSendFD() {
    return notificationPipeFDs_[1];
  }
  SocketFd getNotificationRecvFD() {
    return notificationPipeFDs_[0];
  }

  SocketFd listener_;
  SocketFd notificationPipeFDs_[2];
  const IRequestHandler& reqHandler_;
  ThreadPool pool_;

  struct event_base* evbase_;
};
}

#endif
