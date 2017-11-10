#ifndef CLIENTMOTHOD_H
#define CLIENTMOTHOD_H

//#define VER_SERVER	//�ļ���������(Linux�汾��Ҫ��Linux�����±���)  (����������ΪSOCK_Service.dll)
//#define VER_CLIENT    //�ͻ��ˣãͣİ汾,���ڲ���ʹ�� (����SOCK_Server_Client.exe�����ɺ���startClient.batִ�в���)
#define VER_DLL         //�ͻ������ӽӿ�,���ڶ��ο���  (����������ΪSOCK_Connecter.dll)



/******************************************************************************
ģ������:  DLL��������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:     
��ע:      ����dll: 1.�޸�ClientMothod.h�ĺ�ΪVER_DLL	
					2.�޸���Ŀ��������Ϊ.dll
					3.�޸����ɵ�dll�ļ���ΪSOCK_Connecter.dll,�ٱ��뼴��
		   ����exe; 1.�޸�ClientMothod.h�ĺ�ΪVER_SERVER	
					2.�޸���Ŀ��������Ϊ.exe
					3.�޸�������ΪSOCK_Server.exe�ٱ��뼴��;
 		   ����Linux�汾�ķ�����:
					1.�޸�ClientMothod.h�ĺ�ΪVER_SERVER
					2.��Ҫ��Linux�����±���
*****************************************************************************/
struct FWndCallBackObject
{
	char*	CMDMsg;				//һ������Ϣ����ֵ�������Ϣ
	int		iProgress;			//����/�ϴ�����(0~100)
	bool	isSucessed;			//CMD�Ƿ�ִ�гɹ�(�紴��Ŀ¼��ɾ���ļ�/Ŀ¼��)
	int		HANDLE;				//��Ӧ���صľ��ֵ(��send��recv�ļ������ķ��ؾ��ֵ)
};

#ifdef VER_SERVER
typedef void( *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

#ifdef VER_CLIENT
typedef void(__stdcall *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

#ifdef VER_DLL
typedef void(__stdcall *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

/*---------------------------------------------------------------------------------
˵��:  �ص�����
/*---------------------------------------------------------------------------------*/
#define CALLBACK_TYPE_SEND_PROGRESS 1001
#define CALLBACK_TYPE_SEND_SUCESSED	1002
#define CALLBACK_TYPE_SEND_FAILED	1003
#define CALLBACK_TYPE_RECV_PROGRESS 1004
#define CALLBACK_TYPE_RECV_SUCESSED	1005
#define CALLBACK_TYPE_RECV_FAILED	1006
#define CALLBACK_TYPE_RECV_FILELIST 1007
#define CALLBACK_TYPE_RECV_FILEDEL  1008
#define CALLBACK_TYPE_RECV_DIRCREATE 1009
#define CALLBACK_TYPE_RECV_DIRDEL    1010
//ͬ���ļ�����(��ʾ��Ϣ��CMDMsg��,����isSucessed�����жϽ��)
#define CALLBACK_TYPE_SEND_ISOVERWRITE	1011
#define CALLBACK_TYPE_RECV_ISOVERWRITE	1012
#define CALLBACK_TYPE_RECV_FILELIST_BYDATE 1013

#ifdef VER_DLL
/*---------------------------------------------------------------------------------
ģ������:	�����ļ���������
����˵��:	IP:�ļ�������IP
			port:�ļ���������¼�˿�
			fileNamePath:Ϊ���·��,����:/¼�����/10024/2012-05-21.avi
			callback:�ص�����
����ֵ:		ʧ��-1/�ɹ����ؾ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int sendFileToServer(const char* ip,
								int port,
								const char* LocalfileNamePath,
								const char* RemoteDirPath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:	�����ļ�������
����˵��:	IP:�ļ�������IP
			port:�ļ���������¼�˿�
			RemotefileNamePath:Ϊ���������·��,����:/¼�����/10024/2012-05-21.avi
			LocalDirPath:���ؽ����ļ���
			callback:�ص�����
����ֵ:		ʧ��-1/�ɹ����ؾ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileFramServer(const char* ip,
								  int port,
								  const char* RemotefileNamePath,
								  const char* LocalDirPath,
								  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ��ȡ�ļ��б�
����˵��:  
����ֵ:		ʧ��-1/�ɹ��򷵻�0(��Ϣ����ͨ��CALLBACK_TYPE_RECV_FILELIST����
//DirFileListXml��XML�ṹ����:
/*
<DFList>
     <DList>
        <Ditm>
              <dp></ dp >  //dirname
			  <tp></ tp >   //type
		      <ct></ ct>   //changename
	</ Ditm >
��
     </ DList >
     < FList >
        <Fitm>
            <fp></fp>     //filename
            <size></size >  //filesize
			<tp></tp>     //type
			<ct></ct>     //changename
        </ Fitm >
        �� 
     </FList>
</ DFList >
*/
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListFramServer(const char* ip,
									  int port,
									  const char* path,
									  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ����ʱ��̻�ȡ�ļ��б�
����˵��:  beginDate/endDateʱ���ʽ:20121206141817			
����ֵ:		ʧ��-1/�ɹ��򷵻�0(��Ϣ����ͨ��CALLBACK_TYPE_RECV_FILELIST����
//DirFileListXml��XML�ṹ����:
/*
<DFList>
     < FList >
        <Fitm>
            <fp></fp>     //filename
            <size></size >  //filesize
			<tp></tp>     //type
			<ct></ct>     //changename
        </ Fitm >
        �� 
     </FList>
</ DFList >
*/
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListByDateFramServer(const char* ip,
									  int port,
									  const char* path,
									  const char* beginDate,
									  const char* endDate,
									  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ����Ŀ¼
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool createDirFramServer(const char* ip,
								   int port,
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ɾ��Ŀ¼
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delDirFramServer(const char* ip,
								   int port,
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ɾ���ļ�
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delFileFramServer(const char* ip,
								int port,
								const char* RemoteFilePath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ֹͣ���ͻ�����ļ�
ģ�鹦��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void stopFileSendOrRecvByHandle(int Handle);
/*---------------------------------------------------------------------------------
ģ������:  ��ȡ������������
ģ�鹦��:  �ɹ�����������/ʧ�ܷ���-1
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int getServerConnetedCount(const char* ip,
														   int port);
#endif

#endif

