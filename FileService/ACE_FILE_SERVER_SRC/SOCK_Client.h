/******************************************************************************
ģ������:  �ͻ���
ģ�鹦��:  Client
������:    ��
��������:  2012/10
��������:
��ע:      Client:�ͻ�����,������Server�˵�ͨѶ,ͨ��ָ������ִ�в���
Common:�������ݽṹ(server��,client��,������)
LocalFile.�����ļ�����;  ����Server���ļ�д������Client���ļ�������

����:��������-->�����ļ�Ŀ¼->�����ļ���-->�����ļ���С-->�����ļ�����
//ע��:�ڷ�����û��ʵ�ֳ����ӣ�����ÿ��ֻ��ִ��һ������ٽ�ִ����Ҫ�����������
******************************************************************************/
#ifndef SOCKCLIENT_H
#define SOCKCLIENT_H

#include "ace/SOCK_Connector.h"
//ACE�߳�
#include "ace/OS.h"
#include "ace/Task.h"

#include "ClientMothod.h"
#include "Common.h"
#include "EventHandler.h"

/******************************************************************************
ģ������:  ReactorLoopThread:
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      ACE_Reactor����Ϣѭ���߳�;
******************************************************************************/
class ReactorLoopThread:public ACE_Task_Base
{
private:
	ACE_Reactor* reactor;
public:
	ReactorLoopThread(){this->reactor = NULL;}
public:
	virtual int open(ACE_Reactor* r)
	{
		this->reactor = r;
		return this->activate()	;
	}
	virtual int svc(void)
	{
		if(this->reactor != NULL)
			this->reactor->run_reactor_event_loop();			
		return 0;
	}
};

class Client
{
public:
	Client(){}
	virtual ~Client(){}
private:
	ACE_SOCK_Connector		conn;
	ACE_SOCK_Stream			peer_sock;
	ACE_INET_Addr			serverAddr;	

	CilentCMDFrame   		sendFrame;
	CilentCMDFrame			revcFrame;

	CallBackFunction		FCallBackFunction;
private:
	//���Ͳ��������������
	bool sendAndReceive();
public:
	//���ӷ�����
	bool connectServer(const char *serverName, int port,FWndCallBackHandle callback = NULL);
	//�����ļ�
	int sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath);
	//�����ļ�
	int recvFileFrameServer(const char* RemotefileNamePath,const char* LocalDirPath);
	//��ȡĿ¼�б�
	int recvFileListFramServer(const char* path);
	//�ļ�ɾ��
	bool FileDel(const char* RemoteFilePath);
	//Ŀ¼����
	bool DirCreate(const char* RemoteDirPath);
	//Ŀ¼ɾ��
	bool DirDel(const char* RemoteDirPath);
	//��ȡ������������
	int getConnectedCount();
	//ͨ��ʱ�䷶Χ��ȡ¼���ļ��б�
	int recvFileListByDateFramServer(const char* path,const char* begindate,const char* enddate);
};
#endif