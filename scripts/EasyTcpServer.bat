@echo off
::::::::::::::::::
::key-val
::�ֵ�
::std::map<k,v>
::QQ:282274770
::::::::::::::::::
::�����IP��ַ
set cmd="strIP=any"
::����˶˿�
set cmd=%cmd% nPort=4567
::��Ϣ�����߳�����
set cmd=%cmd% nThread=1
::�ͻ�����������
set cmd=%cmd% nMaxClient=10000
::�ͻ��˷��ͻ�������С���ֽڣ�
set cmd=%cmd% nSendBuffSize=20480
::�ͻ��˽��ջ�������С���ֽڣ�
set cmd=%cmd% nRecvBuffSize=20480
::�յ���Ϣ�󽫷���Ӧ����Ϣ
set cmd=%cmd% -sendback
::��ʾ���ͻ�������д��
::������sendfull��ʾʱ����ʾ������Ϣ������
set cmd=%cmd% -sendfull
::�����յ��Ŀͻ�����ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID
::�Զ����־ δʹ��
set cmd=%cmd% -p

::�������� �������
EasyTcpServer %cmd%

pause