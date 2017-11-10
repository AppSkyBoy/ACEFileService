#ifndef	FTP_CONNECTER_H
#define FTP_CONNECTER_H

#include <string>
#include <map>
#include <afx.h>
#include "ClientMothod_FTP.h"
#include <afxinet.h>

using namespace std;
/******************************************************************************
ģ������:  CallBackFunction:
ģ�鹦��:  �¼��ص��¼�
������:    ��
��������:  2012/10
��������:
��ע:      ���ﴥ�����¼�����µ�Clientע����¼���

��Ϊc++��֧���ຯ���ص�����û������������ʵ�ֻص�����
��Ҫ���¼���ͨ��CallBackFunction����ķ���֪ͨ����ı��(�ļ����ͽ��ȣ��ɹ������¼�
�������������֪ͨ�����
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
	void OnGetFileListByPathImpl(const char* text,bool isSucessed = true);
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
	void OnGetFileListByDateAndPathImpl(const char* text,bool isSucessed = true);
};
/******************************************************************************
ģ������:  FTP�ϴ�������������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      �ϴ��������Ǻܺ�ʱ��,��Ҫ�ŵ��߳��봦��;
*****************************************************************************/
class ThreadHelper
{
public:
	class ThreadParam
	{
	private:
		CRITICAL_SECTION m_csLock;
		bool	m_isTerminated;
	public:
		CFile* pFile;
		CInternetFile* pRemoteFile;
		CInternetSession* pInetSession;
		CFtpConnection*  pFtpConnection;
		CallBackFunction Fcallback;
		bool isDown;  	 
		ULONGLONG iFtpfileSize;
		HANDLE handle;
		ThreadParam(CFile* pFile,CInternetFile * pRemoteFile,CInternetSession* pInetSession,CFtpConnection*  pFtpConnection,FWndCallBackHandle callback,bool isDown = true,ULONGLONG iFtpfileSize = 0)
		{
			::InitializeCriticalSection(&m_csLock);
			this->Fcallback.InitWndCallBack(callback,0);
			this->pFile = pFile;
			this->pRemoteFile = pRemoteFile;
			this->isDown = isDown;			
			this->m_isTerminated = false;
			this->iFtpfileSize = iFtpfileSize;
			this->pInetSession = pInetSession;
			this->pFtpConnection = pFtpConnection;
			this->handle = NULL;
		}
		void InitHandle(HANDLE handle)
		{
			this->Fcallback.InitWndCallBack(this->Fcallback.FWndcallback,(int)handle);
			this->handle = handle;
		}
		~ThreadParam()
		{
			try
			{
				DeleteCriticalSection(&m_csLock);
				delete pRemoteFile;
				delete pFile;
				delete pInetSession;
			}catch(...)
			{
				OutputDebugString("ThreadParam ��������");
			}
		}
		bool isTerminated()
		{
			bool isRet = false;
			::EnterCriticalSection(&m_csLock);
			isRet = m_isTerminated;
			::LeaveCriticalSection(&m_csLock); 
			return isRet;
		}
		void Terminated()
		{
			::EnterCriticalSection(&m_csLock);
			m_isTerminated = true;
			::LeaveCriticalSection(&m_csLock); 
		}
	};
private:	
	ThreadHelper(){}
protected:
	static DWORD WINAPI FileSendOrDownThread(LPVOID lpParam);
public:
	static ThreadHelper* InstensPtr();
	HANDLE SendFileThreadBegin(CFile* pFile,CInternetFile * pRemoteFile,CInternetSession* pInetSession,CFtpConnection*  pFtpConnection,FWndCallBackHandle callback);
	HANDLE DownFileThreadBegin(CFile* pFile,CInternetFile * pRemoteFile,CInternetSession* pInetSession,CFtpConnection*  pFtpConnection,FWndCallBackHandle callback,ULONGLONG iTotalfileSize);
	void   StopSendOrDownFileByHandle(HANDLE handle);
};
/******************************************************************************
ģ������:  ��ǰ�����ػ��ϴ��߳��б�
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      ����һ���߳�ͬ���࣬����ָֹͣ���̵߳����غ��ϴ�
*****************************************************************************/
class SyncList
{
private:
	map<HANDLE,ThreadHelper::ThreadParam*>CurMap;
	CRITICAL_SECTION m_csLock;
private:
	SyncList();	
public:
	~SyncList();
	static SyncList* InstensPtr();
	int GetCount();
	void Clear();
	ThreadHelper::ThreadParam* GetParamByHandle(HANDLE handle);
	void AddPram(HANDLE handle,ThreadHelper::ThreadParam* param);
	ThreadHelper::ThreadParam* RemoveByHandle(HANDLE handle);
};
/******************************************************************************
ģ������:  ����ת������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      FTP�������ʹ�õ���UTF8���룬������Ҫת����
*****************************************************************************/
class Encode
{
public:
	static int ConvUtf8ToAnsi(CString& strSource, CString& strChAnsi);
	static CString UnicodeToUTF8(const char* pSrc, int nLen);
};
/******************************************************************************
ģ������:  FTP����ģ��
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:     �ɹ�/ʧ��(����)
*****************************************************************************/
class Ftp_Connecter
{
private:
	string IP;
	int Port;
	string userName;
	string passWord;

	CInternetSession	m_pInetSession;			//�Ự����
	CFtpConnection*		m_pFtpConnection;		//���Ӷ���
	CFileFind			m_LocalFinder;			//���ز����ļ�����
protected:
	bool login();
	void logout();
public:	
	void InitConnected(const char* IP,int Port,const char* userName,const char* passWord);
	Ftp_Connecter();
	~Ftp_Connecter(){this->logout();}
	static Ftp_Connecter* InstensPtr();
	int  sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath,FWndCallBackHandle callback);
	int  recvFileFramServer(const char* RemotefileNamePath,const char* LocalDirPath,FWndCallBackHandle callback);
	int  recvFileListFramServer(const char* RemoteDirPath,FWndCallBackHandle callback);
	int  recvFileListByDateFramServer(const char* RemoteDirPath,const char* beginDate,const char* endDate,FWndCallBackHandle callback);
	bool createDirFramServer(const char* RemoteDirPath,FWndCallBackHandle callback);
	bool delDirFramServer(const char* RemoteDirPath,FWndCallBackHandle callback);
	bool delFileFramServer(const char* RemoteFilePath,FWndCallBackHandle callback);
	void stopFileSendOrRecvByHandle(int Handle);
	bool  CheckedConnected();
};
#endif