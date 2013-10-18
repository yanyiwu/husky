#Husky 简易C++的HTTP服务框架

主要模块：

+ ServerFrame
+ Daemon

ServerFrame主要是一个http服务的socket通信框架，这个ServerFrame需要传入IRequestHandler这个接口类。   
当每次有http请求进来时，会调用IRequestHandler这个类的相关函数进行处理。   

Daemon是一个守护进程，代码是根据经典书籍APUE的代码修改而成。    
逻辑上很简单，就是每次运行的时候会有一个主进程和子进程，主要的逻辑都在子进程中实现，如果子进程意外退出，则主进程会自动重新启动子进程， 从而实现“守护”这个功能。
