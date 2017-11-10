#include "stdafx.h"
#include "Ftp_Connecter.h"
#include <time.h>

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
//�������ڻ�ȡָ��·�����ļ��б�
void CallBackFunction::OnGetFileListByPathImpl(const char* text,bool isSucessed)
{
	//ACE_DEBUG ((LM_ERROR,L"3OnGetFileListByPathImpl(%s)\n", text)); 

	//charתwchar_t
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
//ɾ���ļ�
void CallBackFunction::OnDelFileImpl(bool isSucessed )
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FILEDEL,Fobject);
}
//ɾ��Ŀ¼
void CallBackFunction::OnDelDirImpl(bool isSucessed)
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_DIRDEL,Fobject);
}
//����Ŀ¼
void CallBackFunction::OnCreateDirImpl(bool isSucessed)
{
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_DIRCREATE,Fobject);
}
//�����ļ�ʱ�ķ�����ͬ���ļ�����
bool CallBackFunction::OnFSOverWriterCallBackImpl(char* text)
{
	Fobject->isSucessed = false;
	Fobject->CMDMsg = text;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_SEND_ISOVERWRITE,Fobject);
	return Fobject->isSucessed;
}
//�����ļ�ʱ�ķ�����ͬ���ļ�����
bool CallBackFunction::OnFROverWriterCallBackImpl(char* text)
{
	Fobject->isSucessed = false;
	Fobject->CMDMsg = text;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_ISOVERWRITE,Fobject);
	return Fobject->isSucessed;
}
/******************************************************************************
ģ������:  ��ǰ�����ػ��ϴ��߳��б�
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      ����һ���߳�ͬ���࣬����ָֹͣ���̵߳����غ��ϴ�
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
ģ������:  FTP�ϴ�������������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      �ϴ��������Ǻܺ�ʱ��,��Ҫ�ŵ��߳��봦��;
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
	{//����
		try
		{
			char pBuf[MAX_BUFFER_SIZE];  
			memset(pBuf, 0, MAX_BUFFER_SIZE); 			
			ULONGLONG iTotalfileSize =  ptr->iFtpfileSize;//GetLength()���������ȡ���ǻ���������;
			DWORD dwRead = 0,iTotalRecvSize = 0; 
			int iCurPrgress = 0,iLastPrgress = 0;
			char buff[40];
			while((dwRead = ptr->pRemoteFile->Read(pBuf, MAX_BUFFER_SIZE)) > 0 && !ptr->isTerminated())  
			{  
				ptr->pFile->Write(pBuf, dwRead );  
				iTotalRecvSize += dwRead;

				//���½���
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
				//�Զ�ɾ��
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
	{//�ϴ�
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

				//���½���
				iCurPrgress = iTotalfileSize >0 ? int((double)iTotalUpSize / (double)iTotalfileSize * (double)100.0):0;	
				if(iLastPrgress != iCurPrgress)
				{		
					iLastPrgress = iCurPrgress;
					ptr->Fcallback.OnFSProgressCallBackImpl(100,iLastPrgress);				
				}		   	
			}
			//����Ҫ�ȹرգ���ȼ������¼��ᴦ�����ļ�;
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

	//ɾ������;
	SyncList::InstensPtr()->RemoveByHandle(ptr->handle);
	delete ptr;

	return TRUE;
}
void   ThreadHelper::StopSendOrDownFileByHandle(HANDLE handle)
{
	ThreadHelper::ThreadParam* ptr = SyncList::InstensPtr()->RemoveByHandle(handle);
	if(ptr)
		ptr->Terminated();
	//�߳̽����Զ��ͷŶ���;
}
/******************************************************************************
ģ������:  ����ת������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      FTP�������ʹ�õ���UTF8���룬������Ҫת����
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
ģ������:  FTP����ģ��
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:     
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

//�����ļ���������
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
			OutputDebugString((LPCTSTR)("�����ļ�������:" + CString(LocalfileNamePath)));
			return -1;
		}

		CString utf8 = Encode::UnicodeToUTF8((LPCTSTR)RemoteFileName,strlen(RemoteFileName));

		//��һ��CInternetSession����������ػᱨ������ʹ�ö�����CInternetSession����
		CInternetSession*	pInetSession = new CInternetSession(AfxGetAppName(),1,INTERNET_OPEN_TYPE_PRECONFIG );
		CFtpConnection*  pFtpConnection = pInetSession->GetFtpConnection((LPCTSTR)this->IP.c_str(),this->userName.c_str(),this->passWord.c_str(),this->Port); 
		if(!pFtpConnection)
		{
			OutputDebugString("Զ���ļ���ʧ��");
			return -1;
		}
		pRemoteFile = pFtpConnection->OpenFile(utf8, GENERIC_WRITE);  
		if(!pRemoteFile)
		{
			OutputDebugString("Զ���ļ���ʧ��");
			return -1;
		}	

		//�ŵ��߳���������ϴ�
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

//�ӷ����������ļ�
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
			OutputDebugString((LPCTSTR)("�����ļ�����ʧ��:" + LocalFileName));
			return -1;
		}
		CString utf8 = Encode::UnicodeToUTF8(RemotefileNamePath,strlen(RemotefileNamePath));
		//ע��CInternetFile�����GetLength()�ǻ���������,����ֻ����ͨ��CFtpFileFind��ȡ�ļ���С;
		CFtpFileFind m_pRemoteFinder(m_pFtpConnection);
		ULONGLONG iTotalfileSize =  0;
		if(m_pRemoteFinder.FindFile(utf8))
		{
			m_pRemoteFinder.FindNextFile();
			iTotalfileSize = m_pRemoteFinder.GetLength();			
		}	
		m_pRemoteFinder.Close();

		//��һ��CInternetSession����������ػᱨ������ʹ�ö�����CInternetSession����
		CInternetSession*	pInetSession = new CInternetSession(AfxGetAppName(),1,INTERNET_OPEN_TYPE_PRECONFIG );
		CFtpConnection*  pFtpConnection = pInetSession->GetFtpConnection((LPCTSTR)this->IP.c_str(),this->userName.c_str(),this->passWord.c_str(),this->Port); 
		if(!pFtpConnection)
		{
			OutputDebugString("Զ���ļ���ʧ��");
			return -1;
		}
		pRemoteFile = pFtpConnection->OpenFile(utf8, GENERIC_READ);  
		if(!pRemoteFile)
		{
			OutputDebugString("Զ���ļ���ʧ��");
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

//�ӷ����������ļ��б�
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
				if(m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//Ŀ¼
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
				if(!m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//�ļ�
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
		OutputDebugString("��ȡ�ļ��б�ʧ��");
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
//����ʱ��ε���ѯ�ļ��б�
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
				if(!m_pRemoteFinder->IsDirectory() && !m_pRemoteFinder->IsDots())//�ļ�
				{
					fileName = m_pRemoteFinder->GetFileName();
					Pos = fileName.Find("-");
					if(Pos != 0)
						FileDate = fileName.Right(fileName.GetLength() - (Pos+1));

					//19-20121128111451  [����ʦID-����]
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
		OutputDebugString("��ȡ�ļ��б�ʧ��");
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
//ɾ��Ŀ¼
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
		OutputDebugString("ɾ��Ŀ¼ʧ��");
		Fcallback.OnDelDirImpl(true);
		return false;  
	}  
	Fcallback.OnDelDirImpl(true);
	return true;
}
//����Ŀ¼
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
		OutputDebugString("����Ŀ¼ʧ��");
		Fcallback.OnCreateDirImpl(true);
		return false;  
	}  
	Fcallback.OnCreateDirImpl(true);
	return true;
}
//ɾ���ļ�
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
		OutputDebugString("ɾ���ļ�ʧ��");
		Fcallback.OnDelFileImpl(true);
		return false;  
	}  
	Fcallback.OnDelFileImpl(true);
	return true;  
}
//ֹͣ�ϴ�������
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
//��ȡ��ǰ�����������ֻ�����ڱ������ӵģ���FTP�в��÷���������������0�������Ӽ���,-1��ʾ��FTP�������Ͽ�
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