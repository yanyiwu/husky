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
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "NetUtils.hpp"
#include "IRequestHandler.hpp"

namespace Husky
{
    class Connection
    {
        public:
            Connection()
            {
            }
            ~Connection()
            {
            }
        public:
            SocketFd socket_;

            char* readBuffer_;
            size_t readBufferSize_;

            char* writeBuffer_;
            size_t writeBufferSize_;
    };
    
    class NonblockingServer
    {
        public:
            NonblockingServer(int port, const IRequestHandler& handler)
                : listener_(-1), reqHandler_(handler)
            {
                listener_ = CreateAndListenSocket(port);
                evutil_make_socket_nonblocking(listener_);
            }
            ~NonblockingServer()
            {
            }
            void start()
            {
            }
        private:
            SocketFd listener_;
            const IRequestHandler& reqHandler_;
    };
}

#endif
