#ifndef DAEMON_H_
#define DAEMON_H_
#include "UtilDef.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include "ServerFrameLockAccept.h"
//#include "XmlHttp.h"
#define MASTER_PID_FILE 	"masterDaemon.pid."//daemon 主进程PID文件

namespace Husky

{
    class CWorker
    {
        public:
            virtual bool Init(HIS&his){return true;};
            virtual bool Run(){return true;};
            virtual bool Dispose(){return true;};
            virtual bool close(){return true;};


    };
    //struct SDerivedHandler:public SRequestHandler 
    //{ 
    //
    //	bool Init();
    //	bool Dispose();
    //	virtual void operator()(string &strRec, string &strSnd);
    //	Server m;
    //};
    class CWorkerEx:public CWorker
    {
        public:
            CWorkerEx(SRequestHandler* pHandler);
            virtual bool Init(HIS&his);
            virtual bool Run();
            virtual bool Dispose();
            virtual bool close();

        private:
            CServerFrame     m_server;
            SRequestHandler* m_pHandler;

    };



    class CDaemon
    {

        public:
            CDaemon(CWorker *pWorker){m_pWorker=pWorker;}

            //读文件第一行：buf ，读入缓冲区，maxCount最大字节数量，mode 文件打开方式，失败返回 0。
            int Read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode);
            //将缓冲区写入文件：buf 缓冲区 ，mode 文件打开方式，失败返回 0。
            int WriteBuff2File(const char* fileName, const char* buf, const char* mode);


            int ParseCmdLine(int argc,char** argv);
            bool Start();
            bool Stop();
            static void initAsDaemon();
            static void sigMasterHandler(int sig);
            static void sigChildHandler(int sig);

            bool Run(int argc,char** argv);
            CDaemon(void);
            virtual ~CDaemon(void);
        private:
            HIS      m_hisOptVal;
            string   m_runPath;
            char*    m_pName;
            static	CWorker *m_pWorker;
            static	int m_nChildPid;


    };
}
#endif
