#include "Daemon.h"

namespace Husky
{
    Worker::Worker(IRequestHandler* pHandler):m_pHandler(pHandler){};

    bool Worker::Init(unsigned int port, unsigned int threadNum)
    {
        if(!m_pHandler->init())
        {
            return false;
        }
        return m_server.CreateServer(port, threadNum, m_pHandler);
    }

    bool Worker::Run()
    {
        return m_server.RunServer();
    }

    bool Worker::Dispose()
    {
        return m_pHandler->dispose();
    } 

    bool Worker::CloseServer()
    {
        return m_server.CloseServer();
    }

    Worker *Daemon::m_pWorker = NULL;
    int Daemon::m_nChildPid = 0;
    Daemon::Daemon(Worker *pWorker){m_pWorker = pWorker;};

    /* modify from book apue
     * 为防止子进程意外死亡, 主进程会重新生成子进程.
     * 除非:1.子进程以EXIT方式退出. 2. kill -9 杀死该进程
     */
    bool Daemon::isAbnormalExit(int pid, int status)
    {
        bool bRestart = true;
        if (WIFEXITED(status)) //exit()or return 
        {
            LogInfo("child normal termination, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
            bRestart = false;
        }
        else if (WIFSIGNALED(status)) //signal方式退出
        {
            LogError("abnormal termination, pid = %d, signal number = %d%s", pid, WTERMSIG(status),
#ifdef	WCOREDUMP
                        WCOREDUMP(status) ? " (core file generated)" : 
#endif
                        ""); 

            if (WTERMSIG(status) == SIGKILL)
            {
                bRestart = false;
                LogError("has been killed by user , exit pid = %d, status = %d", pid, WEXITSTATUS(status));
            }
        }
        else if (WIFSTOPPED(status)) //暂停的子进程退出
        {
            LogError("child stopped, pid = %d, signal number = %d", pid, WSTOPSIG(status));
        }
        else
        {
            LogError("child other reason quit, pid = %d, signal number = %d", pid, WSTOPSIG(status));
        }
        return bRestart;
    }


    bool Daemon::Start(unsigned int port, unsigned int threadNum)
    {
        string masterPidStr = loadFile2Str(MASTER_PID_FILE);
        int masterPid = atoi(masterPidStr.c_str());
        if(masterPid)
        {
            LogInfo("readlast masterPid[%d]",masterPid);
            if (kill(masterPid, 0) == 0)
            {
                LogError("Another instance exist, ready to quit!");
                return false;
            }

        }

        initAsDaemon();

        char buf[64];
        sprintf(buf, "%d", getpid());
        if (!WriteStr2File(MASTER_PID_FILE,buf ,"w"))
        {
            LogFatal("Write master pid fail!");
        }

        while(true)
        {
            pid_t pid = fork();
            if (0 == pid)// child process do
            {
                signal(SIGUSR1, sigChildHandler);
                signal(SIGPIPE, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTERM, SIG_IGN);
                signal(SIGINT,  SIG_IGN);
                signal(SIGQUIT, SIG_IGN);

                if (!m_pWorker->Init(port, threadNum))
                {	
                    LogError("Worker init  fail!");
                    return false;
                }
                LogInfo("Worker init  ok pid = %d",(int)getpid());

                if (!m_pWorker->Run())
                {
                    LogError("run finish -fail!");
                    return false;
                }
                LogDebug("run finish -ok!");

                if(!m_pWorker->Dispose())
                {
                    LogError("Worker dispose -fail!");
                    return false;
                }
                LogDebug("Worker dispose -ok!");
                exit(0);
            }

            m_nChildPid=pid;
            int status;
            pid = wait(&status);
            if (!isAbnormalExit(pid, status))
            {
                LogInfo("child exit normally! and Daemon exit");
                break;
            }
        }
        return true;
    }


    bool Daemon::Stop()
    {
        string masterPidStr = loadFile2Str(MASTER_PID_FILE);
        int masterPid = atoi(masterPidStr.c_str());
        if(masterPid)
        {
            LogInfo("readlast masterPid[%d]",masterPid);
            if (kill(masterPid, 0) == 0)
            {
                LogInfo("find previous daemon pid= %d, current pid= %d", masterPid, getpid());
                kill(masterPid, SIGTERM);

                int tryTime = 200;		
                while (kill(masterPid, 0) == 0 && --tryTime)
                {
                  sleep(1);			
                }

                if (!tryTime && kill(masterPid, 0) == 0)
                {
                    LogError("Time out shutdown fail!");		
                    return false;
                }

                return true;
            }

        }
        LogInfo("Another instance doesn't exist, ready to quit!");
        return true;
    }

    void Daemon::initAsDaemon()
    {
        if (fork() > 0)
          exit(0);
        setsid();

        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTERM, sigMasterHandler);
        signal(SIGINT,  sigMasterHandler);
        signal(SIGQUIT, sigMasterHandler);
        signal(SIGKILL, sigMasterHandler);
    }

    //主进程接收USR1信号处理函数
    void Daemon::sigMasterHandler(int sig)
    {		
        kill(m_nChildPid,SIGUSR1);
        LogDebug("master = %d sig child =%d!",getpid(),m_nChildPid);

    }

    void Daemon::sigChildHandler(int sig)
    {		
        if (sig == SIGUSR1)
        {
            m_pWorker->CloseServer();
            LogDebug("master = %d signal accept current pid =%d!",getppid(),getpid());
        }

    }

}


