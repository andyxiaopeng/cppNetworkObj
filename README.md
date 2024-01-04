# cppNetworkObj
高可靠高并发大流量的c++网络服务

> 启用本项目需要注意修改**ip地址**及**端口号**。
>
> 编译此项目需要注意本项目的文件编码格式为**gbk**，出现乱码等现象需要自行转换编码格式。
>
> 使用visual studio编译程序需要改为**Release**模式

> 经测试：1、本地回环（IP号 127 开头）能达到2Gbps，甚至更多。2、本地路由器局域网（IP号 196 开头），能达到四五百Mbps。3、校园内局域网（IP号 10 开头），能达到70多Mbps。

## 客户端量过大导致网络传输速率下降

1. 客户端由几千升至上万后，服务端能接收那么多的客户端，也能接收那么大量的包，但是服务端没办法给那么多反馈的包。也就是说服务端只优化了吞吐量中的“吞”，却没有优化服务端的“吐”。

## 网络模型--select模型

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

## window网络环境和Unix等网络环境异同

> windows网络编程需要开启**socket的环境**，剩下代码与unix无异。
>
> ```c++
> 	#define  WIN32_LEAN_AND_MEAN // 这个宏尽量避免早期一些依赖库的引用
> 	#define _WINSOCK_DEPRECATED_NO_WARNINGS
> 	#include <Windows.h>
> 	#include <WinSock2.h>
> 	#pragma comment(lib, "ws2_32.lib")
> 	
> _sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");
> 	}
> 	
> 	// 开启socket环境
> 	WORD ver = MAKEWORD(2, 2); // winsocket的版本
> 	WSADATA dat;
> 	WSAStartup(ver, &dat);
> 	// ----------------
> 	
> 	// 关闭socket环境
> 	// 关闭套接字 closesocket
> 	closesocket(_sock);
> 	//清除Windows socket环境
> 	WSACleanup();
> 	
> ```
>
> Window的**socket地址类**与Unix有不同:
>
> _sin.sin_addr.**S_un.S_addr** 和  _sin.sin_addr.**s_addr**
>
> ```c++
> 	sockaddr_in _sin = {};
> 	_sin.sin_family = AF_INET;
> 	_sin.sin_port = htons(port);
> 
> #ifdef _WIN32
> 		// --------- Windows环境下的socket地址类
> 	if (ip)
> 	{
> 		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
> 	}
> 	else
> 	{
> 		_sin.sin_addr.S_un.S_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");
> 	}
> #else
> 		// --------- Unix环境下的socket地址类
> 	if (ip)
> 	{
> 		_sin.sin_addr.s_addr = inet_addr(ip); 
> 	}
> 	else
> 	{
> 		_sin.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY 表示本机所有ip都可以访问。 也可以更换为 inet_addr("127.0.0.1");	
> 	}
> #endif
> 		
> ```
>
> 

## server&client 修改为消息缓冲数据接收模式--粘包\少包\分包

> 解决**粘包\分包**等问题

客户端或者服务端，特别是服务端，如果**消息缓冲区**堆积了大量数据就会使得程序在处理消息的while循环停留很久。一般这种情况多考虑**异步处理（多线程）**。

## c++的标准库：高精度计时器\<chrono>

> <chrono>: 高精度计时器标准库，给c++跨平台提供休眠函数
>
> 类 time_point<high_resolution_clock>
>
> high_resolution_clock: 高频计时器
>
> 

## 标准库的多线程\<thread>

> 创建线程\<thread>、互斥锁\<mutex>(信号量)、条件锁、自旋锁、读写锁、递归锁、原子操作\<atomic>、**RAII**风格语法（自解锁，'lock_guard\<mutex> lg(m)'）

经过测试，频繁使用锁会消耗非常多的资源，还会浪费非常多的时间。

锁的**临界区**扩大会大大减少频繁的上锁、开锁操作，节省了资源，但是又会使得线程操作的粒度变小使得线程的效率变差。需要在实际生成环境考量。

**原子操作**：计算机处理命令时最小的操作单位，原子操作的对象本身其各种方法都是锁定的，是不可分的操作。

## Server端收发数据分离

> 在测试中发现，server收发不分离的话，只要client比较多，然后如果多数量的client发来消息，server有能力接收如此大量的数据，但是因为收发不分离，会导致发送能力跟不上接收能力，从而不能及时给client回馈消息。也会导致server的程序阻塞在发送数据的地方，影响client和server的数据交互。

server收发分离，创建task类，将发送数据的任务从接收数据的线程中剥离，使得额外的cpu资源来发送数据，既不影响server的数据接收，又能榨干cpu资源，使得cpu运行效率更高。

在出现大量发送task任务之后会使得程序占用大量的内存空间，因为每个task都是new出来的。所以需要在程序当中加入**内存池**。

## 内存池

c++ 不像其他代码一样有一个运行环境来管理内存，c++是需要开发者来进行内存管理，所以c++代码更加高效，但是承担的风险也更高。

> 内存管理是为了减少**内存碎片**的产生，使得程序长期、有效的运行。
>
> 减少系统对内存调用的控制管理，在程序开发中多写一些控制管理的代码，使得后续多平台部署程序的运维压力减少，也使得程序在具有更强的平台兼容性。
>
> 内存池是向系统申请一片足够大小的内存空间，由程序自己来控制管理该空间。

### 内存池管理类、内存池类、内存块类

```c++
// 内存池管理类
// 单例模式 - 保证全局有且仅有一个类实例化对象 - 相当于所有方法都是类方法

class MemoryMgr
{
public:
	// 单例
	static MemoryMgr& Instance()
	{ // 单例模式 静态
		static MemoryMgr mgr;
		return mgr;
	}
}
```



### 内存对齐

> 使用 sizeof(void*) 来作为内存对齐的基本单位

## 智能指针\<memory>

> 使得没有内存管理的c++变得更加现代化。在c++的标准库**\<memory>**当中

```c++
#include <memory>

std::shared_ptr<int> a = std::make_shared<int>(10);
std::unique_ptr<int> b = std::make_unique<int>(100);
```

## 对象池

各种类的成员变量、成员函数、构造函数的参数等等都不相同，需要做一个对象池的基类，该基类实现了**可变参数模版**的代码。

> 使用**智能指针**的时候不会触发**对象池**的构造函数。需要对实例化的代码进行修改，不能使用常规的智能指针实例化方法。
>
> 但是使用如下的方式用智能指针来实例化对象的话会触发两次new和两次delete。
>
> new：第一次是对象的new方法，第二是智能智能的new方法。
>
> delete：第一次是对象的析构方法，第二次是智能指针的析构方法。
>
> ```c++
> // std::shared_ptr<ClassA> a0 = std::make_shared<ClassA>(xx); // 常规智能指针实例化方式
> std::shared_ptr<ClassA> a1(new ClassA(xx));
> ```



## 在实际网络环境中会存在死链--心跳包

> 在实际的网络环境中（非本机回环），客户端断开连接后，服务端以为客户端还没断开仍然保持连接，这种连接成为**死链**。
