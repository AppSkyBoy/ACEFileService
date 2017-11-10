#include "stdafx.h"
#include "Ftp_Connecter.h"
#include <time.h>

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
CallBackFunction::CallBackFunction()
{
	this->FWndcallback = NULL; 
	this->Fobject = new FWndCallBackObject();
	this->Fobject->CMDMsg = NULL;
	this->Fobject->iProgress = 0;
	this->Fobject->isSucessed = false;
	this->Fobject->HANDLE = 0;
}
CallBackFunction::~CallBackFunction()
{
	delete this->Fobject;
	this->Fobject = NULL;
}
void CallBackFunction::InitWndCallBack(FWndCallBackHandle callback,int handle)
{
	this->FWndcallback = callback;
	this->Fobject->HANDLE = handle;	
}
void CallBackFunction::OnFRProgressCallBackImpl(int iMax,int iProgress)
{
	Fobject->iProgress = iProgress;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_PROGRESS,Fobject);
}
void CallBackFunction::OnFRSucessedCallBackImpl()
{
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_SUCESSED,Fobject);
}
void CallBackFunction::OnFRFailedCallBackImpl()
{
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FAILED,Fobject);
}
void CallBackFunction::OnFSProgressCallBackImpl(int iMax,int iProgress)
{
	Fobject->iProgress = iProgress;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_SEND_PROGRESS,Fobject);
}
void CallBackFunction::OnFSSucessedCallBackImpl()
{
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_SEND_SUCESSED,Fobject);
}
void CallBackFunction::OnFSFailedCallBackImpl()
{
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_SEND_FAILED,Fobject);
}
//根据日期获取指定路径的文件列表
void CallBackFunction::OnGetFileListByPathImpl(const char* text,bool isSucessed)
{
	//ACE_DEBUG ((LM_ERROR,L"3OnGetFileListByPathImpl(%s)\n", text)); 

	//char转wchar_t
	//size_t len = strlen(text) + 1;
	//size_t converted = 0;
	//Fobject->CMDMsg=(wchar_t*)malloc(len*sizeof(wchar_t));
	//mbstowcs_s(&converted, Fobject->CMDMsg, len, text, _TRUNCATE);

	Fobject->CMDMsg = const_cast<char*>(text);	
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FILELIST,Fobject);
}
void CallBackFunction::OnGetFileListByDateAndPathImpl(const char* text,bool isSucessed)
{
	Fobject->CMDMsg = const_cast<char*>(text);	
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FILELIST_BYDATE,Fobject);
}
//删除文件
void CallBackFunction::OnDelFileImpl(bool isSucessed )
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FILEDEL,Fobject);
}
//删除目录
void CallBackFunction::OnDelDirImpl(bool isSucessed)
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_DIRDEL,Fobject);
}
//创建目录
void CallBackFunction::OnCreateDirImpl(bool isSucessed)
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_DIRCREATE,Fobject);
}
//发送文件时的服务器同名文件处理
bool CallBackFunction::OnFSOverWriterCallBackImpl(char* text)
{
	Fobject->isSucessed = false;
	Fobject->CMDMsg = text;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_SEND_ISOVERWRITE,Fobject);
	return Fobject->isSucessed;
}
//接收文件时的服务器同名文件处理
bool CallBackFunction::OnFROverWriterCallBackImpl(char* text)
{
	Fobject->isSucessed = false;
	Fobject->CMDMsg = text;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_ISOVERWRITE,Fobject);
	return Fobject->isSucessed;
}
/******************************************************************************
模块名称:  当前的下载或上传线程列表
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      这是一个线程同步类，用于停止指定线程的下载和上传
*****************************************************************************/
SyncList::SyncList()
{
	::InitializeCriticalSection(&m_csLock);
}
SyncList::~SyncList()
{
	this->Clear();
}
SyncList* SyncList::InstensPtr()
{
	static SyncList* ptr = NULL;
	if(!ptr)
		ptr = new SyncList();
	return ptr;
}
int SyncList::GetCount()
{
	int iRet = 0;
	::EnterCriticalSection(&m_csLock);
	iRet = this->CurMap.size();
	::LeaveCriticalSection(&m_csLock); 
	return iRet;
}
void SyncList::Clear()
{
	::EnterCriticalSection(&m_csLock);
	_try
	{
		for (map <HANDLE,ThreadHelper::ThreadParam*> ::iterator i = this->CurMap.begin(); i != this->CurMap.end(); i++)
		{
			ThreadHelper::ThreadParam* param = i->second;
			param->Terminated();
		}
		this->CurMap.clear();
	}
	__finally
	{
		::LeaveCriticalSection(&m_csLock); 
	}
}
ThreadHelper::ThreadParam* SyncList::GetParamByHandle(HANDLE handle)
{
	ThreadHelper::ThreadParam* ptr= NULL;
	::EnterCriticalSection(&m_csLock);
	_try
	{
		map<HANDLE,ThreadHelper::ThreadParam*>::iterator iter = this->CurMap.find(handle);
		if( this->CurMap.end() != iter )		
			ptr= iter->second;
	}
	__finally
	{
		::LeaveCriticalSection(&m_csLock); 
	}
	return ptr;
}
void SyncList::AddPram(HANDLE handle,ThreadHelper::ThreadParam* param)
{
	if(NULL == handle)
		return;

	::EnterCriticalSection(&m_csLock);
	_try
	{
		map<HANDLE,ThreadHelper::ThreadParam*>::iterator iter = this->CurMap.find(handle);
		if( this->CurMap.end() == iter )		
			this->CurMap.insert(map<HANDLE,ThreadHelper::ThreadParam*>::value_type(handle,param));
	}
	__finally
	{
		::LeaveCriticalSection(&m_csLock); 
	}
}
ThreadHelper::ThreadParam* SyncList::RemoveByHandle(HANDLE handle)
{
	if(NULL == handle)
		return NULL;

	ThreadHelper::ThreadParam* param = NULL;
	::EnterCriticalSection(&m_csLock);
	_try
	{
		map<HANDLE,ThreadHelper::ThreadParam*>::iterator iter = this->CurMap.find(handle);
		if( this->CurMap.end() != iter )
		{
			param = iter->second;			
			this->CurMap.erase(iter);
		}
	}
	__finally
	{
		::LeaveCriticalSection(&m_csLock); 
	}
	return param;
}
/******************************************************************************
模块名称:  FTP上传或下载助手类
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      上传和下载是很耗时的,需要放到线程离处理;
*****************************************************************************/
ThreadHelper* ThreadHelper::InstensPtr()
{
	static ThreadHelper* ptr = NULL;
	if(!ptr)
	{
		ptr = new ThreadHelper();
	}
	return ptr;
}

HANDLE ThreadHelper::SendFileThreadBegin(CFile* pFile,CInternetFile * pRemoteFile,CInternetSession* pInetSession,CFtpConnection*  pFtpConnection,FWndCallBackHandle callback)
{
	DWORD CurdwThreadId;
	ThreadHelper::ThreadParam* ptr = new ThreadHelper::ThreadParam(pFile,pRemoteFile,pInetSession,pFtpConnection,callback,false);
	HANDLE handle = CreateThread(NULL, 0, FileSendOrDownThread, (LPVOID)ptr, THREAD_PRIORITY_NORMAL, &CurdwThreadId);
	ptr->InitHandle(handle);
	SyncList::InstensPtr()->AddPram(handle,ptr);
	return handle;
}

HANDLE ThreadHelper::DownFileThreadBegin(CFile* pFile,CInternetFile * pRemoteFile,CInternetSession* pInetSession,CFtpConnection*  pFtpConnection,FWndCallBackHandle callback,ULONGLONG iTotalfileSize)
{
	DWORD CurdwThreadId;
	ThreadHelper::ThreadParam* ptr = new ThreadHelper::ThreadParam(pFile,pRemoteFile,pInetSession,pFtpConnection,callback,true,iTotalfileSize);	
	HANDLE handle = CreateThread(NULL, 0, FileSendOrDownThread, (LPVOID)ptr, THREAD_PRIORITY_NORMAL, &CurdwThreadId);
	ptr->InitHandle(handle);
	SyncList::InstensPtr()->AddPram(handle,ptr);
	return handle;
}
const int MAX_BUFFER_SIZE = 8*1024;
DWORD WINAPI ThreadHelper::FileSendOrDownThread(LPVOID lpParam)
{
	ThreadHelper::ThreadParam* ptr = (ThreadHelper::ThreadParam*)lpParam;
	if(ptr->isDown)
	{//下载
		try
		{
			char pBuf[MAX_BUFFER_SIZE];  
			memset(pBuf, 0, MAX_BUFFER_SIZE); 			
			ULONGLONG iTotalfileSize =  ptr->iFtpfileSize;//GetLength()这个函数获取的是缓冲区长度;
			DWORD dwRead = 0,iTotalRecvSize = 0; 
			int iCurPrgress = 0,iLastPrgress = 0;
			char buff[40];
			while((dwRead = ptr->pRemoteFile->Read(pBuf, MAX_BUFFER_SIZE)) > 0 && !ptr->isTerminated())  
			{  
				ptr->pFile->Write(pBuf, dwRead );  
				iTotalRecvSize += dwRead;

				//更新进度
				iCurPrgress = iTotalRecvSize >0 ? int((double)iTotalRecvSize / (double)iTotalfileSize * (double)100.0):0;	
				if(iLastPrgress != iCurPrgress)
				{		
					iLastPrgress = iCurPrgress;
					ptr->Fcallback.OnFRProgressCallBackImpl(100,iLastPrgress);				
				}		   	
			} 
			CString fileName = ptr->pFile->GetFilePath();
			ptr->pFile->Close();          
			ptr->pRemoteFile->Close();
			ptr->pFtpConnection->Close();
			ptr->pInetSession->Close();
			
			if(iTotalRecvSize == iTotalfileSize)
				ptr->Fcallback.OnFRSucessedCallBackImpl();
			else
			{				
				ptr->Fcallback.OnFRFailedCallBackImpl();
				//自动删除
				::DeleteFile((LPCTSTR)fileName);
			}
		}
		catch(CInternetException *pEx)  
		{  		
			ptr->pFile->Close();          
			ptr->pRemoteFile->Close();
			ptr->pFtpConnection->Close();
			ptr->pInetSession->Close();

			char m_errorMsg[1024]; 
			pEx->GetErrorMessage( m_errorMsg, 1024 );
			OutputDebugString(m_errorMsg);
			ptr->Fcallback.OnFRFailedCallBackImpl();
		}	
	}
	else 
	{//上传
		try
		{
			char pBuf[MAX_BUFFER_SIZE];  
			memset(pBuf, 0, MAX_BUFFER_SIZE); 
			ULONGLONG iTotalfileSize =  ptr->pFile->GetLength(); 
			DWORD dwRead = 0,iTotalUpSize = 0; 
			int iCurPrgress = 0,iLastPrgress = 0;

			while((dwRead = ptr->pFile->Read(pBuf, MAX_BUFFER_SIZE)) > 0 && !ptr->isTerminated())  
			{  
				ptr->pRemoteFile->Write(pBuf, dwRead );  
				iTotalUpSize += dwRead;

				//更新进度
				iCurPrgress = iTotalfileSize >0 ? int((double)iTotalUpSize / (double)iTotalfileSize * (double)100.0):0;	
				if(iLastPrgress != iCurPrgress)
				{		
					iLastPrgress = iCurPrgress;
					ptr->Fcallback.OnFSProgressCallBackImpl(100,iLastPrgress);				
				}		   	
			}
			//这里要先关闭，不燃后面的事件会处理不了文件;
			ptr->pFile->Close();          
			ptr->pRemoteFile->Close();
			ptr->pFtpConnection->Close();
			ptr->pInetSession->Close();

			if(iTotalUpSize == iTotalfileSize)
				ptr->Fcallback.OnFSSucessedCallBackImpl();
			else
				ptr->Fcallback.OnFSFailedCallBackImpl();
		}
		catch(CInternetException *pEx)  
		{  		
			ptr->pFile->Close();          
			ptr->pRemoteFile->Close();
			ptr->pFtpConnection->Close();
			ptr->pInetSession->Close();

			char m_errorMsg[1024]; 
			pEx->GetErrorMessage( m_errorMsg, 1024 );
			OutputDebugString(m_errorMsg);
			ptr->Fcallback.OnFSFailedCallBackImpl();
		}	
	 }//if

	//删除对象;
	SyncList::InstensPtr()->RemoveByHandle(ptr->handle);
	delete ptr;

	return TRUE;
}
void   ThreadHelper::StopSendOrDownFileByHandle(HANDLE handle)
{
	ThreadHelper::ThreadParam* ptr = SyncList::InstensPtr()->RemoveByHandle(handle);
	if(ptr)
		ptr->Terminated();
	//线程结束自动释放对象;
}
/******************************************************************************
模块名称:  编码转换函数
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      FTP对象参数使用的是UTF8编码，这里需要转换下
*****************************************************************************/
int Encode::ConvUtf8ToAnsi(CString& strSource, CString& strChAnsi)
{  
	if (strSource.GetLength() <= 0)
		return 0;

	CString strWChUnicode;

	strSource.TrimLeft();
	strSource.TrimRight();   
	strChAnsi.Empty();

	int iLenByWChNeed = MultiByteToWideChar(CP_UTF8, 0,
		strSource.GetBuffer(0),
		strSource.GetLength(), //MultiByteToWideChar
		NULL, 0);

	int iLenByWchDone = MultiByteToWideChar(CP_UTF8, 0,
		strSource.GetBuffer(0),
		strSource.GetLength(),
		(LPWSTR)strWChUnicode.GetBuffer(iLenByWChNeed * 2),
		iLenByWChNeed); //MultiByteToWideChar

	strWChUnicode.ReleaseBuffer(iLenByWchDone * 2);

	int iLenByChNeed  = WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)strWChUnicode.GetBuffer(0),
		iLenByWchDone,
		NULL, 0,
		NULL, NULL); 

	int iLenByChDone  = WideCharToMultiByte(CP_ACP, 0,
		(LPCWSTR)strWChUnicode.GetBuffer(0),
		iLenByWchDone,
		strChAnsi.GetBuffer(iLenByChNeed),
		iLenByChNeed,
		NULL, NULL);

	strChAnsi.ReleaseBuffer(iLenByChDone);

	if (iLenByWChNeed != iLenByWchDone || iLenByChNeed != iLenByChDone)
		return 1;

	return 0;   
}

CString Encode::UnicodeToUTF8(const char* pSrc, int nLen)
{
	WCHAR* pWDes = NULL;
	int nWDLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pSrc, nLen, pWDes, 0);
	pWDes = new WCHAR[nWDLen+1];
	memset(pWDes, 0, nWDLen+1);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pSrc, nLen, pWDes, nWDLen);
	char* pUTF8 = NULL;
	int nUTF8 = WideCharToMultiByte(CP_UTF8, 0, pWDes, nWDLen, pUTF8, 0, 0, 0);
	pUTF8 = new char[nUTF8+1];
	memset(pUTF8, 0, nUTF8+1);
	WideCharToMultiByte(CP_UTF8, 0, pWDes, nWDLen, pUTF8, nUTF8, 0, 0);
	CString strResult = pUTF8;
	delete [] pWDes;
	delete [] pUTF8;
	return strResult;
}
/******************************************************************************
模块名称:  FTP连接模块
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:     
*****************************************************************************/
Ftp_Connecter::Ftp_Connecter()
{
	this->IP = "";
	this->Port = 0;
	this->userName = "";
	this->passWord = "";
	this->m_pFtpConnection = NULL;
}
void Ftp_Connecter::InitConnected(const char* IP,int Port,const char* userName,const char* passWord)
{
	this->IP = IP;
	this->Port = Port;
	this->userName = userName;
	this->passWord = passWord;			
}
void Ftp_Connecter::logout()
{
	  if(m_pFtpConnection)  
	  {  
		  m_pFtpConnection->Close();  
		  delete m_pFtpConnection;  
		  m_pFtpConnection = NULL;  
	  } 
	  m_pInetSession.Close();
}
bool Ftp_Connecter::login()
{
	try  
	{  	
		m_pFtpConnection = m_pInetSession.GetFtpConnection((LPCTSTR)this->IP.c_str(),this->userName.c_str(),this->passWord.c_str(),this->Port); 
		if(!m_pFtpConnection)  
			return false;  
	}  
	catch(CInternetException *pEx)  
	{  		
		char m_errorMsg[1024]; 
		pEx->GetErrorMessage( m_errorMsg, 1024 );
		OutputDebugString(m_errorMsg);
		m_pFtpConnection = NULL;
		return false;  
	}  
	catch (...)  
	{  
		if(m_pFtpConnection)
		{
			delete m_pFtpConnection;
			m_pFtpConnection = NULL;
		}
		return false;  
	}        
	return true;      
}

std::string GetFileNameByFilePath(const std::string filepath)
{ 
	int index = filepath.rfind('\\'); 
	return filepath.substr(index+1,filepath.size()-index);
}

//发送文件到服务器
int  Ftp_Connecter::sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return -1;

	bool bOpen = false;	
	CString RemoteFileName;
	CFile* m_CFile = NULL;
	CInternetFile* pRemoteFile = NULL;
	try
	{		
		m_CFile = new CFile();
		RemoteFileName.Format("%s\\%s",RemoteDirPath,GetFileNameByFilePath(LocalfileNamePath).c_str());
		OutputDebugString((LPCTSTR)RemoteFileName);		

		bOpen = m_CFile->Open(LocalfileNamePath, CFile::modeRead );		
		if(!bOpen)
		{
			OutputDebugString((LPCTSTR)("本地文件不存在:" + CString(LocalfileNamePath)));
			return -1;
		}

		CString utf8 = Encode::UnicodeToUTF8((LPCTSTR)RemoteFileName,strlen(RemoteFileName));

		//用一个CInternetSession创建多个下载会报错，这里使用独立的CInternetSession对象
		CInternetSession*	pInetSession = new CInternetSession(AfxGetAppName(),1,INTERNET_OPEN_TYPE_PRECONFIG );
		CFtpConnection*  pFtpConnection = pInetSession->GetFtpConnection((LPCTSTR)this->IP.c_str(),this->userName.c_str(),this->passWord.c_str(),this->Port); 
		if(!pFtpConnection)
		{
			OutputDebugString("远程文件打开失败");
			return -1;
		}
		pRemoteFile = pFtpConnection->OpenFile(utf8, GENERIC_WRITE);  
		if(!pRemoteFile)
		{
			OutputDebugString("远程文件打开失败");
			return -1;
		}	

		//放到线程里面进行上传
		return (int)ThreadHelper::InstensPtr()->SendFileThreadBegin(m_CFile,pRemoteFile,pInetSession,pFtpConnection,callback);		
	}  
	catch(CInternetException *pEx)  
	{  
		char m_errorMsg[1024]; 
		pEx->GetErrorMessage( m_errorMsg, 1024 );
		OutputDebugString(m_errorMsg);
		if(bOpen)  
			m_CFile->Close();
		
		if(m_CFile)
			delete m_CFile;
		if(pRemoteFile)
		{
			pRemoteFile->Close();
			delete pRemoteFile;
		}
		return -1;  
	}  
	catch (CException *pEx)  
	{  
		char m_errorMsg[1024]; 
		pEx->GetErrorMessage( m_errorMsg, 1024 );
		OutputDebugString(m_errorMsg);				

		if(bOpen)  
			m_CFile->Close();  

		if(m_CFile)
			delete m_CFile;
		if(pRemoteFile)
		{
			pRemoteFile->Close();
			delete pRemoteFile;
		}
		return -1;  
	}    
	return -1;  
}

//从服务器接收文件
int  Ftp_Connecter::recvFileFramServer(const char* RemotefileNamePath,const char* LocalDirPath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return -1;

	CString LocalFileName;
	CFile* wrireFile = NULL;
	CInternetFile* pRemoteFile = NULL;
	bool bOpen = false;
	try
	{	
		wrireFile = new CFile();
		LocalFileName.Format("%s\\%s",LocalDirPath,GetFileNameByFilePath(RemotefileNamePath).c_str());
		bOpen = wrireFile->Open(LocalFileName,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		if(!bOpen)
		{
			OutputDebugString((LPCTSTR)("本地文件创建失败:" + LocalFileName));
			return -1;
		}
		CString utf8 = Encode::UnicodeToUTF8(RemotefileNamePath,strlen(RemotefileNamePath));
		//注意CInternetFile对象的GetLength()是缓冲区长度,这里只有先通过CFtpFileFind获取文件大小;
		CFtpFileFind m_pRemoteFinder(m_pFtpConnection);
		ULONGLONG iTotalfileSize =  0;
		if(m_pRemoteFinder.FindFile(utf8))
		{
			m_pRemoteFinder.FindNextFile();
			iTotalfileSize = m_pRemoteFinder.GetLength();			
		}	
		m_pRemoteFinder.Close();

		//用一个CInternetSession创建多个下载会报错，这里使用独立的CInternetSession对象
		CInternetSession*	pInetSession = new CInternetSession(AfxGetAppName(),1,INTERNET_OPEN_TYPE_PRECONFIG );
		CFtpConnection*  pFtpConnection = pInetSession->GetFtpConnection((LPCTSTR)this->IP.c_str(),this->userName.c_str(),this->passWord.c_str(),this->Port); 
		if(!pFtpConnection)
		{
			OutputDebugString("远程文件打开失败");
			return -1;
		}
		pRemoteFile = pFtpConnection->OpenFile(utf8, GENERIC_READ);  
		if(!pRemoteFile)
		{
			OutputDebugString("远程文件打开失败");
			return -1;
		} 
		return (int)ThreadHelper::InstensPtr()->DownFileThreadBegin(wrireFile,pRemoteFile,pInetSession,pFtpConnection,callback,iTotalfileSize) ;
	}  
	catch(CInternetException *pEx)  
	{  
		char m_errorMsg[1024]; 
		pEx->GetErrorMessage( m_errorMsg, 1024 );
		OutputDebugString(m_errorMsg);
		if(bOpen)  
			wrireFile->Close();

		if(wrireFile)
			delete wrireFile;
		if(pRemoteFile)
		{
			pRemoteFile->Close();
			delete pRemoteFile;
		}
		return -1;  
	}  
	catch (...)  
	{            
		if(bOpen)  
			wrireFile->Close();

		if(wrireFile)
			delete wrireFile;
		if(pRemoteFile)
		{
			pRemoteFile->Close();
			delete pRemoteFile;
		}
		return -1;  
	} 
	return -1;
}

//从服务器接受文件列表
int  Ftp_Connecter::recvFileListFramServer(const char* RemoteDirPath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return -1;
	CallBackFunction	Fcallback;
	Fcallback.FWndcallback = callback;

	char buff[MAX_PATH];
	memset(buff,0,MAX_PATH);
	string dirlist = "";	
	try
	{
		CFtpFileFind* m_pRemoteFinder = new CFtpFileFind(m_pFtpConnection);
		CString UTF8RemoteDirPath = Encode::UnicodeToUTF8((LPCTSTR)RemoteDirPath,strlen(RemoteDirPath));
		BOOL res = m_pRemoteFinder->FindFile(UTF8RemoteDirPath);
		if(res)
		{
			dirlist = "<DList>";
			while(res)
			{	
				res = m_pRemoteFinder->FindNextFile();			
				if(m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//目录
				{				
					CString fileName;
					Encode::ConvUtf8ToAnsi(m_pRemoteFinder->GetFileName(),fileName);
					sprintf(buff,"<Ditm><dp>%s</dp><ct>%s</ct></Ditm>\n", fileName, "yyy-MM-dd hh:mm");
					dirlist += buff;
				}
				
			}
			dirlist += "</DList>";	
		}
		res = m_pRemoteFinder->FindFile(UTF8RemoteDirPath);
		if(res)
		{
			dirlist += "<FList>";
			char tmp[20];
			while(res)
			{	
				res = m_pRemoteFinder->FindNextFile();
				if(!m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//文件
				{
					ULONGLONG _totalsize = m_pRemoteFinder->GetLength();
					ULONGLONG _size = _totalsize / (long)1024.0 / (long)1024.0;
					if (_size > (long)1.0)
						sprintf(tmp,"%dMB", _size);
					else
						sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);

					CString fileName;
					Encode::ConvUtf8ToAnsi(m_pRemoteFinder->GetFileName(),fileName);
					sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", fileName,tmp,"yyy-MM-dd hh:mm");
					dirlist += buff;
				}				
			}
			dirlist += "</FList>";	
		}
		delete m_pRemoteFinder;
	}catch(...)
	{
		OutputDebugString("获取文件列表失败");
	}	

	if(dirlist != "")
	{
		dirlist = "<DFList>" + dirlist + "<DFList>";
		Fcallback.OnGetFileListByPathImpl(dirlist.c_str());
		return 0;
	}
	else
	{
		Fcallback.OnGetFileListByPathImpl("",false);
		return -1;
	}
}


time_t ConvToDate(const char* date)
{
	//char* date = "20121206141817";
	if(strlen(date) != 14)
		return time_t();

	try
	{
		char yyyy[10];
		char MM[10];
		char dd[10];
		char hh[10];
		char mm[10];
		char ss[10];
		memcpy(yyyy,date,4);
		yyyy[4] = '\0';
		memcpy(MM,&date[4],2);
		MM[2] = '\0';
		memcpy(dd,&date[6],2);
		dd[2] = '\0';
		memcpy(hh,&date[8],2);
		hh[2] = '\0';
		memcpy(mm,&date[10],2);
		mm[2] = '\0';
		memcpy(ss,&date[12],2);	
		ss[2] = '\0';

		struct tm t;
		t.tm_year = atol(yyyy)-1900;
		t.tm_mon = atol(MM)-1;
		t.tm_mday = atol(dd);	

		t.tm_hour = atol(hh);
		t.tm_min = atol(mm);
		t.tm_sec = atol(ss);

		//char strTime[80];
		//strftime(strTime, sizeof(strTime), "%Y/%m/%d %X",&t);
		//printf("%s\n",strTime);  
		return mktime(&t);
	}
	catch (const char* error)
	{
		OutputDebugString("[ConvToDate]catch Error:%s\n"); 
	}
	return time_t();	
}
//根据时间段到查询文件列表
int Ftp_Connecter::recvFileListByDateFramServer(const char* RemoteDirPath,const char* beginDate,const char* endDate,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return -1;
	CallBackFunction	Fcallback;
	Fcallback.FWndcallback = callback;

	time_t beginTime = ConvToDate(beginDate);
	time_t endTime = ConvToDate(endDate);

	if(!beginTime || !endTime)
		return -1;

	char buff[MAX_PATH];
	memset(buff,0,MAX_PATH);
	string dirlist = "";	
	try
	{
		CFtpFileFind* m_pRemoteFinder = new CFtpFileFind(m_pFtpConnection);
		CString UTF8RemoteDirPath = Encode::UnicodeToUTF8((LPCTSTR)RemoteDirPath,strlen(RemoteDirPath));
		BOOL res = m_pRemoteFinder->FindFile(UTF8RemoteDirPath);
		if(res)
		{
			dirlist = "<FList>";
			CString fileName = "",EngineerID = "",FileDate = "";
			int Pos = 0;
			char tmp[20];
			while(res)
			{	
				res = m_pRemoteFinder->FindNextFile();
				if(!m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//文件
				{
					fileName = m_pRemoteFinder->GetFileName();
					Pos = fileName.Find("-");
					if(Pos != 0)
						FileDate = fileName.Right(fileName.GetLength() - (Pos+1));

					//19-20121128111451  [工程师ID-日期]
					time_t fileTime = ConvToDate((LPCTSTR)FileDate);
					if(!fileTime)
						continue;					

					if(difftime(fileTime,beginTime) >= 0.0 && difftime(fileTime,endTime) <= 0)			
					{
						ULONGLONG _totalsize = m_pRemoteFinder->GetLength();
						ULONGLONG _size = _totalsize / (long)1024.0 / (long)1024.0;
						if (_size > (long)1.0)
							sprintf(tmp,"%dMB", _size);
						else
							sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);	

						CString fileName;
						Encode::ConvUtf8ToAnsi(m_pRemoteFinder->GetFileName(),fileName);
						sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", fileName,tmp,"yyy-MM-dd hh:mm");
						dirlist += buff;
					}
				}				
			}//while
			dirlist += "</FList>";	
		}
		delete m_pRemoteFinder;
	}catch(...)
	{
		OutputDebugString("获取文件列表失败");
	}	

	if(dirlist != "")
	{
		dirlist = "<DFList>" + dirlist + "<DFList>";
		Fcallback.OnGetFileListByDateAndPathImpl(dirlist.c_str());
		return 0;
	}
	else
	{
		Fcallback.OnGetFileListByDateAndPathImpl("",false);
		return -1;
	}
}
//删除目录
bool Ftp_Connecter::delDirFramServer(const char* RemoteDirPath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return false;
	CallBackFunction	Fcallback;
	Fcallback.FWndcallback = callback;
	try  
	{         		
		if(!m_pFtpConnection->RemoveDirectory(Encode::UnicodeToUTF8(RemoteDirPath,strlen(RemoteDirPath))))  
		{
			Fcallback.OnDelDirImpl(false);
			return false;                    
		}
	}  
	catch (...)  
	{  
		OutputDebugString("删除目录失败");
		Fcallback.OnDelDirImpl(true);
		return false;  
	}  
	Fcallback.OnDelDirImpl(true);
	return true;
}
//创建目录
bool Ftp_Connecter::createDirFramServer(const char* RemoteDirPath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return false;

	CallBackFunction	Fcallback;
	Fcallback.FWndcallback = callback;
	try  
	{           
		if(!m_pFtpConnection->CreateDirectory(Encode::UnicodeToUTF8(RemoteDirPath,strlen(RemoteDirPath))))  
		{
			Fcallback.OnCreateDirImpl(false);
			return false;                    
		}
	}  
	catch (...)  
	{  
		OutputDebugString("创建目录失败");
		Fcallback.OnCreateDirImpl(true);
		return false;  
	}  
	Fcallback.OnCreateDirImpl(true);
	return true;
}
//删除文件
bool Ftp_Connecter::delFileFramServer(const char* RemoteFilePath,FWndCallBackHandle callback)
{
	if(!m_pFtpConnection)
		return false;
	CallBackFunction	Fcallback;
	Fcallback.FWndcallback = callback;
	try  
	{           
		if(!m_pFtpConnection->Remove(Encode::UnicodeToUTF8(RemoteFilePath,strlen(RemoteFilePath))))  
		{
			Fcallback.OnDelFileImpl(false);
			return false;                    
		}
	}  
	catch (...)  
	{  
		OutputDebugString("删除文件失败");
		Fcallback.OnDelFileImpl(true);
		return false;  
	}  
	Fcallback.OnDelFileImpl(true);
	return true;  
}
//停止上传和下载
void Ftp_Connecter::stopFileSendOrRecvByHandle(int Handle)
{
	if(Handle == -1)
		return;
	try
	{
		//ThreadHelper::InstensPtr()->StopSendOrDownFileByHandle((HANDLE)Handle);
		ThreadHelper::ThreadParam* ptr = SyncList::InstensPtr()->RemoveByHandle((HANDLE)Handle);
		if(ptr)
			ptr->Terminated();
	}	
	catch (CException* e)
	{
	}
}
//获取当前连接数，这个只是用于保持连接的，在FTP中不用返回连接数，返回0保持连接即可,-1表示与FTP服务器断开
bool Ftp_Connecter::CheckedConnected()
{
	bool isConncted = false;
	if(m_pFtpConnection)
	{
		try
		{
			CString CurPath = "";
			if(m_pFtpConnection->GetCurrentDirectory(CurPath))
				isConncted = true;
		}
		catch(CInternetException *pEx)  
		{  
			char m_errorMsg[1024]; 
			pEx->GetErrorMessage( m_errorMsg, 1024 );
			OutputDebugString(m_errorMsg);

			isConncted = false;
			m_pFtpConnection = NULL;
		}
		catch(CException *pEx)
		{
			char m_errorMsg[1024]; 
			pEx->GetErrorMessage( m_errorMsg, 1024 );
			OutputDebugString(m_errorMsg);							

			isConncted = false;
			m_pFtpConnection = NULL;
		}
	}else
		isConncted = false;

	if(!isConncted)
		isConncted = this->login();
	
	return isConncted;
}

Ftp_Connecter* Ftp_Connecter::InstensPtr()
{
	static Ftp_Connecter* ptr = NULL;
	if(!ptr)
		ptr = new Ftp_Connecter();
	return ptr;
}