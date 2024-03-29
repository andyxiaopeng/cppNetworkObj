# cppNetworkObj
高可靠高并发大流量的c++网络服务

> 启用本项目需要注意修改**ip地址**及**端口号**。
>
> 编译此项目需要注意本项目的文件编码格式为**gbk**，出现乱码等现象需要自行转换编码格式。
>

> 经测试：1、本地回环（IP号 127 开头）能达到2Gbps，甚至更多。2、本地路由器局域网（IP号 196 开头），能达到四五百Mbps，极限大概是1Gbps（因为主机网口是千兆网口，而千兆以太网就是1Gbps）。3、校园内局域网（IP号 10 开头），能达到70多Mbps。

## Socket网络编程

> Linux和windows的socket导入的包不相同，且开启socket网络环境的语句也不相同。

- socket网络环境的开启和关闭

  - 开启

    ```c++
    #ifdef _WIN32
    		//启动Windows socket 2.x环境
    		WORD ver = MAKEWORD(2, 2);
    		WSADATA dat;
    		WSAStartup(ver, &dat);
    #endif
    
    #ifndef _WIN32
    		//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    		//	return (1);
    		//忽略异常信号，默认情况会导致进程终止
    		signal(SIGPIPE, SIG_IGN);
    #endif
    ```

  - 关闭

    ```c++
    #ifdef _WIN32
    		//清除Windows socket环境
    		WSACleanup();
    #endif
    ```

- 服务端

  1. 创建socket

     ```c++
     SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     ```

  2. 绑定端口bing

     ```c++
         // 2.1 bind 绑定用于接受客户端连接的网络端口
     
     	// 创建描述地址端口信息的对象   Linux的写法
         sockaddr_in _sin = {};
         _sin.sin_family = AF_INET;
         _sin.sin_port = htons(4567);//host to net unsigned short
         // _sin.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY 代表本机所有网口
     	const char* ip = "192.168.1.1"
     #ifdef _WIN32
     		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
     #else
     		_sin.sin_addr.s_addr = inet_addr(ip);
     #endif
     
         // 2.2
         if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
         {
             printf("错误,绑定网络端口失败...\n");
         }
         else {
             printf("绑定网络端口成功...\n");
         }
     ```

  3. 监听端口listen

     ```c++
         // 3 listen 监听网络端口
         if (SOCKET_ERROR == listen(_sock, 64))
         {
             printf("错误,监听网络端口失败...\n");
         }
         else {
             printf("监听网络端口成功...\n");
         }
     ```

  4. 等待连接accept

     accept需要与网络IO模型相配合进行业务逻辑程序编写。

     ```c++
     // 4 accept 等待接受客户端连接
     sockaddr_in clientAddr = {};
     int nAddrLen = sizeof(sockaddr_in);
     SOCKET _cSock = INVALID_SOCKET;
     _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
     ```

  5. 循环交互：

     循环交互则是 发送数据 和 接收数据 ，两者需要结合IO模型来进行程序编写。

     接收数据：

     ```c++
     g_nLen = (int)recv(cSock, g_szBUff, 4096, 0);
     ```

     发送数据：

     ```c++
     int nLen = (int)send(cSock, g_szBUff, g_nLen, 0);
     ```

  6. 关闭socket

     ```c++
     #ifdef _WIN32
     		int ret = closesocket(sockfd);
     #else
     		int ret = close(sockfd);
     #endif
     ```

- 客户端

  1. 创建socket

     ```c++
     SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     ```

  2. 连接服务器connect

     ```c++
     // 2 连接服务器 connect
     	const char* ip = "192.168.1.1";
         unsigned short port = 4567;
     
         sockaddr_in _sin = {};
         _sin.sin_family = AF_INET;
         _sin.sin_port = htons(port);
     #ifdef _WIN32
         _sin.sin_addr.S_un.S_addr = inet_addr(ip);
     #else
         _sin.sin_addr.s_addr = inet_addr(ip);
     #endif
     
         int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
     ```

     

  3. 循环交互

     循环交互则是 发送数据 和 接收数据 ，两者需要结合IO模型来进行程序编写。

     接收数据：

     ```c++
     g_nLen = (int)recv(cSock, g_szBUff, 4096, 0);
     ```

     发送数据：

     ```c++
     int nLen = (int)send(cSock, g_szBUff, g_nLen, 0);
     ```

  4. 关闭socket

     ```c++
     #ifdef _WIN32
     		int ret = closesocket(sockfd);
     #else
     		int ret = close(sockfd);
     #endif
     ```

     

## 五种网络IO模型

> IO就是输入和输出。
>
> 同步与异步：线程之间的关系，两线程之间要么是同步，要么是异步。（join 和 detach）
>
> 阻塞与非阻塞：线程内的关系，在同一个线程内，某个时刻该线程处于阻塞或者处于非阻塞状态。阻塞调用是指在调用结果返回之前，当前线程会被挂起。非阻塞调用是指在不能立刻得到结果之前，该调用不会阻塞当前线程。



五种网络IO模型分别是：1）阻塞式IO模型。2）非阻塞式IO模型。3）IO多路复用。4）信号驱动IO。5）异步IO模型

1. 阻塞式IO模型

   

2. 非阻塞IO模型

## 客户端量过大导致网络传输速率下降

1. 客户端由几千升至上万后，服务端能接收那么多的客户端，也能接收那么大量的包，但是服务端没办法给那么多反馈的包。也就是说服务端只优化了吞吐量中的“吞”，却没有优化服务端的“吐”。

## 网络模型--select模型

> 有SELECT、IOCP、EPOLL这三种IO复用的网络模型。
>
> [Select、Poll、Epoll的使用和区别，多种IO的区别_io多路复用,epoll和select的区别-CSDN博客](https://blog.csdn.net/iuu77/article/details/129836824)
>
> [c++——iocp模型-CSDN博客](https://blog.csdn.net/www_dong/article/details/125667928)
>
> select网络模型并不是某个平台、某个系统下独有的，select模型和IOcp和epoll不同，select模型几乎在所有平台、系统都有相类似的实现，是一个较为古老的模型。

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

## select网络模型优化 - 异步发送数据

> 在创建socket的时候，不进行非阻塞的设置。（但实际上是可以将socket设置位非阻塞模式）
>
> 使用阻塞模式send的意义是：随时可写、控制收发简单。
>
> select函数的三个参数分别是检查socket的可读、可写、异常。

在阻塞的socket下，不考虑是否可以写入数据就直接调用send来发送数据，如果当前send不满足发送数据的条件则会陷入阻塞从而使得发送效率降低。

在实际的网络环境当中，有些客户端没有断开连接，并且能从客户端发数据到服务端，但是该客户端不能接收服务端发送的数据，从而使得服务端对该客户端的socket下的send数据发送操作阻塞，进而使得该线程阻塞，如果该线程下存在多个客户端，会导致该线程下服务端所连接所有客户端的通信都失效。

如果将阻塞的send数据发送函数改成异步发送的话，需要根据不同的业务来对发送不成功的数据进行缓存、备份、重发等处理。

---

此处优化的 **异步数据发送** 是将发送数据分为两个部分：

1. 数据发送写入发送缓冲区
2. select网络模型调用select函数轮询的时候，执行数据发送操作，该操作将发送缓冲区的数据发送对所对应的客户端。

将发送操作拆分为以上两个部分，使得发送数据不会陷入send函数的阻塞当中。

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
>
> 粘包现象只会在Tcp协议中出现，在Udp协议中永远不会出现。
>
> 所谓粘包就说接收方不知道消息之间的界限，不知道一次性提取多少字节的数据。
>
> 而Tcp协议如何造成粘包现象的出现，主要是因为Tcp协议为了提高传输效率，发送方往往需要收集到足够多的数据才凑成一个Tcp段发送，这样接收方就会受到了粘包数据。

----

**两种粘包的情况：**

1. 发送端需要等缓冲区满了才发出去，造成粘包（发送数据时间间隔很短，数据很小，数据就会合到一起，产生粘包）
2. 接收方不及时处理缓冲区的包，造成多个包接收（客户端发送了一段数据，服务端只接受了一小部分，服务端下次再收的时候还是从缓冲区拿走上次遗留的数据，产生粘包）

---

客户端或者服务端，特别是服务端，如果**消息缓冲区**（此处的消息缓冲区是指操作系统给网络io的消息缓冲区）堆积了大量数据就会使得程序在处理消息的while循环停留很久。一般这种情况多考虑**异步处理（多线程）**。使用多线程来创建**收发分离**的环境，其次使用缓冲区来接收数据。

使用自建缓冲区做到**定时定量**发送数据。

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

锁的**临界区**扩大会大大减少频繁的上锁、开锁操作，节省了资源，但是又会使得线程操作的粒度变小使得线程的效率变差。需要在实际生产环境考量。

**原子操作**：计算机处理命令时最小的操作单位，原子操作的对象本身其各种方法都是锁定的，是不可分的操作。

## 锁\<mutex>



## 阻塞等待-条件变量\<condition_variable>

> 程序使用condition_variable （条件变量） 来实现性能更好、更安全的信号量。
>
> 信号量主要是两个功能：1、使得线程陷入阻塞等待。2、唤醒阻塞的线程。
>
> 使用信号量来保障每个线程的安全退出，使得退出流程的各个步骤安全可控。（退出流程不可控的话，在某些指针、对象释放之后，还会被调用的可能，从而引发内存泄漏。）

```c++
#include <condition_variable>

std::condition_variable cv;

std::mutex _mutex;
std::unique_lock<std::mutex> lock(_mutex); // 独占锁 比 lock_guard 功能更多一点

cv.wait(lock);//参数需要传入一个锁，阻塞等待
cv.notify_one();// 唤醒
```

```c++
cv.wait(lock,()[] ->ret {}); //第二个参数是传入一个lambda表达式；这个向wait函数提供一个解除等待的必要条件

//例如：
int aa = 0;
cv.wait(lock,[](aa) ->bool{
	return aa >= 0;
});
```



### 虚假唤醒

> 根本没有任何等待，但是已经调用了唤醒。
>
> 无效的唤醒可能会导致后续等待陷入死循环。使得等待永远不可能得到唤醒。

### 线程优化

> 

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

> 使用 sizeof(void*) 来作为内存对齐的基本单位。
>
> 然后通过在类（结构体）中填充不需要的char变量来填充对象的内存占用空间，使得在程序在多种系统环境中都能稳定运行。



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

## 虚函数、纯虚函数、接口类

1. 接口类

   - 接口类成员方法全是纯虚函数，需要被继承且重写成员方法才有效
   - 接口类的成员方法也可以叫做 事件

2. 纯虚函数

   - 需要在方法前面加上 " virtual  " 来修饰该方法
   - 纯虚函数在声明方法后需要对使该方法等于0
   - 子类想继承具有纯虚函数的父类，则必须重写该成员方法

3. 虚函数

   - 需要在方法前面加上 " virtual  " 来修饰该方法

   ---
   
   - **虚析构函数**：为什么一般来说类的析构函数需要指定为虚函数呢？
     - 由于类的多态性，基类指针可以指向派生类的对象，如果删除（delete）该基类的指针，就会调用该指针指向的派生类析构函数，而派生类的析构函数又自动调用基类的析构函数，这样整个派生类的对象完全被释放。
     - 如果析构函数不被声明成虚函数，则**编译器实施静态绑定**，在删除基类指针时，只会调用基类的析构函数而不调用派生类析构函数，这样就会造成派生类对象析构不完全，造成内存泄漏。
     - 所以将析构函数声明为虚函数是十分必要的。在实现多态时，当用基类操作派生类，在析构时防止只析构基类而不析构派生类的状况发生，要将基类的析构函数声明为虚函数。
   

## 初始化列表

> 初始化列表，成员变量初始化的地方，**只有**在这里进行的操作才能叫做初始化。
>
> 浅显来看类似于链表阶段的Init()的构造函数是相当于用了赋值操作**在函数体内**进行的对变量的copy，而初始化列表是在**定义阶段**进行的初始化。

Date类进行学习，以**冒号 ":"** 开始进行第一个成员变量的初始化操作，以**逗号 ","** 分隔接下来剩余的成员变量，最后一个不加分号，每个成员变量后面跟着**用括号括起来的**用来进行初始化操作的初始值或者表达式，这就是初始化列表的模样。

```c++
// 初始化列表
Date::Date(int year, int month, int day)
	:_year(year)
	, _month(month)
	, _day(day)
{}
```

---

特性：

1. 由于是在定义阶段进行的初始化，所以，**只能在定义阶段进行的初始化类型**便只能在初始化列表中进行初始化。如：

   - const对象
   - 引用对象
   - 无默认构造函数的自定义类型

2. **初始化顺序**不是跟着初始化列表走的，而是跟着类中成员变量的**声明顺序**进行的初始化：

   ```c++
   // 错误示例
   class A
   {
   public:
   	A(int a)
   		:_a1(a)
   		,_a2(_a1)  // 在成员变量声明的顺序中，_a2是在_a1前声明，故此这样写会引发错误。
   	{}
   private:
   	int _a2;
   	int _a1;
       
   // 正确示例
       class A
   {
   public:
   	A(int a)
   		:_a2(a)
   		,_a1(_a2)
   	{}
   private:
   	int _a2;
   	int _a1;
   ```

   

## 移动语义：右值引用、移动语义构造函数

> **移动语义**是C++11引入的一项重要特性，它允许在对象间传递资源的所有权，而不进行深层次的拷贝。这样可以提高性能，特别是在涉及动态内存管理的情况下。移动语义的核心是引入了**右值引用**和**移动构造函数**。

- **左值**：左值表示一个**具体的内存位置**，**可以取地址**。它通常是一个具名的变量或对象。等号左边的都是左值，也有左值在等号右边。变量、数组元素、对象成员等都是左值。

- **右值**： 右值表示一个临时的数值，通常是在表达式求值后产生的。右值**没有明确的内存地址**，**不能被取地址**。右值肯定是在等号右边。字面常量、临时对象、表达式的结果等

- **右值引用**：

  - 右值引用是一种新的引用类型，用于表示对右值（临时对象或表达式的结果）的引用。右值引用的语法使用双&&符号。

  - ```c++
    int&& x = 42;  // x是一个对右值的引用
    ```

- **移动语义构造函数**：

  - 移动构造函数是一个特殊的构造函数，用于接受右值引用参数并“窃取”其资源，而不是进行深层次的拷贝。移动构造函数的目的是提高性能，尤其是在涉及大型数据结构或动态内存分配时。

  - ```c++
    class MyString {
    public:
        // 移动构造函数
        MyString(MyString&& other) noexcept {
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;  // 窃取资源，避免资源重复释放
            other.size_ = 0;
        }
    
    private:
        char* data_;
        size_t size_;
    };
    ```

- **例程**：

  `MyString`类包含了移动构造函数、移动赋值运算符以及析构函数。在`main`函数中，通过`std::move`将左值转为右值，从而调用移动构造函数和移动赋值运算符，避免了不必要的深层次拷贝。这有助于提高程序的性能，尤其是在处理大型数据结构时。

  ```c++
  #include <iostream>
  #include <utility>
  
  class MyString {
  public:
      // 移动构造函数
      MyString(MyString&& other) noexcept {
          data_ = other.data_;
          size_ = other.size_;
          other.data_ = nullptr;  // 窃取资源，避免资源重复释放
          other.size_ = 0;
      }
  
      // 构造函数
      MyString(const char* str) {
          size_ = std::strlen(str);
          data_ = new char[size_ + 1];
          std::strcpy(data_, str);
      }
  
      // 移动赋值运算符
      MyString& operator=(MyString&& other) noexcept {
          if (this != &other) {
              delete[] data_;  // 释放当前对象的资源
              data_ = other.data_;
              size_ = other.size_;
              other.data_ = nullptr;  // 窃取资源
              other.size_ = 0;
          }
          return *this;
      }
  
      // 析构函数
      ~MyString() {
          delete[] data_;
      }
  
  private:
      char* data_;
      size_t size_;
  };
  
  int main() {
      MyString str1 = "Hello";
      MyString str2 = std::move(str1);  // 使用std::move将左值转为右值
      MyString str3 = "World";
  
      str1 = std::move(str3);  // 移动赋值运算符
  
      return 0;
  }
  ```

  

## \<functional> c++11匿名表达式：代替c风格的指针函数、函数指针

> function      	====   函数指针
>
> lambda      	 ====	匿名函数

```c++
#include <functional>

int funa(int a, int b)
{
	printf("funca\n");
    return 0;
}

int main()
{
    std::function< int(int, int)> call = funa;
    int n = call(0,1);
    return 0;
}

```

```c++
//lambda 表达式  匿名函数
[ caputure ] ( params ) opt -> ret { body; };
[ 外部变量捕获列表 ] ( 参数列表 ) 特殊操作符 -> 返回值类型 { 函数体; };

capture：外部变量捕获列表, lambda表达式的捕获列表精细控制了lambda表达式能够访问的外部变量，以及如何访问这些变量
    1) []  不捕获任何变量
    2) [&] 捕获外部作用域所有变量，并作为引用在函数体内使用（引用捕获）
    3) [=] 捕获外部作用域所有变量，并作为副本在函数体内使用（按值捕获）
    4) [=, &foo] 捕获外部作用域所有变量，并按引用捕获foo变量
    5) [bar] 按值捕获bar变量，同时不捕获其他变量
    5) [this] 捕获当前类的this指针，让lambda表达式拥有和当前类成员函数同样的访问权限。如果已经使用&或者=，默认包含此选项。
    
opt：函数选项
    
```

```c++
#include <functional>

int main()
{
    std::function<int(char)> call;
    
	int n = 5;
    
    call = [ n /* 外部变量捕获列表 */](char c/* 参数列表 */) -> int /* 返回值类型 */
    {
        // 函数体        
        printf("func Lambda\n");
        
        printf("%d\n",n)
            
        return 2;
    }; // 函数指针call指向一个lambda匿名函数
    
    char c = 'C';
    int r = call(c);
    return 0;
}
```



## 在实际网络环境中会存在死链 -- 心跳检测

> 在实际的网络环境中（非本机回环），客户端断开连接后，服务端以为客户端还没断开仍然保持连接，这种连接成为**死链**。

在服务端设计每个连接的死亡倒计时，而客户端需要发送心跳包来告知服务端自己存活。如果服务端在倒计时结束的时候还没有受到客户端的心跳包则默认客户端死亡，主动断开连接。如果在倒计时期间受到客户端的心跳包则重置倒计时的时间。

> 还有一些偷懒的做法，就是客户端任何的消息都可以认为是附带心跳的作用。使得服务端重置死亡倒计时。

## 代码优化

> 代码优化：包括性能优化、代码结构优化

### 性能优化

包括上述的网络模型修改、读写多线程分离、内存管理优化、智能指针、对象管理等等

- 类的内联函数，使用inline关键字来修饰成员方法就变成了内联函数，内联函数比普通成员方法效果能好一点。

### 代码结构优化

将各部分代码分门别类放到各个单独文件当中，建立项目的统一头文件，建立好各个文件的引用关系。

### 类方法、函数编写的优化 - 防御式编程

对于外来传入的任何参数都进行判断处理，防止传入错误的参数，使得程序的防御力度最大化。

## 消息缓存区管理类 - 分离消息缓冲区

> 随着程序扩大，逻辑结果更加复杂，消息缓存区可以从其他代码中剥离开，形成一个单独的消息缓存区管理类。
>
> 可以初始化的时候定义任意大小的缓冲区，满足不同业务的需求。
>
> 

在该消息缓存区管理类中，只有二进制的长度概念（以字节为单位），不对任何类的长度、数据的长度等进行管理和考虑，保持程序的**高内聚、低耦合**特性。

## 程序日志记录

> 将程序的控制台输入内容改为使用日志记录控制程序向指定文件写入信息。
>
> 在实际生产环境中，也可以选择向数据库里写数据，根据业务需求和程序运行的环境来选择。

## 文件读写

> c++可以使用c++的文件读写，也可以使用c的文件读写。

### 日志记录时间

> 基本所有日志都需要在前面或者后面根据自己的格式来插入该条日志记录的时间，或者是操作发生的时间。
>
> c++程序有自己的时间生成库 \<ctime> 
>
> 1. 使用\<ctime>库的ctime()函数 可以创建格式化日期信息。
> 2. std::tm*指针配合std::gmtime() 来创建一个对象的指针，该对象指针可以调用获取当前时间信息的方法。
>

### 错误信息提示

> Linux和Windos会有不同的错误提示接口，可以通过宏来设定不同的错误提示。



## 分离网络环境的启动和关闭

> 在windows系统、Linux系统都有属于自己的网络环境启动和关闭的函数。一般来说，网络环境的启动和关闭需要保持与程序的逻辑分离，由单独的程序来控制网络环境的启动和关闭。

编写一个单独的类来控制网络环境的启动和关闭。

使用单例模式能保证整个程序只启动一次网络环境和只关闭一次网络环境。

## **字节流**传输网络数据

> 使用字节流传输网络数据
>
> 设计与解析字节流消息协议
>
> 评判 结构体 与 字节流 以及其他的结构化字符串协议的优劣。
>

---

应用**字节流**最重要的要求：

1. 收发双方保证写和读的**大小一致**
2. 收发双方保证写和读的**顺序一致**

---

在网络通讯中，网络传输的双方都是c++编写的程序，使用**结构体**来传输数据非常便利。但是服务端提供接受信息的服务，客户端可以是多种形式的程序，如：java、unity游戏程序、unreal游戏程序、coco游戏程序、c#等等多种形式。因此对于这种情况，使用**字节流**来传输数据就能适配多种平台的前后端网络通讯。各种语言都能自己解析字节流。

其次，例如同样是c++语言编写的代码，如果是按照结构体类型来接受数据的话，但是收发两方的编译的指令位宽不同，会导致同一个类型的变量、结构会占用不一样的内存空间，size_t在32位编译中会占用4个字节，但是它在64位编译中会占用8字节。在c\c++中有一个库叫\<cstdint>用来提供各种位宽的int类型。

---

1. 结构体：
   - 使用结构体的标志就是，开发的程序需要将指针强转为对应的结构体指针。

---

字节流写入的方式：

- 写一个模板类型的基础方法，不同位宽的写入方法都可以基于模板类型方法来开发。
- 使用特定长度的int类型来写入。
- **数组**的写入需要用模板才能实现。写入数组长度需要在开头使用特定的字节（如无符号的32位来保存数组的元素长度）来保存数组元素个数信息。在写入n个元素到数组缓冲区的时候，需要先判断写入缓冲区是否存在n个元素的长度空间。

---

字节流读取的方式：

- 写一个模板类型的基础方法，不同位宽的读取方法都可以基于模板类型方法来开发。
- 在读取数据的方法中无法判断的一点是：读取的长度没有超出读取缓冲区的总长度，但是不能保证的是里面是否有足够长度的被读数据。例如：读取缓冲区长度是200个字节，缓存区内只有80个字节的数据长度，已经读了76个字节，还剩4个字节数据，但是此时想要读取8字节的数据长度，这样的读取操作并没有超出缓冲区的总长度，但是超出了实际数据的长度。
- **数组**的读取需要模板才能实现。读取数组的长度需要在开头使用特定的字节（如无符号的32位来保存数组的元素长度）来保存数组元素个数信息。在读取数组元素的时候，需要先读取数组元素个数n再判断接收数组元素的缓冲区长度是否足够容纳n个元素，再真正的开始读取。

## 动态库、静态库

> 动态库、静态库都是二进制可执行文件
>
> 在unity、cocos等平台或者跨语言都可以将c++程序编译为动态库、静态库然后根据平台的规则做成可用的插件。

- **动态库**：其他程序需要使用动态库的方法，则在程序的中指明动态库的位置，然后在程序运行过程中动态调用相应的方法。动态链接库也不像普通程序一样需要一个main入口函数。
- **静态库**：其他程序需要使用静态库的方法，则在程序中引入静态库，然后该程序在编译的时候，编译器会将静态库跟当前程序一起编译，最终形成一个可执行文件。不需要像普通程序一样设置一个main入口函数。
- 各平台的        动态库、静态库    对应名称:
  - windows:    .dll .lib
  - Linux:          .so  .a
  - Android:     .so   .a
  - osx:             .bundle
  - MacOS:       .dylib .a
  - IOS:             .a

## 批量脚本命令 - 程序控制

> 程序开发完成之后，编译成为方便使用的可执行文件，而有些参数是根据不同的条件需要进行适当的更改的，比如网络的IP、端口，输入输出文件的路径等等信息，为了更改这些信息再重新编译程序是非常低效的行为，通过将这些可变的信息改成外部传入数值的变量，通过批处理脚本命令来传入必要的数据即可实现上述功能。

- windows: 				**bat**
- Linux\Unix\Mac:     **shell**

---

- c++的main函数有两个参数分别代表了 参数的个数 和 包含传入参数的char数组 。

```c++
int main(int argc, char* args[]){
    // argc 表示传入参数的个数
    // args 是传入参数的存储数组
    // 默认情况下，args[0]既第一个参数是程序启动的路径，而且自己传入的参数都是从第二个数组元素args[1]开始存储。
    
	for(int n = 0; n < argc; n++){
		print("%s\n",args[n]);
	}
    
    // 类型转换示例
    int pa = atoi(args[3]); // 将char类型的元素转换为int类型
    
}
```

- 程序参数 安全性 

  - 参数的数量

    传入参数的数量少于程序所需要的数量，造成程序的崩溃。

  - 参数的类型

    传入参数的类型不符合程序所要求的类型，造成程序的崩溃或者程序运行逻辑出错。

  如何避免上述问题的出现，确保程序参数的安全执行？

  1. 判断当前元素的索引值是否超出传入参数的数量。（数组越界访问问题）
  2. 尝试类型转换是否成功，如果类型转换失败，说明传入的参数类型与实际所需的参数类型不符。

---

- Windows批处理脚本 .bat

  ```bash
  :: 在windows的bat文件中，表示注释的符号有两个，分别是 :: 和 rem
  
  :: any是字符串，但是不需要加双引号，直接默认为字符串，但是也可以加上双引号。
  rem 下面两种方式都是设置变量的方法，但是第二种在前面加@符号，可以阻止该行语句在控制台输出，因为bat语句默认在控制台显示输出。
  :: 在脚本最前面也可以写  echo off 这个命令，从而实现不显式输出命令的效果。
  
  set Ip=any
  :: set Ip="any"
  @set port=4567
  
  :: 执行server程序，使用空格将两个变量分开按顺序传入server程序。
  :: %vaeriable%，使用一对百分号将变量围起来就表示取该变量的值。否则bat脚本将变量识别为一个数值是变量名字的字符串。
  server %Ip% %port%
  
  :: pause是将脚本暂停在此处，作用是上述server程序运行结束之后，可以将控制台暂停在此处，方便查看server程序输出的信息。
  @pause
  ```

  ```bash
  @echo off
  
  :: 这是将参数人为的组成一个key-value的形式
  set cmd="Ip=any"
  set cmd=%cmd% port=4567
  
  server %cmd%
  
  :: pause是将脚本暂停在此处，作用是上述server程序运行结束之后，可以将控制台暂停在此处，方便查看server程序输出的信息。
  pause
  ```

- Linux\Unix 批处理脚本 .sh

  ```shell
  # 在windows的bat文件中，表示注释的符号是 #
  
  # any是字符串，但是不需要加双引号，直接默认为字符串
  # 下面是设置变量的方法,直接对变量进行赋值即可。
  Ip="any"
  port=4567
  
  # 执行server程序，使用空格将两个变量分开按顺序传入server程序。
  # 在shell中，使用变量需要在变量前加上$符号。
  ./server $Ip $port
  
  read -p ".. press any key to exit .." var
  ```

  ```shell
  # 这是将参数人为的组成一个key-value的形式
  cmd="Ip=any"
  cmd=$cmd" port=4567"
  
  server $cmd
  
  read -p ".. press any key to exit .." var
  ```
  
  

## Linux的最大文件数量限制 - FD_SETSIZE - 文件描述符的数量

> FD_SETSIZE 是 文件描述符的数量。 也就是等价于socket的数量。
>
> windows的 FD_SETSIZE 是允许设置不限制大小的数值。而Linux的 FD_SETSIZE 无法随意更改，其最大限制是1024个文件（即socket数量）。需要使得Linux突破系统规定的最大文件描述符数量则需要深入的进行一些系统修改。
>
> 

- 修改**Linux**系统的进程打开**文件最大数量的限制**

  使用命令 ``` ulimit -a ``` 即可查询linux系统的限制信息。也可以直接使用 ```ulimit -n```来查询。

  其中```open files```这一行信息就是一个进程管理的最大文件数量。

  ```shell
  # 在控制台输入以下命令即可修改文件最大数量的限制
  # ulimit -n 数量
  ulimit -n 10240
  #上述指令将open files 限制从1024 修改为 10240
  
  # 每次terminal终端关闭之后，open files会变回默认值，需要重新再次使用上述命令设置。
  ```

- 配置Linux的**系统配置**文件来实现修改**文件最大数量限制**。

  修改Linux系统单进程最大打开文件数量限制，系统文件配置方法：

  1. 查询：

     所有进程打开文件总数的限制，由内存来决定，内存越大，打开的文件总数越大。

     - 命令1：

       ```cat /proc/sys/fs/file-max```

     - 命令2：

       ```ulimit - n```

  2. 修改：

     输入指令```sudo vim /etc/security/limits.conf```来修改limits.conf文件。

     添加或者修改以下两行信息

     ``* soft nofile 65535``

     ``* hard nofile 65535``

     然后重启电脑，即可完成文件最大数量限制的修改。

     > soft nofile 是默认修改数值，即重启后每次打开terminal终端默认为 soft nofile的数值。
     >
     > 而hard nofile是普通用户最大的修改数值。普通用户修改最大文件数量不能超过hard nofile的数值。

     > 为什么是 65535 这个数值，因为系统的端口号是uint 32 的整数，所有由2^32-1个端口数，设置跟端口号一致足够程序使用了。

     **ps：**实际上root用户(su)是没有修改限制，只要在1048576范围内就行（0<= ulimit -n <= 1048576）

- Linux系统对于 FD_SETSIZE 实现的库文件与windous不一致，不能简单修改代码的宏从而达到修改 FD_SETSIZE 的目的

  - Linux的 fd_set 是按位存储的，即便是只有一个socket，但是其socket的数值大于1024也会引起错误。
  - Linux的 FD_SET方法与Windows的截然不同。Linux的效率更高，但是代码写死了，限制最大数值为1024，不允许随意更改。

- Linux的 **fd_set 相关问题**解决方式

  - 最安全、稳妥的方法：

    重新编译内核。（极难、麻烦）

  - 不需要重新编译内核的方法：

    > 该方法需要经过足够的测试才能确保真正的稳定
    >
    > 且 该方法不能保证完全消除“未定义的行为”

    创建FDSet类，自己重写FDSet相关的内容：

    1. 建立一个指针作为fd_set
    2. 自定义指针所指空间的大小，不同平台的计算方式和存储方式不同。
    3. 编写自定义的FDSet的增删改查四个方法。

## epoll 网络模型

> epoll是在Linux下开发的，也是专门为Linux做的一个网络模型。适用于Linux各种延申的系统，而select则适用于所有主流系统。
>
> 相对与select的轮询机制，epoll的wait机制更像是一种反射。
>
> epoll相对于select的1024个文件数量限制具有极大优势，因为epoll可以根据系统资源动态调整运行打开的文件数量。
>
> epoll默认LT模式，如果你对fd_sock循环检测是否可写，会一直输出可写。
>
> [linux epoll man 详解](https://www.man7.org/linux/man-pages/man7/epoll.7.html)

### 使用方法

1. 导入epoll头文件

   ```c++
   #include <sys/epoll.h>
   // 导入一个头文件，就可以使用全部epoll的方法
   ```

2. 创建一个epol的对象

   > 创建一个epoll的对象，但是实际上不算是对象，而是使用epoll的描述符或者句柄

   ```c++
   int epfd = epoll_create(10240); //参数是：epoll对象可以管理文件数量的大小
   ```

   参数是：epoll对象可以管理文件数量的大小，该数值是int类型，所以最大为2^32。但实际上Linux2.6.8后，这个参数变得没有意义了，因为Linux2.6.8之后，epoll可以打开的文件数量是根据系统资源动态变化。打开的最大值由电脑的内存大小决定，可以使用如下代码查询：

   ```shell
   cat /proc/sys/fs/file-max
   ```

3. epoll 操作

   - epoll由特定的方法来进行操作：epoll_ctl()

     > 该方法是向epoll对象注册需要管理、监听的socket文件描述符，并且传入需要关注的事件，以及相应的操作。

     ```c++
     epoll_ctl(epfd,EPOLL_CTL_MOD,_sock,&ev);
     ```

   - 该方法有四个参数：

     1. 第一个参数：epoll的对象，类型是int类型，epfd

     2. 第二个参数：epoll的注册操作的类型，由epoll库提供，分别是：EPOLL_CTL_ADD、EPOLL_CTL_DEL、EPOLL_CTL_MOD

        - EPOLL_CTL_ADD：表示新注册事件。

        - EPOLL_CTL_DEL：表示删除已注册的事件。

        - EPOLL_CTL_MOD：表示修改已注册的事件。

     3. 第三个参数：socket描述符，即被epoll操作的文件描述符

     4. 第四个参数：一个事件的对象，该对象的类型是一个结构体，epoll_event：

        ```c++
        // epoll_event
        
        struct epoll_event
        {
          uint32_t events;	/* Epoll events */
          epoll_data_t data;	/* User data variable */
        } __EPOLL_PACKED;
        
        typedef union epoll_data
        {
          void *ptr;
          int fd;
          uint32_t u32;
          uint64_t u64;
        } epoll_data_t;
        ```

        在实际使用中，需要对epoll_event对象进行配置：

        1. 配置ev对象的事件：

           ```c++
           ev.events = EPOLLIN;
           // ev.events = EPOLLOUT;
           ```

           该事件对象就是对文件的一些操作，例如： 可读、可查、可写等操作。

        2. 配置ev对象管理的文件描述符：

           ```c++
           ev.data.fd = _sock;
           ```

   - 该函数的返回值：

     - 返回 0 表示该操作成功；
     - 返回 负数 表示操纵失败，一般都是返回 -1；

4. 等待事件的发生

   - 等待注册的事件发生：epoll_wait()

     > 

     ```c++
     extern int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout);
     
     // epoll_wait(epfd,events,256,0);
     ```

   - 该方法有四个参数：

     1. 第一个参数：epoll的对象，类型是int类型，epfd

     2. 第二个参数：epoll的事件数组，用来接收检测到的事件。数组的大小可以是epoll对象管理文件的数值，尽管epoll对象管理文件数量的数值已经没有意义了，但是为了兼容旧版本，所以需要保存其一致。**epoll的事件数组其实是可以根据客户端的数量来动态变化**的，客户端数量变多了，可以重新new一个epoll的事件数组。

        ```c++
        epoll_event events[256] = {};
        ```

     3. 第三个参数：该参数是说明第二个参数传入的事件数组的大小。也可以理解为能接受事件的能力，允许比事件数组的容量小，但是不允许大于事件数组的容量。

     4. 第四个参数：超时的时间（单位为**毫秒**），可以传入数值``` 0 ```，数值为0则表示触发事件立即反回。传入数值``` -1 ```，表示没有触发事件则一直阻塞。

   - 返回值：

     返回值表示触发事件的个数。

     - 返回 0 表示有0个触发事件；
     - 返回 正数n 表示有n个触发事件；
     - 返回 负数 表示操纵失败，一般都是返回 -1；

5. 接受到n个触发事件后进行事件处理：

   > 在触发事件数组当中会存储当前已经被触发的事件。随后根据这些被调用的事件信息来进行自定义的操作。

   ```c++
   // 这是根据触发事件数组，进行socket的accept操作示例。
   for (int i = 0; i < n; ++i) {
       if (events[i].data.fd == _sock){
           //---------------================-----------------
           // 这是根据程序逻辑来编写不一样的代码，可以是socket的accept操作、可以是socket的消息接收操作、可以是socket的消息发送操作
           sockaddr_in clientAddr = {};
           int nAddrLen = sizeof(sockaddr_in);
           SOCKET _cSock = INVALID_SOCKET;
   
           _cSock = accept(_sock,(sockaddr*)&clientAddr,(socklen_t*)&nAddrLen);
           if (_cSock == INVALID_SOCKET){
           	std::cout << "接受客户端出错\n";
       	}
           //---------------================-----------------
       }
   }
   ```

6. 关闭epoll描述符：

   ```c++
   close(epfd);
   ```

## IOCP网络模型

> IOCP网络模型与select、epoll有较大区别。
>
> IOCP是windows下的网络模型。
>

### 使用方法

> 总体的使用策略就是：
>
> 1. 创建IOCP。
> 2. 将设备（socket，文件描述符，句柄）这类东西与IOCP关联。
> 3. 向IOCP投递特定的任务，如：某个句柄投递接受连接的任务。
> 4. 根据业务可以重复以上2 3操作。

1. 导入头文件

2. 创建 IO完成端口 IOCP (IoCompletionPort)

   ```c++
   CreateIoCompletionPort(_In_ HANDLE FileHandle, _In_opt_ HANDLE ExistingCompletionPort, _In_ ULONG_PTR CompletionKey, _In_ DWORD NumberOfConcurrentThreads);
   
   // 功能1
   HANDLE _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
   if (NULL == _completionPort)
   {
       std::cout << "CreateIoCompletionPort : IOCP create failed with error " << GetLastError() << "\n";
   }
   
   // 功能2
   auto ret = CreateIoCompletionPort((HANDLE)sockServer, _completionPort, (ULONG_PTR)sockServer, 0);
   if (NULL == ret)
   {
       std::cout << "CreateIoCompletionPort : IOCP relevant failed with error " << GetLastError() << "\n";
   }
   ```

   CreateIoCompletionPort 函数要传入四个参数。

   CreateIoCompletionPort 函数有两个功能:

   - 创建一个IO完成端口：

     要使用该功能，前三个参数为：INVALID_HANDLE_VALUE、NULL、0；

     第四个参数是IOCP允许并发线程的数量（为0则默认为cpu的数量）；

     返回一个**IOCP的句柄**。

   - 将一个设备（文件）与IO完成端口相联：

     要使用该功能，第一个参数传入设备句柄（文件、socket等都属于设备）

     第二个参数传入**IOCP的句柄**；

     第三个参数传入完成键值；

     第四个参数传入0，如果使用关联功能的话，该参数其实是被忽略的；

     返回一个**IOCP的句柄**。

3. 关联IOCP和设备（文件，socket）

   上述的 CreateIoCompletionPort 第二个功能。

4. 向IOCP投递接受链接的任务  **AcceptEX**

   > 使用AcceptEX需要提前创建sockfd等待连接socket。
   >
   > AcceptEX是完全的异步操作。

   ```c++
   AcceptEx (
       _In_ SOCKET sListenSocket,
       _In_ SOCKET sAcceptSocket,
       _Out_writes_bytes_(dwReceiveDataLength+dwLocalAddressLength+dwRemoteAddressLength) PVOID lpOutputBuffer,
       _In_ DWORD dwReceiveDataLength,
       _In_ DWORD dwLocalAddressLength,
       _In_ DWORD dwRemoteAddressLength,
       _Out_ LPDWORD lpdwBytesReceived,
       _Inout_ LPOVERLAPPED lpOverlapped
       );
   ```

   使用方式：

   - 导入头文件``` #include <MSWSock.h> ```

   - 需要提前为**客户端**创建一个socket；（只有服务端才会使用accept）

     ```c++
     SOCKET sockClient =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     ```

   - 需要提前创建好一个存储数据的**缓冲区**

     ```c++
     char* buffer[1024] = {};
     ```

   - 接收数据字节数的反馈

     > 该参数仅当配置为同步操作时才有作用，如果是异步操作则不需要设置该参数。

     ```c++
     DWORD dwBytes = 0;
     ```

   - 创建 重叠体

     > 包含用于异步 (或 *重叠*) 输入和输出 (I/O) 的信息。

     ```c++
     OVERLAPPED overlapped = {};
     ```

   - 返回值

     如果没有错误，则返回True
     
     如果存在错误，则返回false

   - 使用 AcceptEx 函数

     ```c++
     SOCKET sockServer =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     SOCKET sockClient =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     char* buffer[1024] = {};
     //DWORD dwBytes = 0;
     OVERLAPPED overlapped = {};
     
     if (false == AcceptEx(sockServer
                           , sockClient
                           , buffer
                           , 0  //  1、 填入 0 代表不要求客户端发送数据   2、 填入 sizeof(buffer) - ((sizeof(sockaddr_in) + 16) * 2) 也可以
                           , sizeof(sockaddr_in) + 16
                           , sizeof(sockaddr_in) + 16
                           , NULL //&dwBytes
                           , &overlapped))
     {
         int err = WSAGetLastError();
         if (ERROR_IO_PENDING != err)
         {
             // AcceptEx 错误
             std::cout << "AcceptEx failed with error " << err << "\n";
             return 0;
         }
     }
     ```

   - 提升效率

     > 将AcceptEx函数加载内存中，调用效率更高

     ```c++
     LPFN_ACCEPTEX lpfnAcceptEx = NULL;
     void loadAcceptEx(SOCKET ListenSocket)
     {
     	GUID GuidAcceptEx = WSAID_ACCEPTEX;
     	DWORD dwBytes = 0;
     	int iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
     		&GuidAcceptEx, sizeof(GuidAcceptEx),
     		&lpfnAcceptEx, sizeof(lpfnAcceptEx),
     		&dwBytes, NULL, NULL);
     
     	if (iResult == SOCKET_ERROR) {
     		printf("WSAIoctl failed with error: %u\n", WSAGetLastError());
     	}
     }
     ```

5. 循环 检测IOCP状态

   > 尝试从指定的 I/O 完成端口取消对 I/O 完成数据包的排队。 如果没有完成数据包排队，函数将等待与完成端口关联的挂起 I/O 操作完成。若要一次取消多个 I/O 完成数据包的排队，请使用 [GetQueuedCompletionStatusEx](https://learn.microsoft.com/zh-cn/windows/desktop/FileIO/getqueuedcompletionstatusex-func) 函数。
   >
   > 此函数将线程与指定的完成端口相关联。 一个线程最多可以与一个完成端口相关联。
   >
   > 
   >
   > 检测和获取完成端口队列中的端口状态。
   >
   > WSARecv、WSASend 分别是IOCP的接收操作和发送操作。
   >
   > 两个操作都完全是异步操作。

   检测IOCP关联任务的完成状态 GetQueuedCompletionStatus();

   - 函数参数

     ```c++
     GetQueuedCompletionStatus(
         _In_ HANDLE CompletionPort,
         _Out_ LPDWORD lpNumberOfBytesTransferred,
         _Out_ PULONG_PTR lpCompletionKey,
         _Out_ LPOVERLAPPED* lpOverlapped,
         _In_ DWORD dwMilliseconds
         );
     ```

   - 使用方式

     ```c++
     // _completionPort 是 IOCP
     DWORD bytesTrans = 0;
     SOCKET sock = INVALID_SOCKET;
     LPOVERLAPPED lpoverlapped;
     
     GetQueuedCompletionStatus(_completionPort
     			, &bytesTrans
     			, (PULONG_PTR)&sock
     			, &lpoverlapped
     			, 1 /*INFINITE*/)
     			)
     ```

     - [in] CompletionPort

       完成端口的句柄。 若要创建完成端口，请使用 [CreateIoCompletionPort](https://learn.microsoft.com/zh-cn/windows/desktop/FileIO/createiocompletionport) 函数。

     - lpNumberOfBytesTransferred

       指向变量的指针，该变量接收在完成的 I/O 操作中传输的字节数。

     - [out] lpCompletionKey

       指向变量的指针，该变量接收与 I/O 操作已完成的文件句柄关联的完成键值。 完成键是在对 [CreateIoCompletionPort](https://learn.microsoft.com/zh-cn/windows/desktop/FileIO/createiocompletionport) 的调用中指定的每个文件密钥。

     - [out] lpOverlapped

       指向变量的指针，该变量接收在启动完成 I/O 操作时指定的 [OVERLAPPED](https://learn.microsoft.com/zh-cn/windows/desktop/api/minwinbase/ns-minwinbase-overlapped) 结构的地址。

     - [in] dwMilliseconds

       调用方愿意等待完成数据包出现在完成端口上的毫秒数。 如果完成数据包未在指定时间内显示，则该函数超时，返回 **FALSE**，并将 \**lpOverlapped* 设置为 **NULL**。

       如果 *dwMilliseconds* 为 **INFINITE**，则函数永远不会超时。如果 *dwMilliseconds* 为零，并且没有要取消排队的 I/O 操作，则函数将立即超时。

   - 函数返回值

     如果成功，则返回非零 (**TRUE**) ，否则返回零 (**FALSE**) 。

   - 具体工作内容\实际使用

   1. GetQueuedCompletionStatus的使用基本逻辑

      ```c++
      while (true)
      {
      	DWORD bytesTrans = 0;
      	SOCKET sock = INVALID_SOCKET;
      	IO_DATA_BASE* pIOData;
      
      	if (FALSE == GetQueuedCompletionStatus(_completionPort, &bytesTrans, (PULONG_PTR)&sock, (LPOVERLAPPED*)&pIOData, 1))
      	{
      		int err = GetLastError();
      		if (WAIT_TIMEOUT == err)
      		{
      			continue;
      		}
      		if (ERROR_NETNAME_DELETED == err)
      		{
      			printf("关闭 sockfd=%d\n", pIOData->sockfd);
      			closesocket(pIOData->sockfd);
      			continue;
      		}
      		printf("GetQueuedCompletionStatus failed with error %d\n", err);
      		break;
      	}
      	// 接受链接 完成
      	if (IO_TYPE::ACCEPT == pIOData->iotype)
      	{
      		printf("新客户端加入 sockfd=%d\n", pIOData->sockfd);
      		// 关联IOCP与ClientSocket
      		auto ret = CreateIoCompletionPort((HANDLE)pIOData->sockfd, _completionPort, (ULONG_PTR)pIOData->sockfd, 0);
      		if (!ret)
      		{
      			printf("关联IOCP与ClientSocket=%d失败\n", pIOData->sockfd);
      			closesocket(pIOData->sockfd);
      			continue;
      		}
      		// 向IOCP投递接收数据任务
      		postRecv(pIOData);
      	}
      	// 接收数据 完成 Completion
      	else if (IO_TYPE::RECV == pIOData->iotype)
      	{
      		if (bytesTrans <= 0)
      		{//客户端断开处理
      			printf("关闭 sockfd=%d, RECV bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
      			closesocket(pIOData->sockfd);
      			continue;
      		}
      		printf("收到数据: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, ++msgCount);
      		pIOData->length = bytesTrans;
      		// 向IOCP投递发送数据任务
      		postSend(pIOData);
      	}
      	// 发送数据 完成 Completion
      	else if (IO_TYPE::SEND == pIOData->iotype)
      	{
      		if (bytesTrans <= 0)
      		{//客户端断开处理
      			printf("关闭 sockfd=%d, SEND bytesTrans=%d\n", pIOData->sockfd, bytesTrans);
      			closesocket(pIOData->sockfd);
      			continue;
      		}
      		printf("发送数据: sockfd=%d, bytesTrans=%d msgCount=%d\n", pIOData->sockfd, bytesTrans, msgCount);
      		// 向IOCP投递接收数据任务
      		postRecv(pIOData);
      	}
      	else {
      		printf("未定义行为 sockfd=%d", sock);
      	}
      }
      ```

   2. 投递接受连接的任务

      ```c++
      void postAccept(SOCKET sockServer, IO_DATA_BASE* pIO_DATA)
      {
      	pIO_DATA->iotype = IO_TYPE::ACCEPT;
      	pIO_DATA->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      	if (FALSE == lpfnAcceptEx(sockServer
      		, pIO_DATA->sockfd
      		, pIO_DATA->buffer
      		, 0
      		, sizeof(sockaddr_in) + 16
      		, sizeof(sockaddr_in) + 16
      		, NULL
      		, &pIO_DATA->overlapped
      	))
      	{
      		int err = WSAGetLastError();
      		if (ERROR_IO_PENDING != err)
      		{
      			printf("AcceptEx failed with error %d\n", err);
      			return;
      		}
      	}
      }
      ```

   3. 投递接收数据的任务

      ```c++
      void postRecv(IO_DATA_BASE* pIO_DATA)
      {
      	pIO_DATA->iotype = IO_TYPE::RECV;
      	WSABUF wsBuff = {};
      	wsBuff.buf = pIO_DATA->buffer;
      	wsBuff.len = DATA_BUFF_SIZE;
      	DWORD flags = 0;
      	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));
      
      	if (SOCKET_ERROR == WSARecv(pIO_DATA->sockfd, &wsBuff, 1, NULL, &flags, &pIO_DATA->overlapped, NULL))
      	{
      		int err = WSAGetLastError();
      		if (ERROR_IO_PENDING != err)
      		{
      			printf("WSARecv failed with error %d\n", err);
      			return;
      		}
      	}
      }
      
      ```

   4. 投递发送数据的任务

      ```c++
      void postSend(IO_DATA_BASE* pIO_DATA)
      {
      	pIO_DATA->iotype = IO_TYPE::SEND;
      	WSABUF wsBuff = {};
      	wsBuff.buf = pIO_DATA->buffer;
      	wsBuff.len = pIO_DATA->length;
      	DWORD flags = 0;
      	ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));
      
      	if (SOCKET_ERROR == WSASend(pIO_DATA->sockfd, &wsBuff, 1, NULL, flags, &pIO_DATA->overlapped, NULL))
      	{
      		int err = WSAGetLastError();
      		if (ERROR_IO_PENDING != err)
      		{
      			printf("WSASend failed with error %d\n", err);
      			return;
      		}
      	}
      }
      ```

6. 关闭完成端口IOCP

   ```c++
   closesocket(sockServer);
    // close IOCP
   closeHandle(_completionPort);
   // close windows socket
   WSACleanup();
   ```

   











