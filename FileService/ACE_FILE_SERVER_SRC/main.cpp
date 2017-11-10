/******************************************************************************
模块名称:  
模块功能:  按需要生成指定的程序(Server端/Client测试端/Dll模块)
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      生成dll: 1.修改ClientMothod.h的宏为VER_DLL	
					2.修改项目属性生成为.dll
					3.修改生成的dll文件名为SOCK_Connecter.dll,再编译即可
		   生成exe; 1.修改ClientMothod.h的宏为VER_SERVER	
					2.修改项目属性生成为.exe
					3.修改生成名为SOCK_Server.exe再编译即可;
 		   生成Linux版本的服务器:
					1.修改ClientMothod.h的宏为VER_SERVER
					2.需要在Linux下重新编译
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
	//日志输入文件
	ACE_OSTREAM_TYPE *output = new ofstream("FileServer.txt", ios::app);

	ACE_LOG_MSG->msg_ostream (output, 1);    
  
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::STDERR);    	
	//输出到文件
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM); 
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("Server begin\n")));

	//清除之前的标记并输出到屏幕
	//ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR); 

	int port  = argc >= 2 ? ACE_OS::atoi(argv[1]) : 19998;

	//初始化系统目录
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
//回调函数
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
//客户端
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
				//client.DirCreate("\\录相文件","录相文件ssddfsdfasdssdfsdf");
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
				//日志输入文件
				ACE_OSTREAM_TYPE *output = new ofstream("FileClient.txt", ios::app);

				ACE_LOG_MSG->msg_ostream (output, 1);    
				//清除之前的标记并输出到屏幕
				//ACE_LOG_MSG->clr_flags (ACE_Log_Msg::STDERR);    
				ACE_LOG_MSG->set_flags (ACE_Log_Msg::STDERR);    	
				//输出到文件
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
模块名称:	发送文件到服务器
参数说明:	IP:文件服务器IP
port:文件服务器登录端口
fileNamePath:为相对路径,例如:/录相管理/10024/2012-05-21.avi
callback:回调函数
返回值:		发送句柄
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
模块名称:	接收文件到本地
参数说明:	IP:文件服务器IP
port:文件服务器登录端口
RemotefileNamePath:为服务器相对路径,例如:/录相管理/10024/2012-05-21.avi
LocalDirPath:本地接收文件夹
callback:回调函数
返回值:		接收句柄
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
模块名称:  获取文件列表
参数说明:  
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
模块名称:  根据日期获取文件列表
参数说明:  
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
模块名称:  创建目录
参数说明:  
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
模块名称:  删除目录
参数说明:  
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
模块名称:  删除文件
参数说明:  
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
模块名称:  停止发送或接收文件
模块功能:  
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
模块名称:  获取服务器连接数
模块功能:  成功返回连接数/失败返回-1
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

