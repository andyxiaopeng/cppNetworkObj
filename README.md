# cppNetworkObj
高可靠高并发大流量的c++网络服务



服务器的服务相当于有三种：

**阻塞**：像前面说的调用了accept()一直在阻塞，有了连接请求就连上，结束阻塞，没有就一直阻塞

**非阻塞忙轮询**：不阻塞，一直去问有没有需要连接的，有就调用accept()，没有就一直问

**响应式**：别人有连接我再调用accept()——就是多路IO复用或者说多路IO转接

select网络模型参考资料：[C++网络编程select函数原理详解_c++ select-CSDN博客](https://blog.csdn.net/mrqiuwen/article/details/127591210)

> select网络模型基本思路：
>
> select是由内核管理的工具。
>
> 没有加入select模型之前，网络程序需要accept阻塞等待链接才行。在加入select模型之后，由内核的select来管理文件描述符fd（socket）的IO事件。
>
> 若是select检测到fd有IO事件才启用accept来建立连接。这样避免了程序长期被accept阻塞。
>
> 程序设计的思维 由accept阻塞等待链接到来 变成了 先由select检测到客户端的连接请求再启动accept创建fd（socekt）连接。
