#include "Daemon.h"

namespace Husky
{
    CWorker *CDaemon::m_pWorker=NULL;
    int CDaemon::m_nChildPid=0;

    CDaemon::CDaemon(void)
    {
    }

    CDaemon::~CDaemon(void)
    {
    }

    CWorkerEx::CWorkerEx(SRequestHandler* pHandler):m_pHandler(pHandler)
    {
    }

    bool CWorkerEx::Init(HIS&his)
    {
        int nPort,nThreadNum;
        HISI i=his.find('n');   
        if(i!=his.end())
          nThreadNum=atoi(i->second.c_str());
        i=his.find('p');   
        if(i!=his.end())
          nPort=atoi(i->second.c_str());
        if(nThreadNum<=0||nPort<=0)
          return false;
        if(!m_pHandler->init())
        {
            return false;
        }

        return m_server.CreateServer(nPort, nThreadNum, m_pHandler);
    }

    bool CWorkerEx::Run()
    {
        return m_server.RunServer();
    }

    bool CWorkerEx::Dispose()
    {
        return m_pHandler->dispose();
    } 

    bool CWorkerEx::close()
    {
        return m_server.CloseServer();
    }


    //读文件第一行：buf ，读入缓冲区，maxCount最大字节数量，mode 文件打开方式，失败返回 0。
    int CDaemon::Read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode)
    {
        FILE* fp = fopen(fileName, mode);
        if (!fp)
          return 0;
        int ret;
        fgets(buf, maxCount, fp) ? ret = 1 : ret = 0;
        fclose(fp);
        return ret;
    }

    //将缓冲区写入文件：buf 缓冲区 ，mode 文件打开方式，失败返回 0。
    int CDaemon::WriteBuff2File(const char* fileName, const char* buf, const char* mode)
    {
        FILE* fp = fopen(fileName, mode);
        if (!fp)
          return 0;
        int n = fprintf(fp, "%s", buf);
        fclose(fp);
        return n;
    }

    //************************************
    // Method:    ParseCmdLine 解析命令行参数，格式为 -x xxx .... 形式
    // FullName:  CDaemon::ParseCmdLine
    // Access:    public 
    // Returns:   int	
    // Qualifier:
    // Parameter: int argc
    // Parameter: char * * argv
    //************************************
    int CDaemon::ParseCmdLine(int argc,char** argv)
    {
        m_pName=argv[0];

        if(argc<2)return 0;
        for (int i=1;i<argc;++i)
        {
            if (argv[i][0]!='-')
              continue;

            if (i==argc-1)//后面还有
              break;

            if (argv[i+1][0]=='-')
            {
                m_hisOptVal[argv[i][1]]="";
                continue;
            }

            m_hisOptVal[argv[i][1]]=argv[i+1];
            ++i;
        }	

        const char* pchDir;
        HISI it=m_hisOptVal.find('d');
        if(it!=m_hisOptVal.end())
          pchDir=it->second.c_str();
        else
          pchDir=".";

        struct stat st;	
        if(stat(pchDir,&st)<0)
        {
            LogError("stat dir %s fail, -- info:%s", pchDir,strerror(errno));
            exit(0);
        }

        if(0>chdir(pchDir))
        {

            LogError("work dir %s does not exist",pchDir);
            exit(0);
        }

        char buf[128];
        getcwd(buf,128);	
        m_runPath = buf;
        m_runPath+='/';

        return m_hisOptVal.size();
    }

    /* modify from book apue
     * 为防止子进程意外死亡, 主进程会重新生成子进程.
     * 除非:1.子进程以EXIT方式退出. 2. kill -9 杀死该进程
     */
    bool isAbnormalExit(int pid, int status)
    {
        bool bRestart = true;
        if (WIFEXITED(status)) //exit()or return 方式退出
        {
            LogError("child normal termination, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
            bRestart = false;
        }
        else if (WIFSIGNALED(status)) //signal方式退出
        {
            LogError("abnormal termination, pid = %d, signal number = %d%s", pid, WTERMSIG(status,
#ifdef	WCOREDUMP
                        WCOREDUMP(status) ? " (core file generated)" : 
#endif
            "")); 

            if (WTERMSIG(status) == SIGKILL)
            {
                bRestart = false;
                LogError("has been killed by user ??, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
            }
        }
        else if (WIFSTOPPED(status)) //暂停的子进程退出
          LogError("child stopped, pid = %d, signal number = %d", pid, WSTOPSIG(status));
        else
          LogError("child other reason quit, pid = %d, signal number = %d", pid, WSTOPSIG(status));

        return bRestart;
    }

    bool CDaemon::Start()
    {
        //从文件获取正在运行的daemon的pid
        char buf[640];
        int masterPid;

        string strName = m_runPath + MASTER_PID_FILE ;
        strName+=m_pName;
        if ( 0<Read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0)
        {
            LogInfo("readlast %d:masterPid",masterPid);
            if (kill(masterPid, 0) == 0)
            {
                LogInfo("Another instance exist, ready to quit!");
                return true;
            }

        }

        initAsDaemon();

        sprintf(buf, "%d", getpid());
        if (!WriteBuff2File(strName.c_str(), buf, "w"))
        {
            //log_warn(g_logger, "Write master pid fail!");
            LogError("Write master pid fail!");
        }

        while(true)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                //子进程做的事情

                signal(SIGUSR1, sigChildHandler);
                signal(SIGPIPE, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTERM, SIG_IGN);
                signal(SIGINT,  SIG_IGN);
                signal(SIGQUIT, SIG_IGN);

                if (!m_pWorker->Init(m_hisOptVal))
                {	
                    LogError("Worker init  fail!");
                    return false;
                }
                LogError("Worker init  ok pid = %d",(int)getpid());

                if (!m_pWorker->Run())
                {
                    LogError("run finish -fail!");
                    return false;
                }

                LogError("run finish -ok!");

                if(!m_pWorker->Dispose())
                {
                    LogError("Worker dispose -fail!");
                    return false;
                }

                LogError("Worker dispose -ok!");
                exit(0);
            }
            m_nChildPid=pid;
            int status;
            pid = wait(&status);
            if (!isAbnormalExit(pid, status))
            {
                LogError("child exit normally!");
                break;
            }
        }
        return true;
    }


    bool CDaemon::Stop()
    {
        char buf[640];
        int masterPid;

        string strName = m_runPath + MASTER_PID_FILE ;
        strName+=m_pName;

        if ( 0<Read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0)
        {
            if (kill(masterPid, 0) == 0)
            {
                LogInfo("find previous daemon pid= %d, current pid= %d", masterPid, getpid());
                int tryTime = 200;		
                kill(masterPid, SIGTERM);
                while (kill(masterPid, 0) == 0 && --tryTime)
                  sleep(1);			

                if (!tryTime && kill(masterPid, 0) == 0)
                {
                    LogError("Time out shutdown fail!");		
                    return false	;
                }

                return true;
            }

        }

        printf("Another instance doesn't exist, ready to quit!\n");
        return true;
    }
    //初始化DAEMON
    void CDaemon::initAsDaemon()
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
    void CDaemon::sigMasterHandler(int sig)
    {		
        kill(m_nChildPid,SIGUSR1);
        LogError("master = %d sig child =%d!",getpid(),m_nChildPid);

    }

    void CDaemon::sigChildHandler(int sig)
    {		
        if (sig == SIGUSR1)
        {
            m_pWorker->close();
            LogError("master = %d signal accept current pid =%d!",getppid(),getpid());
        }

    }


    bool CDaemon::Run(int argc,char** argv)
    {
        ParseCmdLine(argc,argv);
        char*p=strrchr(argv[0],'/');
        p!=NULL?p=p+1:
            p=argv[0];
        m_pName=p;


        HISI i=m_hisOptVal.find('k');
        if (i!=m_hisOptVal.end())
        {
            if (i->second=="start")
            {
                return Start();
            }
            else
            {
                return Stop();
            }
        }
        return true;

    }
}


