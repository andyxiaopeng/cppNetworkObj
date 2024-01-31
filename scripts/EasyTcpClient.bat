@echo off
::::::::::::::::::
::key-val
::字典
::std::map<k,v>
::QQ:282274770
::::::::::::::::::
::服务端IP地址
set cmd="strIP=127.0.0.1"
::服务端端口
set cmd=%cmd% nPort=4567
::工作线程数量
set cmd=%cmd% nThread=1
::每个工作线程，创建多少个客户端
set cmd=%cmd% nClient=10000
::::::数据会先写入发送缓冲区
::::::等待socket可写时才实际发送
::每个客户端在nSendSleep(毫秒)时间内
::最大可写入nMsg条Login消息
::每条消息100字节（Login）
set cmd=%cmd% nMsg=10
set cmd=%cmd% nSendSleep=1000
::客户端发送缓冲区大小（字节）
set cmd=%cmd% nSendBuffSize=20480
::客户端接收缓冲区大小（字节）
set cmd=%cmd% nRecvBuffSize=20480
::检查接收到的服务端消息ID是否连续
set cmd=%cmd% -checkMsgID
::::::
::::::
::启动程序 传入参数
EasyTcpClient %cmd%

pause