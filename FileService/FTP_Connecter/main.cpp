/******************************************************************************
ģ������:  
ģ�鹦��:  ����Ҫ����ָ���ĳ���(Server��/Client���Զ�/Dllģ��)
������:    ��
��������:  2012/10
��������:
��ע:      ����dll����: 1.�޸�ClientMothod.h�ĺ�ΪVER_DLL
						2.�޸���Ŀ��������Ϊ.dll
						3.�޸������ļ�ΪXXX.exe
		   ����exe���෴;
******************************************************************************/
#include "stdafx.h"
#include "ClientMothod_FTP.h"
#include "Ftp_Connecter.h"



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
ģ������:	�����ļ���������
����˵��:	IP:�ļ�������IP
port:�ļ���������¼�˿�
fileNamePath:Ϊ���·��,����:/¼�����/10024/2012-05-21.avi
callback:�ص�����
����ֵ:		���;��
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
ģ������:	�����ļ�������
����˵��:	IP:�ļ�������IP
port:�ļ���������¼�˿�
RemotefileNamePath:Ϊ���������·��,����:/¼�����/10024/2012-05-21.avi
LocalDirPath:���ؽ����ļ���
callback:�ص�����
����ֵ:		���վ��
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
ģ������:  ��ȡ�ļ��б�
����˵��:  
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
ģ������:  �������ڻ�ȡ�ļ��б�
����˵��:  
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
ģ������:  ����Ŀ¼
����˵��:  
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
ģ������:  ɾ��Ŀ¼
����˵��:  
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
ģ������:  ɾ���ļ�
����˵��:  
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
ģ������:  ֹͣ���ͻ�����ļ�
ģ�鹦��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPstopFileSendOrRecvByHandle(int Handle)
{
	//if(Ftp_Connecter::InstensPtr()->CheckedConnected())
	if(Handle != -1)
		Ftp_Connecter::InstensPtr()->stopFileSendOrRecvByHandle(Handle);	
}
/*---------------------------------------------------------------------------------
ģ������:  ��ȡ������������
ģ�鹦��:  �ɹ�����������/ʧ�ܷ���-1
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPgetServerConnetedCount()
{
	if(Ftp_Connecter::InstensPtr()->CheckedConnected())
		return SyncList::InstensPtr()->GetCount();
	else 
		return -1;
}
/*---------------------------------------------------------------------------------
ģ������:  ��ʼ������
ģ�鹦��:  �ɹ�/ʧ��(����)
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPInitConnected(const char* IP,int Port,const char* userName,const char* password)
{
	Ftp_Connecter::InstensPtr()->InitConnected(IP,Port,userName,password);
}
#endif //VER_DLL
