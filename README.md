#Husky 基于epoll的http服务框架

## 概况

是一个简单的http服务框架，只需要继承一个纯虚类并实现`do_GET`函数就可以处理http的get请求。

网络相关的使用`epoll`来处理并发请求。

单元测试使用`google test`

对于http请求参数的解析暂时写的比较糙，等有时间了准备使用`http_parser`开源库来进行。

所有代码全写成`hpp`模式，使用该项目只需要`#include` 而不需要链接。

## 客服

wuyanyi09@gmail.com

