#ifndef CLIENTMOTHOD_FTP_H
#define CLIENTMOTHOD_FTP_H

//#define VER_CLIENT
#define VER_DLL



/******************************************************************************
ģ������:  DLL��������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:     
*****************************************************************************/
struct FWndCallBackObject
{
	char*	CMDMsg;				//һ������Ϣ����ֵ�������Ϣ
	int		iProgress;			//����/�ϴ�����(0~100)
	bool	isSucessed;			//CMD�Ƿ�ִ�гɹ�(�紴��Ŀ¼��ɾ���ļ�/Ŀ¼��)
	int		HANDLE;				//��Ӧ���صľ��ֵ(��send��recv�ļ������ķ��ؾ��ֵ)
};

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
extern "C" _declspec(dllexport) int FTPsendFileToServer(
								const char* LocalfileNamePath,
								const char* RemoteDirPath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:	�����ļ�������
����˵��:	RemotefileNamePath:Ϊ���������·��,����:/¼�����/10024/2012-05-21.avi
			LocalDirPath:���ؽ����ļ���
			callback:�ص�����
����ֵ:		ʧ��-1/�ɹ����ؾ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPrecvFileFramServer(
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
extern "C" _declspec(dllexport) int FTPrecvFileListFramServer(
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
extern "C" _declspec(dllexport) int FTPrecvFileListByDateFramServer(
									  const char* path,
									  const char* beginDate,
									  const char* endDate,
									  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ����Ŀ¼
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPcreateDirFramServer(
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ɾ��Ŀ¼
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPdelDirFramServer(
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ɾ���ļ�
����˵��:  
����ֵ:		�ɹ�/ʧ��
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool FTPdelFileFramServer(
								const char* RemoteFilePath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
ģ������:  ֹͣ���ͻ�����ļ�
ģ�鹦��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPstopFileSendOrRecvByHandle(int Handle);
/*---------------------------------------------------------------------------------
ģ������:  ��������
ģ�鹦��:  �ɹ�����������/ʧ�ܷ���-1(FTP���صĲ��Ƿ�����������)
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int FTPgetServerConnetedCount();
/*---------------------------------------------------------------------------------
ģ������:  ��ʼ������
ģ�鹦��:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void FTPInitConnected(const char* IP,int Port,const char* userName,const char* password);
#endif

#endif

