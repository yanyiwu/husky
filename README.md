#Husky 简易C++的HTTP服务框架

## 主要模块：

### Daemon.cpp/h 

守护进程的模式，这个服务启动之后有两个进程，父进程和子进程。
父进程作为子进程的守护进程，主要的代码逻辑都是在子进程里面进行。
父进程的主要职责就是启动子进程，然后等待。
然后在等待的过程中，子进程异常退出了，就会继续启动子进程。
如果子进程是正常退出，则父进程也一起退出。

### ServerFrame.cpp/h

ServerFrame主要是一个http服务的socket通信框架，这个ServerFrame需要传入IRequestHandler这个接口类。   
当每次有http请求进来时，会调用IRequestHandler这个类的相关函数进行处理。   

### HttpReqInfo.cpp/h 

http参数的解析

### globals.h

定义一些文件路径和参数配置

