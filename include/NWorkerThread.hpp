#ifndef HUSKY_NWORKER_THREAD_H
#define HUSKY_NWORKER_THREAD_H

#include "HttpReqInfo.hpp"
#include "NetUtils.hpp"
#include "Limonp/ThreadPool.hpp"

namespace Husky
{
    class NWorkerThread: public ITask
    {
        public:
            NWorkerThread(const HttpReqInfo& req, const IRequestHandler& reqHandler)
                : httpReq_(req), reqHandler_(reqHandler_)
            {
                cout << __FILE__ << __LINE__ << endl;
            }
            virtual ~NWorkerThread()
            {
            }
            
            void run()
            {
                string sendbuf;
                string result;
                cout << __FILE__ << __LINE__ << endl;
                cout << httpReq_ << endl;
                cout << httpReq_.isGET() << endl;
                if(httpReq_.isGET() && !reqHandler_.do_GET(httpReq_, result))
                {
                    LogError("do_GET failed.");
                    return;
                }
                if(httpReq_.isPOST() && !reqHandler_.do_POST(httpReq_, result))
                {
                    LogError("do_POST failed.");
                    return;
                }
                sendbuf = string_format(HTTP_FORMAT, CHARSET_UTF8, result.length(), result.c_str());
                LogDebug(sendbuf.c_str());
            }
        private:
            HttpReqInfo httpReq_;
            const IRequestHandler& reqHandler_;
    };
}

#endif
