#include "EventHandler.h"
#include "ClientMothod.h"


/******************************************************************************
ģ������:  CallBackFunction:
ģ�鹦��:  �¼��ص��¼�
������:    ��
��������:  2012/10
��������:
��ע:      ���ﴥ�����¼�����µ�Clientע����¼���

��Ϊc++��֧���ຯ���ص�����û����Bass_Event_Handler����������ʵ�ֻص�������client
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
void CallBackFunction::OnGetFileListByPathImpl(char* text,bool isSucessed)
{
	//ACE_DEBUG ((LM_ERROR,L"3OnGetFileListByPathImpl(%s)\n", text)); 

	//charתwchar_t
	//size_t len = strlen(text) + 1;
	//size_t converted = 0;
	//Fobject->CMDMsg=(wchar_t*)malloc(len*sizeof(wchar_t));
	//mbstowcs_s(&converted, Fobject->CMDMsg, len, text, _TRUNCATE);

	Fobject->CMDMsg = text;	
	Fobject->isSucessed = isSucessed;
	if(FWndcallback)
		FWndcallback(CALLBACK_TYPE_RECV_FILELIST,Fobject);
}
void CallBackFunction::OnGetFileListByDateAndPathImpl(char* text,bool isSucessed)
{
	Fobject->CMDMsg = text;	
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
ģ������:  Bass_Event_Handler:
ģ�鹦��:  �¼��������
������:    ��
��������:  2012/10
��������:
��ע:      ����sock��һ�㴦�����̼��ذ�����
******************************************************************************/
//��Reactorע��READ_TASK�¼�����ʼ���peer_sock�ϵ���������
int Bass_Event_Handler::Open(ACE_SOCK_Stream& sock,FWndCallBackHandle	FWndcallback)
{	
	this->peer_sock_ = sock;
	this->FCallBackFunction.InitWndCallBack(FWndcallback,(int)this->get_handle());

	int ret = this->reactor()->register_handler(this, ACE_Event_Handler::READ_MASK);
	this->peer().get_remote_addr(client_Addr_);
	this->isRegWRITE_MASK = true;

	Client_List::Instens()->AddEvent(this->get_handle(),this);
	return ret;
}
//���ͻ��˷�������
 int Bass_Event_Handler::handle_output(ACE_HANDLE handle)
{
	ACE_Message_Block *mb;	

	//�Ӷ���ͷ��ȡ��Ϣ�鷢�ͣ�ѭ��ֱ������Ϊ��
	while(this->output_queue.dequeue_head(mb, &timeout) == 0)
	{
		int sendSize = this->peer().send_n(mb->rd_ptr(), mb->length());
		if(-1 == sendSize)
		{
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[handle_output] Can not Send feedback Info to:%s\n"),client_Addr_.get_host_addr()));
			break;
		}
		else 
			mb->rd_ptr(sendSize);

		//��Ϣ��û�����꣬�������ͷ����������
		if(mb->length() > 0)
			this->output_queue.enqueue_head(mb);

		mb->release();
	}

	//������Ϊ�գ���ע��WRITE_TASK
	if(this->output_queue.is_empty())
	{
		isRegWRITE_MASK = false;
		this->reactor()->remove_handler(this, ACE_Event_Handler::WRITE_MASK);
		return -1;
	}
	else
		return 0;
}
//ע���¼�����
 int Bass_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	if (mask == ACE_Event_Handler::WRITE_MASK)
		return 0;
	
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("Disconnected:%s\n"),client_Addr_.get_host_addr()));
	Bass_Event_Handler* ptr = Client_List::Instens()->RemoveEventByHandle(this->get_handle());
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("Current connected count(%d)\n"),Client_List::Instens()->GetCount()));

	mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	this->reactor()->remove_handler(this, mask);
	this->peer().close();
	this->output_queue.flush();

	//������Ϣѭ��
	reactor()->end_reactor_event_loop();
	reactor()->close();
	return 0;
}
 //���ͻ��˷��ͷ�����Ϣ(�ذ�)
 int Bass_Event_Handler::send_back_info(void* pSendFrame,int sizeof_FrameType)
 {
	 int sendSize = this->peer().send_n(pSendFrame, sizeof_FrameType);

	 if(sizeof_FrameType == sendSize)	//����ȫ����ȷ����
	 {
		 return 0;
	 }
	 else if(-1 == sendSize)	//�������ݳ���
	 {
		 ACE_DEBUG((LM_DEBUG,ACE_TEXT("[send_back]Can not Send feedback Info(%s)\n"),client_Addr_.get_host_addr()));
		 return -1;
	 }
	 else if(sendSize > 0 && sendSize < sizeof_FrameType) //���ݲ��ַ���
	 {
		 ACE_Message_Block *mb;	//���ʣ�������
		 ACE_NEW_RETURN(mb, ACE_Message_Block((char*)(pSendFrame) + sendSize, sizeof_FrameType - sendSize), -1);
		 if(this->output_queue.enqueue_tail(mb, &timeout) == -1)
		 {
			 mb->release();
			 return -1;
		 }

		 //һ��û�������ע��WRITE_TASK
		 if(!isRegWRITE_MASK)
		 {
			 isRegWRITE_MASK = true;
			 return this->reactor()->register_handler(this, ACE_Event_Handler::WRITE_MASK);
		 }
	 }
	 return 0;
 }
//��ǰsock����
ACE_SOCK_Stream& Bass_Event_Handler::peer() 
{
	return this->peer_sock_; 
}
ACE_INET_Addr& Bass_Event_Handler::get_addr()
{
	return this->client_Addr_;
}
//���
 ACE_HANDLE Bass_Event_Handler::get_handle (void) const
{ 
	return peer_sock_.get_handle(); 
}
/******************************************************************************
ģ������:  File_Recv_Event_Handler
ģ�鹦��:  �ļ����մ�����
������:    ��
��������:  2012/10
��������:
��ע:      �����ļ����ղ���
******************************************************************************/
 File_Recv_Event_Handler::File_Recv_Event_Handler(ACE_Reactor *r):Bass_Event_Handler(r)
{
	this->RemotefileSize = 0;
	this->iLastPrgress = 0;
	this->iTotalRecvSize = 0;
}

//���տͻ��˵�����
 int File_Recv_Event_Handler::handle_input(ACE_HANDLE handle)
{			
	//�������ݴ���
	int retSize = this->peer().recv_n(&(recvFrame), sizeof(FileSendFrame));
	if(0 == retSize)
	{
		return -1;//�����˳�
	}
	else if(retSize == -1 || retSize != sizeof(FileSendFrame))
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]recv data error[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	//����ȫ�Լ��
	if(this->recvFrame.length > MAX_BUFFER_SIZE)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]recv error,recvFrame size too large[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	//��������֡���ͣ���ȡ��Ӧ�Ĳ�����׼���÷��͵�����֡
	this->sendFrame.length = 0;
	this->sendFrame.type = FILE_RECV_INVALID;
	switch(recvFrame.type)
	{
	case FILE_SEND_NAME:		//�ļ���					
		this->recvFileNameMsg();
		break;
	case FILE_SEND_SIZE:		//�ļ���С
		this->recvFileSizeMsg();
		break;			
	case FILE_SEND_DATA:		//�ļ�����
		this->recvFileDataMsg();	
		return 0;
	case FILE_SEND_END:			//�ļ��������
		this->recvFileSendMsg();
		break;
	case FILE_SEND_ERROR:		//���Ͷ˳���
		this->recvFileSendErrorMsg();
		break;
	case FILE_SEND_OVERWRITE:	//���Ͷ�ȷ�ϸ����ļ�
		this->recvOverWriteSameFileMsg();
		break;;
	case FILE_SEND_FILE_NOTEXIST:		//client��ȡָ���ļ�ʱ�������������ļ���������Ͽ�(����ʱֱ�ӷ����ļ���С����ʼ������������)
		this->sendFrame.type = FILE_RECV_ERROR;
		this->sendFrame.info[this->sendFrame.length] = '\0';
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]Server file does not exist[file name:%s]\n"),this->sendFrame.info));
		break;
	case FILE_SEND_DISCONNECT:	//�Ͽ�����
	default:
		this->sendFrame.type = FILE_RECV_INVALID;
		break;
	}

	//������߿ͻ��˶Ͽ���Ͽ�����
	if(FILE_SEND_ERROR == recvFrame.type 
		|| FILE_SEND_DISCONNECT == recvFrame.type || FILE_RECV_ERROR == this->sendFrame.type)
	{
		return -1;
	}	

	//���ͻذ�
	if(sendFrame.type != FILE_RECV_INVALID)
		this->send_back_info((void*)&sendFrame,sizeof(FileRecvFrame));		
	return 0;
}

 
int File_Recv_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	const char* fileName = this->localFile.getPathName();
	this->localFile.close();
	//ɾ����ʱ�ļ�(û������ʱ)
	if(iTotalRecvSize != RemotefileSize)
	{
		if(LocalFile::fileExist(fileName))
			LocalFile::deleteFile(fileName);
	}

	//֪ͨ���淢�ͳɹ���ʧ��
	if(iTotalRecvSize == RemotefileSize  && RemotefileSize > 0)
		FCallBackFunction.OnFRSucessedCallBackImpl();
	else
		FCallBackFunction.OnFRFailedCallBackImpl();
	
	return this->Bass_Event_Handler::handle_close(ACE_INVALID_HANDLE,mask);
}

//��ʾ���ս�����־
void File_Recv_Event_Handler::showProgress(int recvSize)
{
	iTotalRecvSize += recvSize;
	int iCurPrgress = RemotefileSize >0 ? int((double)iTotalRecvSize / (double)this->RemotefileSize * (double)100.0):0;

	if(iLastPrgress != iCurPrgress)
	{		
		iLastPrgress = iCurPrgress;

		//֪ͨ�����
		FCallBackFunction.OnFRProgressCallBackImpl(100,this->iLastPrgress);

		if(iCurPrgress % 10 == 0)
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]resv data [%d%%][%s][IP:%s:%d]\n"),iLastPrgress,this->localFile.getFileName(),client_Addr_.get_host_addr(),client_Addr_.get_port_number()));
	}	
}

//�յ��ļ���
//����ֻ��server�Ż�ִ��;client��ȡ�ļ�ʱ���Ƿ����ļ�������Ϣ�����������������������ֱ���·��ļ���С,����������������(ֻ����һ���Ĳ���)
//·��Ϊ���������·��:
void File_Recv_Event_Handler::recvFileNameMsg()
{
	this->recvFrame.buffer[this->recvFrame.length] = '\0';
	
	//���ж��Ƿ���·����·���Ƿ����(���������򷵻ش���,���ﲻ����·��)
	char* serverFilePath = DirHelper::GetServerLocalDirPath(this->recvFrame.buffer);
    if(LocalFile::fileExist(serverFilePath))
	{
		this->sendFrame.type = FILE_RECV_FILE_EXIST;	
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]The file name already exists[%s][IP:%s]\n"),this->recvFrame.buffer,client_Addr_.get_host_addr()));
	}
	else if(this->localFile.open(serverFilePath))
	{
		this->sendFrame.type = FILE_RECV_GET_FILESIZE;
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]Begin receiving the file[%s][IP:%s]\n"),this->recvFrame.buffer,client_Addr_.get_host_addr()));
	}
	else
	{
		this->sendFrame.type = FILE_RECV_ERROR;
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]recv file Error[FileName:%s][IP:%s]\n"),this->recvFrame.buffer,client_Addr_.get_host_addr()));
	}

	delete[] serverFilePath;
}
//�յ��ļ���С
void File_Recv_Event_Handler::recvFileSizeMsg()
{
	this->recvFrame.buffer[this->recvFrame.length+1] = '\0';
	this->RemotefileSize = ACE_OS::atoi(this->recvFrame.buffer);
	this->sendFrame.type = FILE_RECV_GET_FILEDATA;

	long ret = this->RemotefileSize / (long)1024.0 / (long)1024.0;
	char buf[20];
	if (ret > (long)1.0)
		ACE_OS::sprintf(buf,"%dMB", ret);
	else
		ACE_OS::sprintf(buf,"%dKB", this->RemotefileSize / (long)1024 == 0 ? 1 : this->RemotefileSize / (long)1024);

	ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]receiving filesize[%s][fileSize:%s][IP:%s]\n"),this->localFile.getFileName(),buf,client_Addr_.get_host_addr()));
}
//�յ��ļ�����
void File_Recv_Event_Handler::recvFileDataMsg()
{
	if(this->localFile.write(this->recvFrame.buffer, this->recvFrame.length) < this->recvFrame.length)
		this->sendFrame.type = FILE_RECV_ERROR;
	else
		this->sendFrame.type = FILE_RECV_GET_FILEDATA;

	//��ʾ����
	this->showProgress(this->recvFrame.length);
}

//�յ��ļ����ͽ�����Ϣ
void File_Recv_Event_Handler::recvFileSendMsg()
{
	//дʣ������
	if(this->recvFrame.length > 0)
	{
		if(this->localFile.write(this->recvFrame.buffer, this->recvFrame.length) < this->recvFrame.length)
			this->sendFrame.type = FILE_RECV_ERROR;
	}
	this->localFile.close();
	this->sendFrame.type = FILE_RECV_FINISHED;

	//��ʾ����
	this->showProgress(this->recvFrame.length);
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]File receive completed[%s][IP:%s]\n"),this->localFile.getFileName(),client_Addr_.get_host_addr()));
}
//�յ��ļ����Ͷ˳�����Ϣ
void File_Recv_Event_Handler::recvFileSendErrorMsg()
{
	//����Ѿ������ļ�����رղ�ɾ���ļ�(�ڶϿ�ʱ���д����ˣ�����Ͳ���Ҫ������)	
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]Client error , receive files failed[%s][IP:%s]\n"),this->localFile.getFileName(),client_Addr_.get_host_addr()));
}
//�յ����Ͷ˶�ͬ���ļ��ĸ�����Ϣ(ֻ�з������Ż��յ�����Ϣ)
void File_Recv_Event_Handler::recvOverWriteSameFileMsg()
{
	this->recvFrame.buffer[this->recvFrame.length] = '\0';

	//���ж��Ƿ���·����·���Ƿ����(���������򷵻ش���,���ﲻ����·��)
	char* serverFilePath = DirHelper::GetServerLocalDirPath(this->recvFrame.buffer);

	//��ɾ��֮ǰ���ļ�
	if(LocalFile::deleteFile(serverFilePath))
	{
		//���ļ���׼��д����
		if(this->localFile.open(serverFilePath))
			this->sendFrame.type = FILE_RECV_GET_FILESIZE;
		else
			this->sendFrame.type = FILE_RECV_ERROR;
	}
	else
	{
		this->sendFrame.type = FILE_RECV_ERROR;
	}
}
//�ӷ���������ָ���ļ�(client����)
//�������յ�FILE_RECV_CLIENT_GET_FILENAME��Ϣ����������������������ļ�����ֱ�ӷ����ļ���С,������������
int File_Recv_Event_Handler::recvFileFromServer(const char* RemotefileNamePath,
												 const char* LocalDirPath)
{
	char* _RemotefilePath = const_cast<char*>(RemotefileNamePath);
	char* fileName = DirHelper::GetFileNameByPath(_RemotefilePath);
	ACE_OS::sprintf(this->sendFrame.info,"%s\\%s",LocalDirPath,fileName);
	delete[] fileName;

	if(this->localFile.open(this->sendFrame.info))
	{
		this->sendFrame.type = FILE_RECV_CLIENT_GET_FILENAME;
		ACE_OS::strcpy(this->sendFrame.info,RemotefileNamePath);
		this->sendFrame.length = ACE_OS::strlen(this->sendFrame.info);

		this->send_back_info((void*)&this->sendFrame,sizeof(FileRecvFrame));
		return (int)this->get_handle();
	}
	return -1;
}
/******************************************************************************
ģ������:  File_Send_Event_Handler
ģ�鹦��:  �ļ����ʹ�����
������:    ��
��������:  2012/10
��������:
��ע:      �����ļ����͵�����¼�
*******************************************************************************/
File_Send_Event_Handler::File_Send_Event_Handler(ACE_Reactor *r):Bass_Event_Handler(r)
{
	iLocalFileSize = 0;
	iSendPrgress = 0;
	iTotalSendSize = 0;
}
//�����ļ����ն˵�����
 int File_Send_Event_Handler::handle_input(ACE_HANDLE handle)
{
	int retSize = this->peer().recv_n(&(this->recvFrame), sizeof(FileRecvFrame));
	if(0 == retSize)
	{
		return -1;//�����˳�
	}
	else if(retSize == -1 || retSize != sizeof(FileRecvFrame))
	{
		ACE_DEBUG((LM_ERROR,L"[fileSend]recv data error[IP:%s]\n",client_Addr_.get_host_addr()));
		return -1;
	}

	//����ȫ�Լ��
	if(this->recvFrame.length > MAX_INFO_SIZE)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]recv error,recvFrame size too large[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	this->sendFrame.type = FILE_SEND_INVALID;
	this->sendFrame.length = 0;

	switch(this->recvFrame.type)
	{
	case FILE_RECV_CLIENT_GET_FILENAME://��ȡ�ļ���(�����Ϣ��client���͹����ģ�server�ȼ����ն˱����ļ��Ƿ����,�ٷ��ͻ�ִ)
		this->checkedLocalFileIsExistAndSendResult();
		break;
	case FILE_RECV_GET_FILENAME:	//��ȡ�ļ���
		this->sendFileName();
		break;
	case FILE_RECV_GET_FILESIZE:	//��ȡ�ļ���С
		this->sendFileSize();
		break;
	case FILE_RECV_GET_FILEDATA:	//��������֤��ɼ��������ļ����ݣ�ֱ���յ���ɻ򱨴�Ϊֹ;
		{
			while(true)
			{
				this->sendFileData();			
				if(this->send_back_info((void*)&(sendFrame), sizeof(FileSendFrame)) == -1)
					return -1;
				if(this->sendFrame.type == FILE_SEND_END)
					return 0;
			}		
			break;
		}
	case FILE_RECV_FINISHED:		//�ļ��������
		ACE_DEBUG((LM_ERROR,L"[fileSend]Send a file successfully(%s)\n",this->localFile.getFileName())); 			
		break;
	case FILE_RECV_FILE_EXIST:		//ͬ���ļ�������ֱ�Ӹ���
		ACE_DEBUG((LM_ERROR,L"[fileSend]Direct overwrite a file of the same name(%s)\n",this->localFile.getFileName())); 								
		this->sendIsOverWriteSameFile();
		break;
	case FILE_RECV_DIR_NOTEXIST:
		ACE_DEBUG((LM_ERROR,L"[fileSend]Recv error,Server filedir not exist!\n"));
		this->sendFrame.type  = FILE_SEND_DISCONNECT;
		break;
	case FILE_RECV_ERROR:			//���ն˱���;
		ACE_DEBUG((LM_ERROR,L"[fileSend]Receiving end of an error\n")); 
		break;
	default:
		this->sendFrame.type = FILE_SEND_INVALID;
		break;
	}

	//����/�ͻ��˶Ͽ����ļ����������Ͽ�����
	if(FILE_SEND_DISCONNECT == sendFrame.type 
		|| FILE_RECV_FINISHED == recvFrame.type 
		|| FILE_RECV_ERROR == recvFrame.type)
	{		
		return -1;
	}	

	//���ͻذ�(���Ϳͻ�������֡�������ļ��Ӷ˷���)
	//if(FILE_SEND_INVALID != sendFrame.type)
	this->send_back_info((void*)&(sendFrame), sizeof(FileSendFrame));	
	return 0;
}

 int File_Send_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
 {
		 //֪ͨ���淢�ͳɹ���ʧ��
		 if(iLocalFileSize == iTotalSendSize && iLocalFileSize > 0)
			FCallBackFunction.OnFSSucessedCallBackImpl();
		 else
			FCallBackFunction.OnFSFailedCallBackImpl();
			
		 this->localFile.close();
		 return this->Bass_Event_Handler::handle_close(ACE_INVALID_HANDLE,mask);
 }

//��ʾ���ͽ���
void File_Send_Event_Handler::showSendProgress(int readSize)
{
	//��ʾ������־;
	this->iTotalSendSize += readSize;
	int iCurPrgress = this->iLocalFileSize >0 ? int((double)iTotalSendSize / (double)this->iLocalFileSize * (double)100.0):0;

	if(iSendPrgress != iCurPrgress)
	{		
		iSendPrgress = iCurPrgress;

		//֪ͨ����
		FCallBackFunction.OnFSProgressCallBackImpl(100,iCurPrgress);

		if(iCurPrgress % 10 == 0)
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]Send data [%d%%][%s][IP:%s:%d]\n"),iSendPrgress,this->localFile.getFileName(),client_Addr_.get_host_addr(),client_Addr_.get_port_number()));
	}
}
//(��Ϣ���ͺ���)
//�����ļ���
void File_Send_Event_Handler::sendFileName()
{
	this->sendFrame.type = FILE_SEND_NAME;
	ACE_OS::strcpy(this->sendFrame.buffer, this->localFile.getFileName());
	this->sendFrame.length = ACE_OS::strlen(this->localFile.getFileName());

	ACE_DEBUG((LM_DEBUG,L"[fileSend]Starts to send the file(%s)\n", this->localFile.getFileName())); 
}
//�����ļ���С
void File_Send_Event_Handler::sendFileSize()
{
	this->sendFrame.type = FILE_SEND_SIZE;
	this->iLocalFileSize = this->localFile.getFileSize();
	ACE_OS::sprintf(this->sendFrame.buffer,"%d",iLocalFileSize);
	this->sendFrame.length = ACE_OS::strlen(this->sendFrame.buffer);
}
//�����ļ�����
void File_Send_Event_Handler::sendFileData()
{
	bool	sendFlag = false;
	int		readSize = this->localFile.read(this->sendFrame.buffer, MAX_BUFFER_SIZE);
	long	iLocalFileSize =  this->localFile.getFileSize();

	if(readSize < 0)
	{
		this->sendFrame.type = FILE_SEND_END;
	}
	else
	{
		sendFlag = true;
		this->sendFrame.length = readSize;
		if(MAX_BUFFER_SIZE == readSize)
			this->sendFrame.type = FILE_SEND_DATA;			

		if(readSize < MAX_BUFFER_SIZE)
			this->sendFrame.type = FILE_SEND_END;	

		//��ʾ������־;
		this->showSendProgress(readSize);
	}

	//�����ļ���С��MAX_BUFFER_SIZE�������������
	if(sendFlag && 0 == readSize && this->sendFrame.type != FILE_SEND_END)
		this->sendFrame.type = FILE_SEND_END;	
}

//�Ƿ񸲸�ͬ���ļ�
void File_Send_Event_Handler::sendIsOverWriteSameFile()
{	
	ACE_OS::strcpy(this->sendFrame.buffer, this->localFile.getFileName());
	this->sendFrame.length = ACE_OS::strlen(this->localFile.getFileName());

	char buf[MAX_CMD_SIZE];
	ACE_OS::sprintf(buf,"�������Ѵ����ļ�\"%s\"��ȷ���Ƿ񸲸Ǹ��ļ�?",this->localFile.getFileName());
	bool isOverWrite = this->FCallBackFunction.OnFSOverWriterCallBackImpl(buf);

	this->sendFrame.type = (isOverWrite ? FILE_SEND_OVERWRITE : FILE_SEND_DISCONNECT);	
}	
//�������������ļ��Ƿ���ڲ����ͻ�ִ(client��ȡָ���ļ�ʱ)
void File_Send_Event_Handler::checkedLocalFileIsExistAndSendResult()
{
	this->recvFrame.info[this->recvFrame.length] = '\0';
	char* serverFileName = DirHelper::GetServerLocalDirPath(this->recvFrame.info);

	if(LocalFile::fileExist(serverFileName))
	{
		if(this->localFile.open(serverFileName))
		{
			this->sendFileSize();		
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]The file name already exists[%s][IP:%s]\n"),this->localFile.getFileName(),client_Addr_.get_host_addr()));
		}
		else
			this->sendFrame.type = FILE_SEND_ERROR;
	}
	else
	{
		this->sendFrame.type = FILE_SEND_FILE_NOTEXIST;
		ACE_OS::strcpy(this->sendFrame.buffer,this->recvFrame.info);
		this->sendFrame.length = ACE_OS::strlen(this->recvFrame.info);

		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]file is Not Exist:[%s]\n"),this->recvFrame.info));
	}
}
//���ͱ����ļ���������(client����)
int File_Send_Event_Handler::sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath)
{
	if(LocalFile::fileExist(LocalfileNamePath))
	{
		if(this->localFile.open(LocalfileNamePath))
		{			
			char* _localfilePath = const_cast<char*>(LocalfileNamePath);
			char* fileName = DirHelper::GetFileNameByPath(_localfilePath);

			if(ACE_OS::strlen(RemoteDirPath) + ACE_OS::strlen(fileName) > MAX_CMD_SIZE)
			{
				ACE_DEBUG((LM_ERROR,L"[fileSend]send failed,filePath is too long(%s\\%s)\n", RemoteDirPath,fileName)); 
				delete[] fileName;

				return -1;
			}						
		
			this->sendFrame.type = FILE_SEND_NAME;
			ACE_OS::sprintf(this->sendFrame.buffer,"%s\\%s",RemoteDirPath,fileName);			
			this->sendFrame.length = ACE_OS::strlen(this->sendFrame.buffer);
			this->iLocalFileSize = this->localFile.getFileSize();

			ACE_DEBUG((LM_DEBUG,L"[fileSend]Starts to send the file(%s),remotePath(%s)\n", this->localFile.getFileName(),RemoteDirPath)); 
			delete[] fileName;
			//���͵�������
			this->send_back_info((void*)&this->sendFrame,sizeof(FileSendFrame));
			return (int)this->get_handle();
		}
		else
			ACE_DEBUG((LM_ERROR,L"[fileSend]Unable to open file(%s)\n", LocalfileNamePath)); 
	}
	else
		ACE_DEBUG((LM_ERROR,L"[fileSend]The file does not exist(%s)\n", LocalfileNamePath)); 

	return -1;
}

/******************************************************************************
ģ������:  CMDMsg_Recv_Send_Event_Handler
ģ�鹦��:  ��Ϣ�ı��ķ��ͺͽ�����
������:    ��
��������:  2012/10
��������:
��ע:      ������Ϣ�ı����͵�Clientʱͨ���������д���(ClientҲͨ�����������ı�)
*****************************************************************************/
CMDMsg_Recv_Send_Event_Handler::CMDMsg_Recv_Send_Event_Handler(CilentCMDFrameType type,ACE_Reactor *r,const char* sendCMDMsg):Bass_Event_Handler(r)
{
	this->CMDFrameType = type;
	this->RecvCMDMsg = NULL;
	this->SendCMDMsg = NULL;

	this->iCMDMsgRecvLenth = 0;
	this->iCMDMsgSendLenth = 0;
	
	if(sendCMDMsg != NULL)
	{
		int len = ACE_OS::strlen(sendCMDMsg);
		this->SendCMDMsg = new char[len + 1];
		ACE_OS::strcpy(this->SendCMDMsg,sendCMDMsg);
		this->iCMDMsgTotalLenth = len;
	}
	else
	{
		this->SendCMDMsg = NULL;
		this->iCMDMsgTotalLenth = 0;
	}
}

int CMDMsg_Recv_Send_Event_Handler::handle_input(ACE_HANDLE handle)
{
	int recvSize = peer_sock_.recv_n(&recvCMDMsgFrame, sizeof(CMDMsgFrame)) ;
	if(0 == recvSize)
	{
		return -1;//�����˳�
	}
	else if(-1 == recvSize || recvSize != sizeof(CMDMsgFrame))
	{
		ACE_DEBUG((LM_ERROR,L"[CMD]Receive CMDMsg error!(%s)\n",client_Addr_.get_host_addr())); 
		return -1;
	}
	//���ͻ�ִ��Client
	this->sendCMDMsgFrame.type = MSG_INVALID;
	this->sendCMDMsgFrame.length = 0;
	try
	{
		switch(recvCMDMsgFrame.type)
		{
		case RECV_MSG_LENTH:				//�����ı�����
			{
				if(this->recvCMDMsgFrame.length > 0)
				{
					this->iCMDMsgTotalLenth = this->recvCMDMsgFrame.length;

					this->RecvCMDMsg = new char[this->iCMDMsgTotalLenth+1];
					ACE_OS::memset(this->RecvCMDMsg,0,this->iCMDMsgRecvLenth);					

					this->sendCMDMsgFrame.type = SEND_MSG_DATA;
					ACE_DEBUG((LM_DEBUG,L"[MSGRecv]recv lenth(totalLenth:%d)\n",recvCMDMsgFrame.length)); 
				}else
					return -1;				//ֱ�ӶϿ�
			}
			break;
		case RECV_MSG_DATA:					//�����ı�����
			{
				if(this->recvCMDMsgFrame.length > 0)
				{
					ACE_OS::memcpy(RecvCMDMsg + iCMDMsgRecvLenth,recvCMDMsgFrame.CMDMsg,recvCMDMsgFrame.length);
					this->iCMDMsgRecvLenth += recvCMDMsgFrame.length;
				}
				this->sendCMDMsgFrame.type = SEND_MSG_DATA;

				ACE_DEBUG((LM_DEBUG,L"[MSGRecv]recv data(recvSize:%d)\n",recvCMDMsgFrame.length)); 
				return 0;
			}			
		case RECV_MSG_END:					//������ɺ�ֱ�ӶϿ�����
			{
				if(this->recvCMDMsgFrame.length > 0)
				{
					ACE_OS::memcpy(RecvCMDMsg + iCMDMsgRecvLenth,recvCMDMsgFrame.CMDMsg,recvCMDMsgFrame.length);
					this->iCMDMsgRecvLenth += recvCMDMsgFrame.length;
				}
				this->RecvCMDMsg[this->iCMDMsgTotalLenth] = '\0';

				ACE_DEBUG((LM_DEBUG,L"[MSGRecv]recv end\n")); 
			}
			break;
		case SEND_MSG_LENTH:				//�����ı�����
			{
				this->sendCMDMsgFrame.length = iCMDMsgTotalLenth;				
				this->sendCMDMsgFrame.type = RECV_MSG_LENTH;

				ACE_DEBUG((LM_DEBUG,L"[MSGSend](lenth:%d)\n",iCMDMsgTotalLenth)); 
			}		
			break;
		case SEND_MSG_DATA:					//�����ı�����
			{
				while (true)
				{
					int iSendSize = (iCMDMsgTotalLenth - iCMDMsgSendLenth) > MAX_CMD_MSG_SIZE ? MAX_CMD_MSG_SIZE:(iCMDMsgTotalLenth - iCMDMsgSendLenth);
					ACE_OS::memcpy(this->sendCMDMsgFrame.CMDMsg,SendCMDMsg + iCMDMsgSendLenth,iSendSize);
					iCMDMsgSendLenth += iSendSize;

					this->sendCMDMsgFrame.length = iSendSize;
					ACE_DEBUG((LM_DEBUG,L"[MSGSend](iSendSize:%d)\n",iSendSize)); 

					if(iCMDMsgSendLenth < iCMDMsgTotalLenth)
					{
						this->sendCMDMsgFrame.type = RECV_MSG_DATA;
					}
					else
					{
						this->sendCMDMsgFrame.type = RECV_MSG_END;
						ACE_DEBUG((LM_DEBUG,L"[MSGSend]send end\n")); 
					}	

					if(this->send_back_info((void*)&(this->sendCMDMsgFrame), sizeof(CMDMsgFrame)) == -1)
						return -1;

					if (this->sendCMDMsgFrame.type == RECV_MSG_END)
						return 0;
				}
			}
			break;
		default:
			break;
		}		
	}
	catch(...)
	{
		ACE_DEBUG((LM_ERROR,L"[CMD]Receive CMDMsg catch error!(%s)\n",client_Addr_.get_host_addr()));
		return -1;
	}		

	if(RECV_MSG_END == this->recvCMDMsgFrame.type)
		return -1;

	//���ͻ�ִ
	this->send_back_info((void*)&(this->sendCMDMsgFrame), sizeof(CMDMsgFrame));
	return 0;
}


int CMDMsg_Recv_Send_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	switch(this->CMDFrameType)
	{
	case CLIENT_FILE_GET_LIST:		//��ȡ�ļ��б�
		if(this->iCMDMsgRecvLenth == this->iCMDMsgTotalLenth)
			FCallBackFunction.OnGetFileListByPathImpl(this->RecvCMDMsg);//֪ͨ����
		else
			FCallBackFunction.OnGetFileListByPathImpl("",false);//֪ͨ����
		break;
	case CLIENT_FILE_GET_LIST_BYDATE:
		if(this->iCMDMsgRecvLenth == this->iCMDMsgTotalLenth)
			FCallBackFunction.OnGetFileListByDateAndPathImpl(this->RecvCMDMsg);//֪ͨ����
		else
			FCallBackFunction.OnGetFileListByDateAndPathImpl("",false);//֪ͨ����
		break;
	default:
		break;
	}

	if(this->RecvCMDMsg != NULL)
		delete[] this->RecvCMDMsg;
	if(this->SendCMDMsg != NULL)
		delete[] this->SendCMDMsg;
	
	return this->Bass_Event_Handler::handle_close(ACE_INVALID_HANDLE,mask);
}
//������Ϣ�ı����ͻ���(client ����)
void CMDMsg_Recv_Send_Event_Handler::recvCMDMsgFramServer()
{
	this->sendCMDMsgFrame.type = SEND_MSG_LENTH;
	this->sendCMDMsgFrame.length = 0;
	//���͵�������
	this->send_back_info((void*)&this->sendCMDMsgFrame,sizeof(CMDMsgFrame));
}
/******************************************************************************
ģ������:  Client_List
ģ�鹦��:  �����б�
������:    ��
��������:  2012/10
��������:
��ע:      
/******************************************************************************/
Client_List* Client_List::Instens()
{
	static Client_List* ptr = NULL;
	if(!ptr)
		ptr = new Client_List();
	return ptr;
}

Bass_Event_Handler* Client_List::GetEventByHandle(ACE_HANDLE Handle)
{
	Bass_Event_Handler* ptr = NULL;
	if(Fmap.find(Handle,ptr) == 0)
		return ptr;
	else
		return NULL;
}

Bass_Event_Handler* Client_List::RemoveEventByHandle(ACE_HANDLE Handle)
{
	Bass_Event_Handler* ptr = NULL;
	if(Fmap.find(Handle,ptr) == 0)
	{
		Fmap.unbind(Handle,ptr);
		return ptr;
	}else
		return NULL;
}

void	Client_List::AddEvent(ACE_HANDLE Handle,Bass_Event_Handler* ptr)
{
	Fmap.bind(Handle,ptr);
}

int Client_List::GetCount()
{
	return Fmap.current_size();
}

void Client_List::Clear()
{
	this->Fmap.unbind_all();
}
/******************************************************************************
ģ������:  DirHelper
ģ�鹦��:  Ŀ¼������
������:    ��
��������:  2012/10
��������:
��ע:      GetFileNameByPath���ú���Ҫʹ��delete[]ɾ������
/******************************************************************************/
char* DirHelper::FtpServerMainDir = new char[MAX_CMD_SIZE];

bool DirHelper::InitMainDir(const char* pzMainDir)
{
	char* buff = new char[MAX_CMD_SIZE];
	ACE_OS::memset(DirHelper::FtpServerMainDir,0,MAX_CMD_SIZE);
	ACE_OS::memset(buff,0,MAX_CMD_SIZE);

	ACE_OS::getcwd(DirHelper::FtpServerMainDir,MAX_CMD_SIZE);	//��ȡ��ǰ����Ŀ¼
	ACE_OS::sprintf(buff,"%s\\%s",FtpServerMainDir,pzMainDir);

	bool ret = true;
	if(!CheckedDirPathIsExist(buff))
	{	
		ACE_OS::strcpy(buff,pzMainDir);
		ret = DirHelper::CreateServerDirPath(buff);		
	}
	ACE_OS::strcat(DirHelper::FtpServerMainDir,pzMainDir);
	delete[] buff;
	return ret;
}
//(strtok_r���޸Ĳ�����ֵ!!!!!!!,ʹ��new����ռ�����ͷ����⣬����ʹ��ջ�ռ�������)	
char* DirHelper::GetServerLocalDirPath(const char* pzPath)
{
	if(NULL == pzPath)
		return NULL;

	char* retDirPath = new char[MAX_CMD_SIZE];
	ACE_OS::memset(retDirPath,0,MAX_CMD_SIZE);
	if(ACE_OS::strlen(pzPath) + ACE_OS::strlen(FtpServerMainDir) < MAX_CMD_SIZE)	
	{
		if(ACE_OS::strlen(pzPath) == 0)
			ACE_OS::strcpy(retDirPath,FtpServerMainDir);
		else
			ACE_OS::sprintf(retDirPath,"%s\\%s",FtpServerMainDir,pzPath);
	}
	else
		ACE_DEBUG((LM_ERROR,L"[ERROR]Path is too long,GetServerLocalDir execution failed(%s\\%s)\n",FtpServerMainDir,pzPath));

	//·����ȷ�Լ�飬�ж��\\\\ʱֱ���滻��һ��
	char* keyStr = ACE_OS::strstr(retDirPath,"\\\\");    
	while(keyStr)
	{        
		int iTotalLan =	ACE_OS::strlen(retDirPath);
		int iFindSize = ACE_OS::strlen(keyStr);
		int pos = iTotalLan - iFindSize;

		char* tmp = new char[iTotalLan];
		ACE_OS::memset(tmp,0,iTotalLan);   
		ACE_OS::memcpy(tmp,retDirPath,pos); 
		ACE_OS::memcpy(tmp+pos,keyStr+1,iFindSize); 
		ACE_OS::strcpy(retDirPath,tmp); 
		delete[] tmp;

		keyStr = strstr(retDirPath,"\\\\"); 
	}
	DirHelper::SetLinuxPath(retDirPath);

	return retDirPath;
}
//����Linux·��(��//�����滻��\����)
void  DirHelper::SetLinuxPath(char* outPath)
{
	if(!outPath)
		return;

#ifndef ACE_WIN32
	int iTotalLan =	strlen(outPath);
	for(int i = 0;i<iTotalLan;i++)
	{
		if(outPath[i] == '\\')
			outPath[i] = '/';
	}
#endif
}
//(strtok_r���޸Ĳ�����ֵ!!!!!!!,ʹ��new����ռ�����ͷ����⣬����ʹ��ջ�ռ�������)	
bool DirHelper::CreateServerDirPath(const char* pzPath)
{	
	if(NULL == pzPath)
		return false;
	char __pzPath[MAX_CMD_SIZE];
	ACE_OS::strcpy(__pzPath,pzPath);

	char* __serverMainDir = DirHelper::GetServerLocalDirPath("");
	char* __localDir = DirHelper::GetServerLocalDirPath(__pzPath);	

	ACE_DIR* _dir = ACE_OS::opendir(__localDir);
	if(_dir != NULL)
	{
		ACE_OS::closedir(_dir);
		delete[] __localDir;
		delete[] __serverMainDir;
		return true;
	}

#ifdef ACE_WIN32
	char* spliter = "\\";
#else
	char* spliter = "/";
	SetLinuxPath(__pzPath);
#endif

	char* outer_ptr = NULL;	
	char* dirName = NULL;
	char* token = ACE_OS::strtok_r(__pzPath,spliter,&outer_ptr);
	while(token != NULL)		
	{
		dirName = token;
		ACE_OS::strcat(__serverMainDir,spliter);
		ACE_OS::strcat(__serverMainDir,dirName);
		_dir = ACE_OS::opendir(__serverMainDir);
		if(NULL == _dir)
		{				
			if(-1 == ACE_OS::mkdir(__serverMainDir))
			{
				delete[] __localDir;
				delete[] __serverMainDir;
				return false;
			}				
		}
		else
			ACE_OS::closedir(_dir);

		token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
	}
	delete[] __localDir;	
	delete[] __serverMainDir;
	return true;
}
bool DirHelper::CheckedDirPathIsExist(const char* pzPath)
{
	if(NULL == pzPath)
		return false;

	ACE_DIR* _dir = ACE_OS::opendir(pzPath);
	if(_dir != NULL)
	{
		ACE_OS::closedir(_dir);
		return true;
	}else
		return false;
}

bool  DirHelper::CheckedServerDirPathIsExist(const char* pzPath)
{
	if(NULL == pzPath)
		return false;

	if(ACE_OS::strlen(pzPath) + ACE_OS::strlen(FtpServerMainDir) < MAX_CMD_SIZE)		
	{
		ACE_DEBUG((LM_ERROR,L"[ERROR]Path is too long,CheckedServerDirPathIsExist execution  failed(%s\\%s)\n",FtpServerMainDir,pzPath));
		return false;
	}

	char* localPath = DirHelper::GetServerLocalDirPath(pzPath);

	ACE_DIR* _dir = ACE_OS::opendir(localPath);
	if(_dir != NULL)
	{
		ACE_OS::closedir(_dir);
		return true;
	}else
		return false;
}


//(strtok_r���޸Ĳ�����ֵ!!!!!!!,ʹ��new����ռ�����ͷ����⣬����ʹ��ջ�ռ�������)	
char*  DirHelper::GetFileNameByPath(const char* pzPath)
{
	if(NULL == pzPath)
		return NULL;
	char __pzPath[MAX_CMD_SIZE];
	ACE_OS::strcpy(__pzPath,pzPath);

#ifdef ACE_WIN32
	char* spliter = "\\";
#else
	char* spliter = "/";
	SetLinuxPath(__pzPath);
#endif

	char* outer_ptr = NULL;
	char* token = ACE_OS::strtok_r(__pzPath,spliter,&outer_ptr);
	char* fileName = new char[MAX_CMD_SIZE];
	ACE_OS::memset(fileName,0,MAX_CMD_SIZE);
	
	while(token != NULL)		
	{
		ACE_OS::strcpy(fileName,token);	//ȡ�����һ�������ļ���;
		token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
	}
	return fileName;
}
//(strtok_r���޸Ĳ�����ֵ!!!!!!!,ʹ��new����ռ�����ͷ����⣬����ʹ��ջ�ռ�������)	
char*  DirHelper::GetFilePathByName(const char*  pzFullFileName)
{
	if(NULL == pzFullFileName)
		return NULL;
	char __pzFullFileName[MAX_CMD_SIZE];
	ACE_OS::strcpy(__pzFullFileName,pzFullFileName);

#ifdef ACE_WIN32
	char* spliter = "\\";
#else
	char* spliter = "/";
	SetLinuxPath(__pzFullFileName);
#endif

	char* outer_ptr = NULL;
	char* token = ACE_OS::strtok_r(__pzFullFileName,spliter,&outer_ptr);
	char* DirName = NULL;
	char* retDirPath = new char[MAX_CMD_SIZE];
	ACE_OS::memset(retDirPath,0,MAX_CMD_SIZE);

	while(token != NULL)		
	{
		DirName = token;	
		token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);

		if(token != NULL)//�������һ�������Ķ���·��
			ACE_OS::strcat(retDirPath,DirName);
	}
	return retDirPath;
}

