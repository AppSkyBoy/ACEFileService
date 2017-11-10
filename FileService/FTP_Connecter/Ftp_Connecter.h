#ifndef	FTP_CONNECTER_H
#define FTP_CONNECTER_H

#include <string>
#include <map>
#include <afx.h>
#include "ClientMothod_FTP.h"
#include <afxinet.h>

using namespace std;
/******************************************************************************
模块名称:  CallBackFunction:
模块功能:  事件回调事件
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      这里触发的事件会更新到Client注册的事件中

因为c++不支持类函数回调，就没有在派生类中实现回调函数
需要的事件是通过CallBackFunction里面的方法通知界面改变的(文件发送进度，成功与否的事件
都是这个对象来通知界面的
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
	//发送进度
	void OnFRProgressCallBackImpl(int iMax,int iProgress);
	//发送成功
	void OnFRSucessedCallBackImpl();
	//发送失败
	void OnFRFailedCallBackImpl();
	//接收进度
	void OnFSProgressCallBackImpl(int iMax,int iProgress);
	//接收失败
	void OnFSSucessedCallBackImpl();
	//接收成功
	void OnFSFailedCallBackImpl();
	//获取指定路径的文件列表
	void OnGetFileListByPathImpl(const char* text,bool isSucessed = true);
	//删除文件
	void OnDelFileImpl(bool isSucessed = true);
	//删除目录
	void OnDelDirImpl(bool isSucessed = true);
	//创建目录
	void OnCreateDirImpl(bool isSucessed = true);
	//发送文件时的服务器同名文件处理
	bool OnFSOverWriterCallBackImpl(char* text);
	//接收文件时的服务器同名文件处理
	bool OnFROverWriterCallBackImpl(char* text);
	//根据日期获取指定路径的文件列表
	void OnGetFileListByDateAndPathImpl(const char* text,bool isSucessed = true);
};
/******************************************************************************
模块名称:  FTP上传或下载助手类
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      上传和下载是很耗时的,需要放到线程离处理;
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
				OutputDebugString("ThreadParam 析构出错");
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
模块名称:  当前的下载或上传线程列表
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      这是一个线程同步类，用于停止指定线程的下载和上传
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
模块名称:  编码转换函数
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      FTP对象参数使用的是UTF8编码，这里需要转换下
*****************************************************************************/
class Encode
{
public:
	static int ConvUtf8ToAnsi(CString& strSource, CString& strChAnsi);
	static CString UnicodeToUTF8(const char* pSrc, int nLen);
};
/******************************************************************************
模块名称:  FTP连接模块
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:     成功/失败(连接)
*****************************************************************************/
class Ftp_Connecter
{
private:
	string IP;
	int Port;
	string userName;
	string passWord;

	CInternetSession	m_pInetSession;			//会话对象
	CFtpConnection*		m_pFtpConnection;		//连接对象
	CFileFind			m_LocalFinder;			//本地查找文件对象
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