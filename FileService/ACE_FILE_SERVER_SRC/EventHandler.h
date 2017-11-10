/******************************************************************************
ģ������:  ���������(server��client���ö���)
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      

Common:�������ݽṹ(server��,client��,������)
LocalFile.�����ļ�����;  ����Server���ļ�д������Client���ļ�������
Bass_Event_Handler:�¼��������
File_Recv_Event_Handler:�ļ����մ�����
File_Send_Event_Handler:�ļ����ʹ�����

����:��������-->�����ļ�Ŀ¼->�����ļ���-->�����ļ���С-->�����ļ�����
******************************************************************************/
#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h" 
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"
#include "ace/FILE_IO.h"
#include "ace/Reactor.h"
#include "ace/Free_List.h"

#include "ace/Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"

#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"
//#include "ace/Containers_T.h"

#include "ace/OS.h"

//#include <ace/Lock.h>
#include <ace/Synch.h>

#include "LocalFile.h"
#include "Common.h"
#include "ClientMothod.h"

#include <fstream>
#include <string>
using namespace std;
#define UNICODE


/******************************************************************************
ģ������:  CallBackFunction:
ģ�鹦��:  �¼��ص��¼�(Client�ȽϹ�ע���¼�)
������:    ��
��������:  2012/10
��������:
��ע:		1.���ﴥ�����¼�����µ�Clientע����¼���
			2.Bass_Event_Handler�����඼����ʹ�����������½���

��Ϊc++��֧���ຯ���ص�����û����Bass_Event_Handler����������ʵ�ֻص�������client
��Ҫ���¼���ͨ��CallBackFunction����ķ���֪ͨ����ı��(�ļ����ͽ��ȣ��ɹ������¼�
�������������֪ͨ�����(����ʵ���ǻ����첽����)

//����Ϊ�ص����ͣ�����������ClientMothod.hͷ�ļ���
#define CALLBACK_TYPE_SEND_PROGRESS 1001
#define CALLBACK_TYPE_SEND_SUCESSED	1002
#define CALLBACK_TYPE_SEND_FAILED	1003
#define CALLBACK_TYPE_RECV_PROGRESS 1004
#define CALLBACK_TYPE_RECV_SUCESSED	1005
#define CALLBACK_TYPE_RECV_FAILED	1006
#define CALLBACK_TYPE_RECV_FILELIST 1007
******************************************************************************/
class CallBackFunction
{
private:
		FWndCallBackObject*		Fobject;
public:
		FWndCallBackHandle		FWndcallback;
public:
		CallBackFunction();
		~CallBackFunction();
		void InitWndCallBack(FWndCallBackHandle callback,int handle = 0);
		//���ͽ���
		void OnFRProgressCallBackImpl(int iMax,int iProgress);
		//���ͳɹ�
		void OnFRSucessedCallBackImpl();
		//����ʧ��
		void OnFRFailedCallBackImpl();
		//���ս���
		void OnFSProgressCallBackImpl(int iMax,int iProgress);
		//����ʧ��
		void OnFSSucessedCallBackImpl();
		//���ճɹ�
		void OnFSFailedCallBackImpl();
		//��ȡָ��·�����ļ��б�
		void OnGetFileListByPathImpl(char* text,bool isSucessed = true);
		//ɾ���ļ�
		void OnDelFileImpl(bool isSucessed = true);
		//ɾ��Ŀ¼
		void OnDelDirImpl(bool isSucessed = true);
		//����Ŀ¼
		void OnCreateDirImpl(bool isSucessed = true);
		//�����ļ�ʱ�ķ�����ͬ���ļ�����
		bool OnFSOverWriterCallBackImpl(char* text);
		//�����ļ�ʱ�ķ�����ͬ���ļ�����
		bool OnFROverWriterCallBackImpl(char* text);
		//�������ڻ�ȡָ��·�����ļ��б�
		void OnGetFileListByDateAndPathImpl(char* text,bool isSucessed = true);
};
/******************************************************************************
ģ������:  Bass_Event_Handler:
ģ�鹦��:  �¼��������
������:    ��
��������:  2012/10
��������:
��ע:      ����sock��һ�㴦�����̼��ذ�����
******************************************************************************/
class Bass_Event_Handler : public ACE_Event_Handler
{
public:
	Bass_Event_Handler(ACE_Reactor *r): ACE_Event_Handler(r){timeout.sec(ACE_DEFAULT_TIMEOUT);}
	virtual ~Bass_Event_Handler(){}
protected:
	ACE_INET_Addr		client_Addr_;
	ACE_SOCK_Stream		peer_sock_;
	//�Ƿ�ע����WRITE_MASK��Ϣ
	bool isRegWRITE_MASK;
	//������Ҫ���͵���Ϣ����
	ACE_Message_Queue<ACE_NULL_SYNCH> output_queue;	
	//�ص�����
	CallBackFunction	FCallBackFunction;
	ACE_Time_Value		timeout;
public:
	//��Reactorע��READ_TASK�¼�����ʼ���peer_sock�ϵ���������(��ע�����ص��¼�)
	virtual int Open(ACE_SOCK_Stream& sock,FWndCallBackHandle	FWndcallback = NULL);
	//���ͻ��˷�������
	virtual int handle_output(ACE_HANDLE = ACE_INVALID_HANDLE);
	//ע���¼�����
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//��ǰsock����
	virtual ACE_SOCK_Stream& peer() ;
	//ָ����ǰsock����
	ACE_SOCK_Stream& setpeer(ACE_SOCK_Stream& sock) ;
	ACE_INET_Addr& get_addr();
	//���
	virtual ACE_HANDLE get_handle (void) const;
protected:
	//���ͻذ�
	virtual int send_back_info(void* pSendFrame,int sizeof_FrameType);
};
/******************************************************************************
ģ������:  File_Recv_Event_Handler
ģ�鹦��:  �ļ����մ�����
������:    ��
��������:  2012/10
��������:
��ע:      �����ļ����ղ���
			Server:��������-->��ȡ�ļ���-->��ȡ�ļ���С-->��ȡ�ļ�����
			Client:��������-->��ȡָ�����ļ�-->(����������ʱ)��ȡ�ļ���С-->��ȡ�ļ�����
			Client�����ļ�ʱ��������ͨ��FILE_RECV_CLIENT_GET_FILENAME��Ϣ�ж��Ƿ����Ҫ���յ��ļ�
			������ֱ���·��ļ���С��������������;
****************************************************************************/
class File_Recv_Event_Handler : public Bass_Event_Handler
{
private:
	//�����ļ�����
	LocalFile localFile;	
	//���Կͻ��˵�����֡
	FileSendFrame recvFrame;
	//���͵��ͻ��˵�����֡
	FileRecvFrame sendFrame;	
	//�ļ����ս���
	int	iLastPrgress;
	//��ǰ�ܹ����յĴ�С
	long iTotalRecvSize;
	//Ҫ���յ��ļ���С
	long RemotefileSize;
public:
	File_Recv_Event_Handler(ACE_Reactor *r);
	~File_Recv_Event_Handler(){}
	//���տͻ��˵�����
	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//�ӷ���������ָ���ļ�(client����,��ʼ�򷵻�sock�����ʧ�ܷ���-1)
	int  recvFileFromServer(const char* RemotefileNamePath,
							const char* LocalDirPath);
private:
	//��ʾ���ս�����־
	void showProgress(int recvSize);
	//�յ��ļ���
	void recvFileNameMsg();
	//�յ��ļ���С
	void recvFileSizeMsg();
	//�յ��ļ�����
	void recvFileDataMsg();
	//�յ��ļ����ͽ�����Ϣ
	void recvFileSendMsg();
	//�յ��ļ����Ͷ˳�����Ϣ
	void recvFileSendErrorMsg();
	//�յ����Ͷ˶�ͬ���ļ��ĸ�����Ϣ
	void recvOverWriteSameFileMsg();
};

/******************************************************************************
ģ������:  File_Send_Event_Handler
ģ�鹦��:  �ļ����ʹ�����
������:    ��
��������:  2012/10
��������:
��ע:      �����ļ����͵�����¼�
		   Server:��������->�����ļ���-->�����ļ���С-->�����ļ�����	
		   Client:��������->�����ļ���-->�����ļ���С-->�����ļ�����	
*****************************************************************************/
class File_Send_Event_Handler : public Bass_Event_Handler
{
private:
	FileSendFrame sendFrame;
	FileRecvFrame recvFrame;
	//�����ļ�����
	LocalFile localFile;
	//�����ļ���С
	long iLocalFileSize;
	//���ͽ���
	int	 iSendPrgress;
	//�ѷ��ʹ�С
	long iTotalSendSize;
public:
	File_Send_Event_Handler(ACE_Reactor *r);
	~File_Send_Event_Handler(){}

	//�����ļ����ն˵�����
	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//���ͱ����ļ���������(client����,��ʼ�򷵻�sock�����ʧ�ܷ���-1)
	int  sendFileToServer(const char* LocalfileNamePath,
						  const char* RemoteDirPath);
private:
	//��ʾ���ͽ���
	void showSendProgress(int readSize);
	//(��Ϣ���ͺ���)
	//�����ļ���
	void sendFileName();
	//�����ļ���С
	void sendFileSize();
	//�����ļ�����
	void sendFileData();
	//�Ƿ񸲸�ͬ���ļ�
	void sendIsOverWriteSameFile();
	//����Client��ȡserver����ָ���ļ��Ļ�ִ
	void checkedLocalFileIsExistAndSendResult();
};

/******************************************************************************
ģ������:  CMDMsg_Recv_Send_Event_Handler
ģ�鹦��:  ��Ϣ�ı��ķ��ͺͽ�����
������:    ��
��������:  2012/10
��������:
��ע:      ������Ϣ�ı����͵�Clientʱͨ���������д���(ClientҲͨ�����������ı�)
*****************************************************************************/
class CMDMsg_Recv_Send_Event_Handler:public Bass_Event_Handler
{
private:
	CMDMsgFrame	sendCMDMsgFrame;
	CMDMsgFrame	recvCMDMsgFrame;

	char*	RecvCMDMsg;
	char*	SendCMDMsg;
	//�ѽ�����Ϣ����
	int	 iCMDMsgRecvLenth;
	//�ѷ�����Ϣ����
	int	 iCMDMsgSendLenth;
	//����Ϣ����
	long iCMDMsgTotalLenth;
	//��ǰ����Ϣ����
	CilentCMDFrameType	CMDFrameType;
public:
	//sendCMDMsgΪҪ���͵�����;�ɷ������趨
	CMDMsg_Recv_Send_Event_Handler(CilentCMDFrameType type,ACE_Reactor *r,const char* sendCMDMsg = NULL);
	~CMDMsg_Recv_Send_Event_Handler(){}

	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//������Ϣ�ı����ͻ���(client ����)
	void recvCMDMsgFramServer();
};
/******************************************************************************
ģ������:  Client_List
ģ�鹦��:  �����б�
������:    ��
��������:  2012/10
��������:
��ע:      
/******************************************************************************/
class Client_List
{
private:
	ACE_Map_Manager<ACE_HANDLE,Bass_Event_Handler*,ACE_Thread_Mutex> Fmap;		
public:
	static Client_List* Instens();
private:
	Client_List(){}
	~Client_List(){Fmap.unbind_all();}
public:
	void	AddEvent(ACE_HANDLE Handle,Bass_Event_Handler* ptr);	
	Bass_Event_Handler* GetEventByHandle(ACE_HANDLE Handle);
	Bass_Event_Handler* RemoveEventByHandle(ACE_HANDLE Handle);
	int		GetCount();	
	void	Clear();
};
/******************************************************************************
ģ������:  DirHelper
ģ�鹦��:  Ŀ¼������
������:    ��
��������:  2012/10
��������:
��ע:      strtok_r��linuxƽ̨�µ�strtok�������̰߳�ȫ��

		   GetFileNameByPath���ú���Ҫʹ��delete[]ɾ������
/******************************************************************************/
class DirHelper
{
private:
	static char* FtpServerMainDir;	
public:
	static bool  InitMainDir(const char* pzMainDir);

	//����������:Client�����Ķ��Ƿ����������·��,����������ʵ·�� = FtpServerMainDir + ���·��;
	//(���ú�ʹ��delete[]ɾ������)
	static char* GetServerLocalDirPath(const char* pzPath);

	//�������·���ĺϷ��ԣ����������Զ�����(!!!����Ϊ���������·��)
	static bool  CreateServerDirPath(const char* pzPath);
	//���Ŀ¼�Ƿ����(����Ϊ���������·��)
	static bool  CheckedServerDirPathIsExist(const char* pzPath);
	//���ָ��Ŀ¼�Ƿ����(����Ϊ����·��)
	static bool	 CheckedDirPathIsExist(const char* pzPath);

	//����2������ʹ��strtok_r�ָ�·��(��������Ϊ���·��,Ҳ����Ϊ����·��)
	//��ȡ·��(���ú�ʹ��delete[]ɾ������)
	static char* GetFileNameByPath(const char* pzPath);
	//��ȡ�ļ���(���ú�ʹ��delete[]ɾ������)
	static char* GetFilePathByName(const char*  pzFullFileName);
	//����Linux·��(��//�����滻��\����)
	static void  SetLinuxPath(char* outPath);
};
#endif

