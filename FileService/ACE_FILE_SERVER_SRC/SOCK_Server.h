#ifndef SOCKSERVER_H
#define SOCKSERVER_H

#include "Common.h"
#include "EventHandler.h"
#include "ace/Task.h"
/******************************************************************************
ģ������:  CMD_Event_Handler
ģ�鹦��:  �������
������:    ��
��������:  2012/10
��������:
��ע:      ����Client��CMD		   
*****************************************************************************/
class CMD_Event_Handler: public ACE_Task_Base
{
private:
	CilentCMDFrame	serverCMDRetFrame;
	CilentCMDFrame	clientCMDFrame;

	ACE_INET_Addr			client_Addr_;
	ACE_SOCK_Stream			peer_sock_;
	Bass_Event_Handler*		FEvent;
public:
	CMD_Event_Handler():ACE_Task_Base(){FEvent = NULL;}
	~CMD_Event_Handler(){}

	virtual int open(ACE_SOCK_Stream&	sock)
	{
		this->peer_sock_ = sock;
		this->peer_sock_.get_remote_addr(client_Addr_);
		return this->activate()	;
	}
	virtual int svc(void);
private:
	//�����ļ�
	bool recvFileSendCMD();
	//�ļ�����
	bool recvFileRecvCMD();
	//��ȡ�ļ��б�
	bool recvFileGetListCMD();
	//�ļ�ɾ��
	bool recvFileDelCMD();
	//Ŀ¼����
	bool recvDirCreateCMD();
	//Ŀ¼ɾ��
	bool recvDirDelCMD();
	//��ǰ������
	bool recvGetConnectedCountCMD();
	//ͨ��ʱ�䷶Χ��ȡ¼���ļ��б�
	bool recvFileListByDateCMD();
};

/******************************************************************************
ģ������:  �ļ�������
ģ�鹦��:  Server
������:    ��
��������:  2012/10
��������:
��ע:      Server:��������,����Client������

����:��������-->�����ļ�Ŀ¼->�����ļ���-->�����ļ���С-->�����ļ�����
����:����Ϊ�˷�����Linux�½��б��룬�������ʵ�ַŵ���һ���ļ���;
******************************************************************************/
class Server  : public ACE_Event_Handler
{ 
private: 
	ACE_SOCK_Acceptor	peer_acceptor_; 
	CilentCMDFrame		serverCMDRetFrame;
public: 
	Server(ACE_Reactor *r = ACE_Reactor::instance()) : ACE_Event_Handler(r) {}
	virtual ~Server();

	//�򿪶˿�ע�ᣬ����ʼ������Ĭ�϶˿ں���66666
	int open(int port = 66666);
	//��������
	int handle_input(ACE_HANDLE);
	int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0);
	ACE_HANDLE get_handle(void) const ;
};

#endif

