@echo off
::::::::::::::::::
::key-val
::�ֵ�
::std::map<k,v>
::QQ:282274770
::::::::::::::::::
::�����IP��ַ
set cmd="strIP=127.0.0.1"
::����˶˿�
set cmd=%cmd% nPort=4567
::�����߳�����
set cmd=%cmd% nThread=1
::ÿ�������̣߳��������ٸ��ͻ���
set cmd=%cmd% nClient=10000
::::::���ݻ���д�뷢�ͻ�����
::::::�ȴ�socket��дʱ��ʵ�ʷ���
::ÿ���ͻ�����nSendSleep(����)ʱ����
::����д��nMsg��Login��Ϣ
::ÿ����Ϣ100�ֽڣ�Login��
set cmd=%cmd% nMsg=10
set cmd=%cmd% nSendSleep=1000
::�ͻ��˷��ͻ�������С���ֽڣ�
set cmd=%cmd% nSendBuffSize=20480
::�ͻ��˽��ջ�������С���ֽڣ�
set cmd=%cmd% nRecvBuffSize=20480
::�����յ��ķ������ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID
::::::
::::::
::�������� �������
EasyTcpClient %cmd%

pause