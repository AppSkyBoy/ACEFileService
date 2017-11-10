/******************************************************************************
模块名称:  
模块功能:  按需要生成指定的程序(Server端/Client测试端/Dll模块)
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      生成dll方法: 1.修改ClientMothod.h的宏为VER_DLL
						2.修改项目属性生成为.dll
						3.修改生成文件为XXX.exe
		   生成exe则相反;
******************************************************************************/
#include "stdafx.h"
#include "ClientMothod_FTP.h"
#include "Ftp_Connecter.h"



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

	ACE_OS::printf("Press any key to exit");
	char c;
	c = getchar();
	c = getchar();	
	return 1;
} 
#endif //VER_CLIENT

#ifdef VER_DLL
/*BOOL WINAPI DllMain(HANDLE hModule, DWORD CallType, VOID* Reserved)
{
	switch (CallType) {
		case DLL_PROCESS_ATTACH:
			{	
				Ftp_Connecter::InstensPtr();
			}
			break;
		//case DLL_THREAD_ATTACH:
		//case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}*/
/*---------------------------------------------------------------------------------
模块名称:	发送文件到服务器
参数说明:	IP:文件服务器IP
port:文件服务器登录端口
fileNamePath:为相对路径,例如:/录相管理/10024/2012-05-21.avi
callback:回调函数
返回值:		发送句柄
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPsendFileToServer(
							   const char* LocalfileNamePath,
							   const char* RemoteDirPath,
							   FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->sendFileToServer(LocalfileNamePath,RemoteDirPath,callback);
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
extern "C" _declspec(dllexport) int FTPrecvFileFramServer(
								 const char* RemotefileNamePath,
								 const char* LocalDirPath,
								 FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->recvFileFramServer(RemotefileNamePath,LocalDirPath,callback);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
模块名称:  获取文件列表
参数说明:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPrecvFileListFramServer(
									  const char* path,
									  FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->recvFileListFramServer(path,callback);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
模块名称:  根据日期获取文件列表
参数说明:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPrecvFileListByDateFramServer(
																 const char* path,
																 const char* beginDate,
																 const char* endDate,
																 FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->recvFileListByDateFramServer(path,beginDate,endDate,callback);
	else
		return -1;
}
/*---------------------------------------------------------------------------------
模块名称:  创建目录
参数说明:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPcreateDirFramServer(
																   const char* RemoteDirPath,
																   FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->createDirFramServer(RemoteDirPath,callback);
	else
		return false;
}
/*---------------------------------------------------------------------------------
模块名称:  删除目录
参数说明:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPdelDirFramServer(
								const char* RemoteDirPath,
								FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->delDirFramServer(RemoteDirPath,callback);
	else
		return false;
}
/*---------------------------------------------------------------------------------
模块名称:  删除文件
参数说明:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPdelFileFramServer(
								 const char* RemoteFilePath,
								 FWndCallBackHandle callback)
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return Ftp_Connecter::InstensPtr()->delFileFramServer(RemoteFilePath,callback);
	else
		return false;
}
/*---------------------------------------------------------------------------------
模块名称:  停止发送或接收文件
模块功能:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPstopFileSendOrRecvByHandle(int Handle)
{
	//if(Ftp_Connecter::InstensPtr()->CheckedConnected())
	if(Handle != -1)
		Ftp_Connecter::InstensPtr()->stopFileSendOrRecvByHandle(Handle);	
}
/*---------------------------------------------------------------------------------
模块名称:  获取服务器连接数
模块功能:  成功返回连接数/失败返回-1
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPgetServerConnetedCount()
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return SyncList::InstensPtr()->GetCount();
	else 
		return -1;
}
/*---------------------------------------------------------------------------------
模块名称:  初始化连接
模块功能:  成功/失败(连接)
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPInitConnected(const char* IP,int Port,const char* userName,const char* password)
{
	Ftp_Connecter::InstensPtr()->InitConnected(IP,Port,userName,password);
}
#endif //VER_DLL
