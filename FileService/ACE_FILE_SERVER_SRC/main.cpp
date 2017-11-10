/******************************************************************************
ģ������:  
ģ�鹦��:  ����Ҫ����ָ���ĳ���(Server��/Client���Զ�/Dllģ��)
������:    ��
��������:  2012/10
��������:
��ע:      ����dll: 1.�޸�ClientMothod.h�ĺ�ΪVER_DLL	
					2.�޸���Ŀ��������Ϊ.dll
					3.�޸����ɵ�dll�ļ���ΪSOCK_Connecter.dll,�ٱ��뼴��
		   ����exe; 1.�޸�ClientMothod.h�ĺ�ΪVER_SERVER	
					2.�޸���Ŀ��������Ϊ.exe
					3.�޸�������ΪSOCK_Server.exe�ٱ��뼴��;
 		   ����Linux�汾�ķ�����:
					1.�޸�ClientMothod.h�ĺ�ΪVER_SERVER
					2.��Ҫ��Linux�����±���
******************************************************************************/
#include "ClientMothod.h"


#ifdef VER_SERVER
#include "SOCK_Server.h"
#endif

#ifdef VER_CLIENT
#include "SOCK_Client.h"
#endif


#ifdef VER_DLL
#include "SOCK_Client.h"
#endif



#ifdef VER_SERVER
int main (int argc, char *argv[]) 
{ 
	//��־�����ļ�
	ACE_OSTREAM_TYPE *output = new ofstream("FileServer.txt", ios::app);

	ACE_LOG_MSG->msg_ostream (output, 1);    
  
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::STDERR);    	
	//������ļ�
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM); 
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("Server begin\n")));

	//���֮ǰ�ı�ǲ��������Ļ
	//ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR); 

	int port  = argc >= 2 ? ACE_OS::atoi(argv[1]) : 19998;

	//��ʼ��ϵͳĿ¼
	const char* _ftpMainDir = argc >= 3 ? argv[2] : "\\ACE_FileServerDir";	
	if(!DirHelper::InitMainDir(_ftpMainDir))
	{
		ACE_DEBUG((LM_ERROR,ACE_TEXT("InitMainDir Failure!!")));
	}else
	{
		Server* server = new Server();	
		if(server->open(port) != -1)
		{
			while(1) 
				ACE_Reactor::instance()->handle_events(); 
		}
	}
	return 0;
}
#endif //VER_SERVER

#ifdef VER_CLIENT
//�ص�����
void __stdcall OnCallBackImpl(int callBackType,FWndCallBackObject* object)
{
	switch(callBackType)
	{
		case	CALLBACK_TYPE_SEND_PROGRESS:
			printf("CALLBACK_TYPE_SEND_PROGRESS %d%%\r\n",object->iProgress);
			break;
		case	CALLBACK_TYPE_SEND_SUCESSED:
			printf("CALLBACK_TYPE_SEND_SUCESSED\r\n");
			break;
		case	CALLBACK_TYPE_SEND_FAILED:
			printf("CALLBACK_TYPE_SEND_FAILED\r\n");
			break;
		case	CALLBACK_TYPE_RECV_PROGRESS:
			printf("CALLBACK_TYPE_RECV_PROGRESS %d%%\r\n",object->iProgress);
			break;
		case	CALLBACK_TYPE_RECV_SUCESSED:
			printf("CALLBACK_TYPE_RECV_SUCESSED\r\n");
			break;
		case	CALLBACK_TYPE_RECV_FAILED:
			printf("CALLBACK_TYPE_RECV_FAILED\r\n");
			break;
		case	CALLBACK_TYPE_RECV_FILELIST:
			printf("CALLBACK_TYPE_RECV_FILELIST \r\n%s\r\n",object->CMDMsg);
			if(object->CMDMsg)
				printf("lenth (%d)\r\n",strlen(object->CMDMsg));
			break;
		case	CALLBACK_TYPE_RECV_FILEDEL:
			printf("CALLBACK_TYPE_RECV_FILEDEL (%s)\r\n",object->isSucessed ? "sucessed":"failed");
			break;
		case	CALLBACK_TYPE_RECV_DIRCREATE:
			printf("CALLBACK_TYPE_RECV_DIRCREATE (%s)\r\n",object->isSucessed ? "sucessed":"failed");
			break;
		case	CALLBACK_TYPE_RECV_DIRDEL:
			printf("CALLBACK_TYPE_RECV_DIRDEL (%s)\r\n",object->isSucessed ? "sucessed":"failed");
			break;
		case	CALLBACK_TYPE_SEND_ISOVERWRITE:
			printf("CALLBACK_TYPE_SEND_ISOVERWRITE overWRITE\r\n");
			object->isSucessed = true;
			break;
		case	CALLBACK_TYPE_RECV_ISOVERWRITE:
			printf("CALLBACK_TYPE_RECV_ISOVERWRITE overWRITE\r\n");
			object->isSucessed = true;
			break;
		default:
			printf("Error");
			break;
	}
}
//�ͻ���
int main (int argc, char *argv[]) 
{ 
	if(argc < 4) 
	{ 
		ACE_DEBUG((LM_DEBUG,"Usage %s <hostname><port_number><dir><send_filename>\n", argv[0])); 
		char c;
		c = getchar();
		ACE_OS::exit(1); 
	} 
	ACE_INET_Addr remote_addr_;
	ACE_DEBUG((LM_DEBUG,"Usage  <hostname>%s<port_number>%s<dir>%s<send_filename>%s\n",remote_addr_.get_host_name(),argv[2],argv[3],argv[4])); 	

	ACE::init();


	//for(int i = 0;i< 2000;i++)
	{
		//if(i % 2 == 0)
		{
			Client client;
			FWndCallBackHandle OnCallBack;
			OnCallBack = (FWndCallBackHandle)&OnCallBackImpl;

			if(client.connectServer(argv[1], ACE_OS::atoi(argv[2]),OnCallBack) == false)
			{
				ACE_DEBUG((LM_DEBUG,"Failed to connect to server!\n")); 
			}
			else
			{			
				//client.recvFileListFramServer("\\");
				client.sendFileToServer(argv[3],"\\");		
				//client.recvFileFrameServer("\\1\\TeamViewer.exe","");		
				//client.DirCreate("\\12312");
				//client.DirCreate("\\¼���ļ�","¼���ļ�ssddfsdfasdssdfsdf");
			}
		}
		/*else
		{
			Client client;
			FWndCallBackHandle OnCallBack;
			OnCallBack = (FWndCallBackHandle)&OnCallBackImpl;

			if(client.connectServer(argv[1], ACE_OS::atoi(argv[2]),OnCallBack) == false)
			{
				ACE_DEBUG((LM_DEBUG,"Failed to connect to server!\n")); 
			}
			else
			{			
				client.recvFileFrameServer("\\recv.pdf","");
			}
		}*/
	}	

	ACE_OS::printf("Press any key to exit");
	char c;
	c = getchar();
	c = getchar();	
	return 1;
} 
#endif //VER_CLIENT

#ifdef VER_DLL

BOOL WINAPI DllMain(HANDLE hModule, DWORD CallType, VOID* Reserved)
{
	switch (CallType) {
		case DLL_PROCESS_ATTACH:
			{			
				ACE::init();				
				//��־�����ļ�
				ACE_OSTREAM_TYPE *output = new ofstream("FileClient.txt", ios::app);

				ACE_LOG_MSG->msg_ostream (output, 1);    
				//���֮ǰ�ı�ǲ��������Ļ
				//ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);    
				ACE_LOG_MSG->set_flags (ACE_Log_Msg::STDERR);    	
				//������ļ�
				ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM);   

				ACE_DEBUG((LM_DEBUG,L"client start\n"));
			}
			break;
		//case DLL_THREAD_ATTACH:
		//case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			ACE_Reactor::instance()->end_event_loop();
			ACE_Reactor::instance()->close();
			ACE::fini(); 
			break;
	}
	return TRUE;
}
/*---------------------------------------------------------------------------------
ģ������:	�����ļ���������
����˵��:	IP:�ļ�������IP
port:�ļ���������¼�˿�
fileNamePath:Ϊ���·��,����:/¼�����/10024/2012-05-21.avi
callback:�ص�����
����ֵ:		���;��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int sendFileToServer(const char* ip,
							   int port,
							   const char* LocalfileNamePath,
							   const char* RemoteDirPath,
							   FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.sendFileToServer(LocalfileNamePath,RemoteDirPath);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
ģ������:	�����ļ�������
����˵��:	IP:�ļ�������IP
port:�ļ���������¼�˿�
RemotefileNamePath:Ϊ���������·��,����:/¼�����/10024/2012-05-21.avi
LocalDirPath:���ؽ����ļ���
callback:�ص�����
����ֵ:		���վ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileFramServer(const char* ip,
								 int port,
								 const char* RemotefileNamePath,
								 const char* LocalDirPath,
								 FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.recvFileFrameServer(RemotefileNamePath,LocalDirPath);
	else
		return -1;	
}
/*---------------------------------------------------------------------------------
ģ������:  ��ȡ�ļ��б�
����˵��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListFramServer(const char* ip,
									  int port,
									  const char* path,
									  FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.recvFileListFramServer(path);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
ģ������:  �������ڻ�ȡ�ļ��б�
����˵��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListByDateFramServer(const char* ip,
																 int port,
																 const char* path,
																 const char* beginDate,
																 const char* endDate,
																 FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.recvFileListByDateFramServer(path,beginDate,endDate);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
ģ������:  ����Ŀ¼
����˵��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool createDirFramServer(const char* ip,
																   int port,
																   const char* RemoteDirPath,
																   FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.DirCreate(RemoteDirPath);
	else
		return false;
}
/*---------------------------------------------------------------------------------
ģ������:  ɾ��Ŀ¼
����˵��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delDirFramServer(const char* ip,
								int port,
								const char* RemoteDirPath,
								FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.DirDel(RemoteDirPath);
	else
		return false;
}
/*---------------------------------------------------------------------------------
ģ������:  ɾ���ļ�
����˵��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delFileFramServer(const char* ip,
								 int port,
								 const char* RemoteFilePath,
								 FWndCallBackHandle callback)
{
	Client client;
	if(client.connectServer(ip,port,callback))
		return client.FileDel(RemoteFilePath);
	else
		return false;
}
/*---------------------------------------------------------------------------------
ģ������:  ֹͣ���ͻ�����ļ�
ģ�鹦��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void stopFileSendOrRecvByHandle(int Handle)
{
	Bass_Event_Handler* ptr = Client_List::Instens()->GetEventByHandle((ACE_HANDLE)Handle);
	if(ptr)
	{
		File_Recv_Event_Handler* recvPtr = dynamic_cast<File_Recv_Event_Handler*>(ptr);
		if (NULL != recvPtr)
			ACE_DEBUG((LM_DEBUG,L"stop file recv\n"));
		else
			ACE_DEBUG((LM_DEBUG,L"stop file send\n"));
		ptr->handle_close();		
	}		
}
/*---------------------------------------------------------------------------------
ģ������:  ��ȡ������������
ģ�鹦��:  �ɹ�����������/ʧ�ܷ���-1
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int getServerConnetedCount(const char* ip,
														   int port)
{
	Client client;
	if(client.connectServer(ip,port))
		return client.getConnectedCount();
	else
		return -1;
}
#endif //VER_DLL

