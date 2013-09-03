//**接口函数MT_transform**//

#ifndef MT_TRANSFORM
#define MT_TRANSFORM
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
typedef int HANDLE;
typedef int DWORD;
#define INFINITE 0
#endif

#include <algorithm>
#include <vector>
#include <map>
using namespace std;

///////////////**********************************************************************************//////////////
//类ThreadMananger 为一个简单的线程包装管理组件 其接口介绍详见每个接口函数定义 main.cpp有测试用例；
//对外接口为其所有公有成员函数；另Bind函数是名字空间simpleThread的一个公有接口

//使用方法：      1创建对象    ThreadMananger manager;

//                2创建线程     （1） 标准线程函数方法   
//                              （2） 函数对象方法        A 直接函数对象法
//                                                        B 间接函数对象（绑定线程函数指针，和线程参数）
//                                                        C 无参函数指针方法；

//                3等待线程     （1） 等待单个线程        Wait  方法
//                               (2)  等待多个线程        WaitMultipleThread

//                4ThreadMananger对象销毁后自动关闭线程句柄

//                5对于此类中线程函数返回句柄可作为输入参数调用该类其他函数，但不要用API函数CloseHandle关闭；

//                6关于传入线程的参数最好是简单数据类型 （包括指针，简单的结构体等）否则可能由于拷贝对象影响效率；

//                7关于资源泄露，若用户不停的创建新的线程，并且线程非正常退出会有内存泄露--动态分配的线程参数泄露；
///////////////**********************************************************************************//////////////


namespace simpleThread
{
#ifdef _WIN32
	typedef unsigned (__stdcall* PThreadFunc)(void *param );
#else
	typedef void *(* PThreadFunc)(void* param);
#endif



	//以函数对象方式创建线程时，内部调用的线程函数，它用结构体包装以传递FuncObj类型信息
	//它使用的函数对象需是一个Generator 即不用任何参数就可以调用的函数对象 
	//线程参数由函数对象的成员变量携带,用构造函数初始化该成员变量；
	template <class FuncObj>
	struct InnerThreadFuncCls//**用结构包装以传递类型信息
	{
#ifdef _WIN32
		static unsigned __stdcall InnerThreadFunc(void *pPara) //静态成员函数，线程函数
#else
		static void *InnerThreadFunc(void *pPara)
#endif
		{
			FuncObj* pFuncObj=(FuncObj*)pPara;
			(*pFuncObj)();	
			delete pFuncObj;//清理动态函数对象			
			return 0;
		}
	};



	class threadManager                       //线程管理类；
	{
	public:
		threadManager(){;}
		~threadManager()
		{
			//关闭其管理的每一个线程句柄，（这些线程可能有还在运行的）；
#ifdef _WIN32
			for_each(m_vecHandle.begin(),m_vecHandle.end(),CloseHandle);	
#endif	
		}


		//返回其管理的线程总数
		unsigned int HandleCount(){return m_vecHandle.size();}


		//关闭其管理的每一个线程句柄，清理内部资源，（这些线程可能有还在运行的）；
		void clear()
		{
			//WaitMultipleThread();
#ifdef _WIN32
			for_each(m_vecHandle.begin(),m_vecHandle.end(),CloseHandle);
#endif	
			m_vecHandle.clear();
			//	m_vecPvoid.clear();
		}




		//以_beginthreadex线程函数形式创建线程
		//pFunc指向标准线程函数的指针  unsigned __stdcall ThreadFunc(void *param )
		//pPara线程参数指针           : 此参数指向的对象生存其需长于线程函数，并且注意为多个线程生成恰当的参数对象。
		//return value                : 返回值为零说明创建线程失败，否则返回线程句柄
		HANDLE CreateThread( PThreadFunc pFunc,void *pPara)
		{	
#ifdef _WIN32
			HANDLE handle=(HANDLE)_beginthreadex(NULL,0,pFunc,pPara,0,NULL);
			if (handle==0) //_beginthreadex  failure
				return 0;
			m_vecHandle.push_back(handle);	//加入线程列表 为WaitForMultipleObjects准备	 
			//	m_vecPvoid.push_back(0);    //无线程对象情况；
			return handle;
#else
			pthread_t pt;
			int nErrorCode=pthread_create(&pt,NULL,pFunc,pPara);
			if(nErrorCode!=0)
				return nErrorCode;
			m_vecHandle.push_back(pt);	//加入线程列表 为WaitForMultipleObjects准备	
			return nErrorCode;
#endif


		}

		//等待某个线程执行完毕；
		//hThread线程句柄         : 为0时为默认最后一个加入管理器的线程句柄 
		//dwMilliseconds等待时间  : 单位毫秒，默认值无穷时间
		//return value            : -1句柄无效，其他值 WaitForSingleObject函数的返回值
		DWORD Wait(HANDLE hThread=0,DWORD dwMilliseconds=INFINITE )
		{
			if( hThread==0)//最后一个加入的线程
			{   
				if(!m_vecHandle.empty())
				{
#ifdef _WIN32
					return WaitForSingleObject(m_vecHandle.back(),dwMilliseconds);
#else
					return pthread_join(m_vecHandle.back(),NULL);
#endif

				}
				else
					return -1;
			}
			else
			{
				if (find(m_vecHandle.begin(),m_vecHandle.end(),hThread)==m_vecHandle.end())//不存在此句柄
				{
					return -1;
				}

	#ifdef _WIN32
				return WaitForSingleObject(hThread, dwMilliseconds);
	#else
				return pthread_join(hThread, NULL);
	#endif
			}

		}


		//等待所有线程执行完毕
		//bWaitAll是否所有线程  : 默认值1等待所有线程,0有任何线程结束，此函数返回
		//dwMilliseconds        : 单位毫秒，默认值无穷时间
		//return value          : -1没有任何句柄，其他值 WaitForMultipleObjects函数的返回值
		DWORD  WaitMultipleThread( bool bWaitAll=1,DWORD dwMilliseconds=INFINITE)
		{
			if (m_vecHandle.empty())		
				return -1;	
#ifdef _WIN32
			return WaitForMultipleObjects(m_vecHandle.size(),&m_vecHandle[0],bWaitAll,dwMilliseconds);
#else
			int nErrorcode;
			for (int i=0;i<m_vecHandle.size();++i)
			{
				nErrorcode=pthread_join(m_vecHandle[i], NULL); 
				if (nErrorcode!=0)
					return nErrorcode;	
			}	
			return 0;
#endif
		}



		//以函数对象方式创建线程,或者无参的函数指针；void (*pfunc)(void)
		//它使用的函数对象需是一个Generator 即不用任何参数就可以调用的函数对象 
		//线程参数由函数对象的成员变量携带,用构造函数初始化该成员变量；
		template <class FuncObj>
		HANDLE CreateThread(FuncObj funcObj) //引用传递在指向函数指针时 会有编译问题因此用值传递
		{
			FuncObj* pFuncObj=new FuncObj(funcObj);//需要合适的默认拷贝构造函数
			//于线程函数中DELETE
			HANDLE handle;
#ifdef _WIN32
			handle=(HANDLE)_beginthreadex(NULL,0,InnerThreadFuncCls<FuncObj>::InnerThreadFunc,pFuncObj,0,NULL);
			if (handle==0) //_beginthreadex  failure
				return 0;
			m_vecHandle.push_back(handle);	
			return handle;
#else
			pthread_t pt;
			int nErrorCode=pthread_create(&pt,NULL,InnerThreadFuncCls<FuncObj>::InnerThreadFunc,pFuncObj);
			if(nErrorCode!=0)
				return nErrorCode;
			m_vecHandle.push_back(pt);	//加入线程列表 为WaitForMultipleObjects准备	
			return nErrorCode;
#endif
		}
	private:

#ifdef _WIN32
		vector<HANDLE> m_vecHandle;//线程句柄和函数对象的列表
#else
		vector<pthread_t> m_vecHandle;   //线程句柄和函数对象的列表
#endif

		//	vector<void*> m_vecPvoid;  //保存线程参数使用；
	private:
		threadManager(const threadManager&){;}//禁止拷贝
		threadManager & operator=(const threadManager &){;}//禁止赋值			
	};





	//代理函数对象用于绑定线程函数指针和线程参数
	template<class PUserFunc,class ThreadPara>
	struct FuncObjProxy 
	{		
		FuncObjProxy(PUserFunc pFunc,ThreadPara para):m_pFunc(pFunc),m_para(para)
		{;	}
		void operator()()
		{
			m_pFunc(m_para);

		}
		PUserFunc m_pFunc;
		ThreadPara m_para;

	};

	//绑定方法；返回函数对象
	//pfun 函数指针指向用户线程函数，形式为 typename RetCls (*ThreadFunc)(typename ThreadPara) 
	//实际返回类型RetCls用户获取不到，最好为void (*ThreadFunc)(ThreadPara para)形式
	//refPara为用户线程函数参数的引用
	template<class PUserFunc,class ThreadPara>
	FuncObjProxy<PUserFunc,ThreadPara> Bind(PUserFunc pfun,ThreadPara &refPara)
	{
		return FuncObjProxy<PUserFunc,ThreadPara>(pfun,refPara);
	}


}






#endif
