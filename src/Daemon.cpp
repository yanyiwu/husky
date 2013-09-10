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


//bool SDerivedHandler::Init()
//{
//	return m.init();
//}
//
//bool SDerivedHandler::Dispose()
//{
//	return m.dispose();
//}
//
//void SDerivedHandler::operator()(string &strRec, string &strSnd)
//{
//	m(strRec, strSnd);
//}

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
		fprintf(stderr,"stat dir %s fail,file %s,line %d -- info:%s\n", pchDir,__FILE__,__LINE__,strerror(errno));
		exit(0);
	}

	if(0>chdir(pchDir))
	{

		fprintf(stderr, "work dir %s does not exist\n",pchDir);
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
		fprintf(stderr, "child normal termination, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
		bRestart = false;
	}
	else if (WIFSIGNALED(status)) //signal方式退出
	{
		fprintf(stderr, "abnormal termination, pid = %d, signal number = %d%s\n", pid, WTERMSIG(status),
#ifdef	WCOREDUMP
					WCOREDUMP(status) ? " (core file generated)" : "");
#else
		"");
#endif

		if (WTERMSIG(status) == SIGKILL)
		{
			bRestart = false;
			fprintf(stderr, "has been killed by user ??, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
		}
	}
	else if (WIFSTOPPED(status)) //暂停的子进程退出
	  fprintf(stderr, "child stopped, pid = %d, signal number = %d\n", pid, WSTOPSIG(status));
	else
	  fprintf(stderr, "child other reason quit, pid = %d, signal number = %d\n", pid, WSTOPSIG(status));

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
		printf("readlast %d:masterPid\n",masterPid);
		if (kill(masterPid, 0) == 0)
		{
			printf("Another instance exist, ready to quit!\n");
			return true;
		}

	}

	initAsDaemon();

	sprintf(buf, "%d", getpid());
	if (!WriteBuff2File(strName.c_str(), buf, "w"))
	{
		//log_warn(g_logger, "Write master pid fail!");
		fprintf(stderr, "Write master pid fail!\n");
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
				fprintf(stderr, "Worker init  fail!\n");
				return false;
			}
			fprintf(stderr, "Worker init  ok pid = %d\n",(int)getpid());

			if (!m_pWorker->Run())
			{
				fprintf(stderr, "run finish -fail!\n");
				return false;
			}

			fprintf(stderr, "run finish -ok!\n");

			if(!m_pWorker->Dispose())
			{
				fprintf(stderr, "Worker dispose -fail!\n");
				return false;
			}

			fprintf(stderr, "Worker dispose -ok!\n");
			exit(0);
		}
		m_nChildPid=pid;
		int status;
		pid = wait(&status);
		if (!isAbnormalExit(pid, status))
		{
			fprintf(stderr, "child exit normally!\n");
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
			fprintf(stderr, "find previous daemon pid= %d, current pid= %d\n", masterPid, getpid());
			int tryTime = 200;		
			kill(masterPid, SIGTERM);
			while (kill(masterPid, 0) == 0 && --tryTime)
			  sleep(1);			

			if (!tryTime && kill(masterPid, 0) == 0)
			{
				fprintf(stderr, "Time out shutdown fail!\n");		
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
	fprintf(stderr, "master = %d sig child =%d!\n",getpid(),m_nChildPid);

}

void CDaemon::sigChildHandler(int sig)
{		
	if (sig == SIGUSR1)
	{
		m_pWorker->close();
		fprintf(stderr, "master = %d signal accept current pid =%d!\n",getppid(),getpid());
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


