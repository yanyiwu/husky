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



namespace Husky
{
    class EpollServer
    {
        public:
            EpollServer(){};
            ~EpollServer(){};
        public:
            operator bool()
            {
                return true;
            }
    };
}
#endif
