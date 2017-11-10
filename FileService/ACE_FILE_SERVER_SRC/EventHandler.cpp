#include "EventHandler.h"
#include "ClientMothod.h"


/******************************************************************************
模块名称:  CallBackFunction:
模块功能:  事件回调事件
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      这里触发的事件会更新到Client注册的事件中

因为c++不支持类函数回调，就没有在Bass_Event_Handler的派生类中实现回调函数，client
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
void CallBackFunction::OnGetFileListByPathImpl(char* text,bool isSucessed)
{
	//ACE_DEBUG ((LM_ERROR,L"3OnGetFileListByPathImpl(%s)\n", text)); 

	//char转wchar_t
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
模块名称:  Bass_Event_Handler:
模块功能:  事件处理基类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      抽象sock的一般处理流程及回包处理
******************************************************************************/
//向Reactor注册READ_TASK事件，开始监测peer_sock上的数据输入
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
//往客户端发送数据
 int Bass_Event_Handler::handle_output(ACE_HANDLE handle)
{
	ACE_Message_Block *mb;	

	//从队列头部取消息块发送，循环直到队列为空
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

		//消息块没发送完，放入队列头部继续发送
		if(mb->length() > 0)
			this->output_queue.enqueue_head(mb);

		mb->release();
	}

	//若队列为空，则注销WRITE_TASK
	if(this->output_queue.is_empty())
	{
		isRegWRITE_MASK = false;
		this->reactor()->remove_handler(this, ACE_Event_Handler::WRITE_MASK);
		return -1;
	}
	else
		return 0;
}
//注销事件处理
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

	//结束消息循环
	reactor()->end_reactor_event_loop();
	reactor()->close();
	return 0;
}
 //往客户端发送反馈信息(回包)
 int Bass_Event_Handler::send_back_info(void* pSendFrame,int sizeof_FrameType)
 {
	 int sendSize = this->peer().send_n(pSendFrame, sizeof_FrameType);

	 if(sizeof_FrameType == sendSize)	//数据全部正确发送
	 {
		 return 0;
	 }
	 else if(-1 == sendSize)	//发送数据出错
	 {
		 ACE_DEBUG((LM_DEBUG,ACE_TEXT("[send_back]Can not Send feedback Info(%s)\n"),client_Addr_.get_host_addr()));
		 return -1;
	 }
	 else if(sendSize > 0 && sendSize < sizeof_FrameType) //数据部分发送
	 {
		 ACE_Message_Block *mb;	//存放剩余的数据
		 ACE_NEW_RETURN(mb, ACE_Message_Block((char*)(pSendFrame) + sendSize, sizeof_FrameType - sendSize), -1);
		 if(this->output_queue.enqueue_tail(mb, &timeout) == -1)
		 {
			 mb->release();
			 return -1;
		 }

		 //一次没发送完就注册WRITE_TASK
		 if(!isRegWRITE_MASK)
		 {
			 isRegWRITE_MASK = true;
			 return this->reactor()->register_handler(this, ACE_Event_Handler::WRITE_MASK);
		 }
	 }
	 return 0;
 }
//当前sock对象
ACE_SOCK_Stream& Bass_Event_Handler::peer() 
{
	return this->peer_sock_; 
}
ACE_INET_Addr& Bass_Event_Handler::get_addr()
{
	return this->client_Addr_;
}
//句柄
 ACE_HANDLE Bass_Event_Handler::get_handle (void) const
{ 
	return peer_sock_.get_handle(); 
}
/******************************************************************************
模块名称:  File_Recv_Event_Handler
模块功能:  文件接收处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理文件接收操作
******************************************************************************/
 File_Recv_Event_Handler::File_Recv_Event_Handler(ACE_Reactor *r):Bass_Event_Handler(r)
{
	this->RemotefileSize = 0;
	this->iLastPrgress = 0;
	this->iTotalRecvSize = 0;
}

//接收客户端的数据
 int File_Recv_Event_Handler::handle_input(ACE_HANDLE handle)
{			
	//接收数据错误
	int retSize = this->peer().recv_n(&(recvFrame), sizeof(FileSendFrame));
	if(0 == retSize)
	{
		return -1;//正常退出
	}
	else if(retSize == -1 || retSize != sizeof(FileSendFrame))
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]recv data error[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	//包安全性检查
	if(this->recvFrame.length > MAX_BUFFER_SIZE)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]recv error,recvFrame size too large[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	//根据数据帧类型，采取相应的操作并准备好发送的数据帧
	this->sendFrame.length = 0;
	this->sendFrame.type = FILE_RECV_INVALID;
	switch(recvFrame.type)
	{
	case FILE_SEND_NAME:		//文件名					
		this->recvFileNameMsg();
		break;
	case FILE_SEND_SIZE:		//文件大小
		this->recvFileSizeMsg();
		break;			
	case FILE_SEND_DATA:		//文件数据
		this->recvFileDataMsg();	
		return 0;
	case FILE_SEND_END:			//文件传输结束
		this->recvFileSendMsg();
		break;
	case FILE_SEND_ERROR:		//发送端出错
		this->recvFileSendErrorMsg();
		break;
	case FILE_SEND_OVERWRITE:	//发送端确认覆盖文件
		this->recvOverWriteSameFileMsg();
		break;;
	case FILE_SEND_FILE_NOTEXIST:		//client获取指定文件时，服务器返回文件不存在则断开(正常时直接返回文件大小，开始正常下载流程)
		this->sendFrame.type = FILE_RECV_ERROR;
		this->sendFrame.info[this->sendFrame.length] = '\0';
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]Server file does not exist[file name:%s]\n"),this->sendFrame.info));
		break;
	case FILE_SEND_DISCONNECT:	//断开连接
	default:
		this->sendFrame.type = FILE_RECV_INVALID;
		break;
	}

	//出错或者客户端断开则断开连接
	if(FILE_SEND_ERROR == recvFrame.type 
		|| FILE_SEND_DISCONNECT == recvFrame.type || FILE_RECV_ERROR == this->sendFrame.type)
	{
		return -1;
	}	

	//发送回包
	if(sendFrame.type != FILE_RECV_INVALID)
		this->send_back_info((void*)&sendFrame,sizeof(FileRecvFrame));		
	return 0;
}

 
int File_Recv_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	const char* fileName = this->localFile.getPathName();
	this->localFile.close();
	//删除临时文件(没接收完时)
	if(iTotalRecvSize != RemotefileSize)
	{
		if(LocalFile::fileExist(fileName))
			LocalFile::deleteFile(fileName);
	}

	//通知界面发送成功或失败
	if(iTotalRecvSize == RemotefileSize  && RemotefileSize > 0)
		FCallBackFunction.OnFRSucessedCallBackImpl();
	else
		FCallBackFunction.OnFRFailedCallBackImpl();
	
	return this->Bass_Event_Handler::handle_close(ACE_INVALID_HANDLE,mask);
}

//显示接收进度日志
void File_Recv_Event_Handler::showProgress(int recvSize)
{
	iTotalRecvSize += recvSize;
	int iCurPrgress = RemotefileSize >0 ? int((double)iTotalRecvSize / (double)this->RemotefileSize * (double)100.0):0;

	if(iLastPrgress != iCurPrgress)
	{		
		iLastPrgress = iCurPrgress;

		//通知界面层
		FCallBackFunction.OnFRProgressCallBackImpl(100,this->iLastPrgress);

		if(iCurPrgress % 10 == 0)
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]resv data [%d%%][%s][IP:%s:%d]\n"),iLastPrgress,this->localFile.getFileName(),client_Addr_.get_host_addr(),client_Addr_.get_port_number()));
	}	
}

//收到文件名
//这里只有server才会执行;client获取文件时，是发送文件名称消息到服务器，服务器存在则就直接下发文件大小,接下来正常流程走(只有这一步的差异)
//路径为服务器相对路径:
void File_Recv_Event_Handler::recvFileNameMsg()
{
	this->recvFrame.buffer[this->recvFrame.length] = '\0';
	
	//先判断是否有路径和路径是否存在(不存在则则返回错误,这里不创建路径)
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
//收到文件大小
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
//收到文件数据
void File_Recv_Event_Handler::recvFileDataMsg()
{
	if(this->localFile.write(this->recvFrame.buffer, this->recvFrame.length) < this->recvFrame.length)
		this->sendFrame.type = FILE_RECV_ERROR;
	else
		this->sendFrame.type = FILE_RECV_GET_FILEDATA;

	//显示进度
	this->showProgress(this->recvFrame.length);
}

//收到文件发送结束消息
void File_Recv_Event_Handler::recvFileSendMsg()
{
	//写剩余数据
	if(this->recvFrame.length > 0)
	{
		if(this->localFile.write(this->recvFrame.buffer, this->recvFrame.length) < this->recvFrame.length)
			this->sendFrame.type = FILE_RECV_ERROR;
	}
	this->localFile.close();
	this->sendFrame.type = FILE_RECV_FINISHED;

	//显示进度
	this->showProgress(this->recvFrame.length);
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]File receive completed[%s][IP:%s]\n"),this->localFile.getFileName(),client_Addr_.get_host_addr()));
}
//收到文件发送端出错消息
void File_Recv_Event_Handler::recvFileSendErrorMsg()
{
	//如果已经接收文件，则关闭并删除文件(在断开时进行处理了，这里就不需要处理了)	
	ACE_DEBUG((LM_DEBUG,ACE_TEXT("[FileRecv]Client error , receive files failed[%s][IP:%s]\n"),this->localFile.getFileName(),client_Addr_.get_host_addr()));
}
//收到发送端对同名文件的覆盖消息(只有服务器才会收到该消息)
void File_Recv_Event_Handler::recvOverWriteSameFileMsg()
{
	this->recvFrame.buffer[this->recvFrame.length] = '\0';

	//先判断是否有路径和路径是否存在(不存在则则返回错误,这里不创建路径)
	char* serverFilePath = DirHelper::GetServerLocalDirPath(this->recvFrame.buffer);

	//先删除之前的文件
	if(LocalFile::deleteFile(serverFilePath))
	{
		//打开文件并准备写数据
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
//从服务器接收指定文件(client调用)
//服务器收到FILE_RECV_CLIENT_GET_FILENAME消息处理后，如果服务器存在这个文件，则直接发下文件大小,按正常流程走
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
模块名称:  File_Send_Event_Handler
模块功能:  文件发送处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理文件发送的相关事件
*******************************************************************************/
File_Send_Event_Handler::File_Send_Event_Handler(ACE_Reactor *r):Bass_Event_Handler(r)
{
	iLocalFileSize = 0;
	iSendPrgress = 0;
	iTotalSendSize = 0;
}
//接收文件接收端的数据
 int File_Send_Event_Handler::handle_input(ACE_HANDLE handle)
{
	int retSize = this->peer().recv_n(&(this->recvFrame), sizeof(FileRecvFrame));
	if(0 == retSize)
	{
		return -1;//正常退出
	}
	else if(retSize == -1 || retSize != sizeof(FileRecvFrame))
	{
		ACE_DEBUG((LM_ERROR,L"[fileSend]recv data error[IP:%s]\n",client_Addr_.get_host_addr()));
		return -1;
	}

	//包安全性检查
	if(this->recvFrame.length > MAX_INFO_SIZE)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]recv error,recvFrame size too large[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	this->sendFrame.type = FILE_SEND_INVALID;
	this->sendFrame.length = 0;

	switch(this->recvFrame.type)
	{
	case FILE_RECV_CLIENT_GET_FILENAME://获取文件名(这个消息是client发送过来的，server先检查接收端本地文件是否存在,再发送回执)
		this->checkedLocalFileIsExistAndSendResult();
		break;
	case FILE_RECV_GET_FILENAME:	//获取文件名
		this->sendFileName();
		break;
	case FILE_RECV_GET_FILESIZE:	//获取文件大小
		this->sendFileSize();
		break;
	case FILE_RECV_GET_FILEDATA:	//服务器验证完成继续发送文件内容，直到收到完成或报错为止;
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
	case FILE_RECV_FINISHED:		//文件发送完成
		ACE_DEBUG((LM_ERROR,L"[fileSend]Send a file successfully(%s)\n",this->localFile.getFileName())); 			
		break;
	case FILE_RECV_FILE_EXIST:		//同名文件服务器直接覆盖
		ACE_DEBUG((LM_ERROR,L"[fileSend]Direct overwrite a file of the same name(%s)\n",this->localFile.getFileName())); 								
		this->sendIsOverWriteSameFile();
		break;
	case FILE_RECV_DIR_NOTEXIST:
		ACE_DEBUG((LM_ERROR,L"[fileSend]Recv error,Server filedir not exist!\n"));
		this->sendFrame.type  = FILE_SEND_DISCONNECT;
		break;
	case FILE_RECV_ERROR:			//接收端报错;
		ACE_DEBUG((LM_ERROR,L"[fileSend]Receiving end of an error\n")); 
		break;
	default:
		this->sendFrame.type = FILE_SEND_INVALID;
		break;
	}

	//出错/客户端断开或文件发送完成则断开连接
	if(FILE_SEND_DISCONNECT == sendFrame.type 
		|| FILE_RECV_FINISHED == recvFrame.type 
		|| FILE_RECV_ERROR == recvFrame.type)
	{		
		return -1;
	}	

	//发送回包(发送客户端数据帧并接收文件接端反馈)
	//if(FILE_SEND_INVALID != sendFrame.type)
	this->send_back_info((void*)&(sendFrame), sizeof(FileSendFrame));	
	return 0;
}

 int File_Send_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
 {
		 //通知界面发送成功或失败
		 if(iLocalFileSize == iTotalSendSize && iLocalFileSize > 0)
			FCallBackFunction.OnFSSucessedCallBackImpl();
		 else
			FCallBackFunction.OnFSFailedCallBackImpl();
			
		 this->localFile.close();
		 return this->Bass_Event_Handler::handle_close(ACE_INVALID_HANDLE,mask);
 }

//显示发送进度
void File_Send_Event_Handler::showSendProgress(int readSize)
{
	//显示进度日志;
	this->iTotalSendSize += readSize;
	int iCurPrgress = this->iLocalFileSize >0 ? int((double)iTotalSendSize / (double)this->iLocalFileSize * (double)100.0):0;

	if(iSendPrgress != iCurPrgress)
	{		
		iSendPrgress = iCurPrgress;

		//通知界面
		FCallBackFunction.OnFSProgressCallBackImpl(100,iCurPrgress);

		if(iCurPrgress % 10 == 0)
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[fileSend]Send data [%d%%][%s][IP:%s:%d]\n"),iSendPrgress,this->localFile.getFileName(),client_Addr_.get_host_addr(),client_Addr_.get_port_number()));
	}
}
//(消息发送函数)
//发送文件名
void File_Send_Event_Handler::sendFileName()
{
	this->sendFrame.type = FILE_SEND_NAME;
	ACE_OS::strcpy(this->sendFrame.buffer, this->localFile.getFileName());
	this->sendFrame.length = ACE_OS::strlen(this->localFile.getFileName());

	ACE_DEBUG((LM_DEBUG,L"[fileSend]Starts to send the file(%s)\n", this->localFile.getFileName())); 
}
//发送文件大小
void File_Send_Event_Handler::sendFileSize()
{
	this->sendFrame.type = FILE_SEND_SIZE;
	this->iLocalFileSize = this->localFile.getFileSize();
	ACE_OS::sprintf(this->sendFrame.buffer,"%d",iLocalFileSize);
	this->sendFrame.length = ACE_OS::strlen(this->sendFrame.buffer);
}
//发送文件数据
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

		//显示进度日志;
		this->showSendProgress(readSize);
	}

	//处理文件大小是MAX_BUFFER_SIZE的整数倍的情况
	if(sendFlag && 0 == readSize && this->sendFrame.type != FILE_SEND_END)
		this->sendFrame.type = FILE_SEND_END;	
}

//是否覆盖同名文件
void File_Send_Event_Handler::sendIsOverWriteSameFile()
{	
	ACE_OS::strcpy(this->sendFrame.buffer, this->localFile.getFileName());
	this->sendFrame.length = ACE_OS::strlen(this->localFile.getFileName());

	char buf[MAX_CMD_SIZE];
	ACE_OS::sprintf(buf,"服务器已存在文件\"%s\"请确认是否覆盖该文件?",this->localFile.getFileName());
	bool isOverWrite = this->FCallBackFunction.OnFSOverWriterCallBackImpl(buf);

	this->sendFrame.type = (isOverWrite ? FILE_SEND_OVERWRITE : FILE_SEND_DISCONNECT);	
}	
//检查服务器本地文件是否存在并发送回执(client获取指定文件时)
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
//发送本地文件到服务器(client调用)
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
			//发送到服务器
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
模块名称:  CMDMsg_Recv_Send_Event_Handler
模块功能:  消息文本的发送和接收类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      当有消息文本发送到Client时通过这个类进行处理(Client也通过这个类接收文本)
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
		return -1;//正常退出
	}
	else if(-1 == recvSize || recvSize != sizeof(CMDMsgFrame))
	{
		ACE_DEBUG((LM_ERROR,L"[CMD]Receive CMDMsg error!(%s)\n",client_Addr_.get_host_addr())); 
		return -1;
	}
	//发送回执给Client
	this->sendCMDMsgFrame.type = MSG_INVALID;
	this->sendCMDMsgFrame.length = 0;
	try
	{
		switch(recvCMDMsgFrame.type)
		{
		case RECV_MSG_LENTH:				//接收文本长度
			{
				if(this->recvCMDMsgFrame.length > 0)
				{
					this->iCMDMsgTotalLenth = this->recvCMDMsgFrame.length;

					this->RecvCMDMsg = new char[this->iCMDMsgTotalLenth+1];
					ACE_OS::memset(this->RecvCMDMsg,0,this->iCMDMsgRecvLenth);					

					this->sendCMDMsgFrame.type = SEND_MSG_DATA;
					ACE_DEBUG((LM_DEBUG,L"[MSGRecv]recv lenth(totalLenth:%d)\n",recvCMDMsgFrame.length)); 
				}else
					return -1;				//直接断开
			}
			break;
		case RECV_MSG_DATA:					//接收文本内容
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
		case RECV_MSG_END:					//接收完成后直接断开连接
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
		case SEND_MSG_LENTH:				//发送文本长度
			{
				this->sendCMDMsgFrame.length = iCMDMsgTotalLenth;				
				this->sendCMDMsgFrame.type = RECV_MSG_LENTH;

				ACE_DEBUG((LM_DEBUG,L"[MSGSend](lenth:%d)\n",iCMDMsgTotalLenth)); 
			}		
			break;
		case SEND_MSG_DATA:					//发送文本内容
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

	//发送回执
	this->send_back_info((void*)&(this->sendCMDMsgFrame), sizeof(CMDMsgFrame));
	return 0;
}


int CMDMsg_Recv_Send_Event_Handler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	switch(this->CMDFrameType)
	{
	case CLIENT_FILE_GET_LIST:		//获取文件列表
		if(this->iCMDMsgRecvLenth == this->iCMDMsgTotalLenth)
			FCallBackFunction.OnGetFileListByPathImpl(this->RecvCMDMsg);//通知界面
		else
			FCallBackFunction.OnGetFileListByPathImpl("",false);//通知界面
		break;
	case CLIENT_FILE_GET_LIST_BYDATE:
		if(this->iCMDMsgRecvLenth == this->iCMDMsgTotalLenth)
			FCallBackFunction.OnGetFileListByDateAndPathImpl(this->RecvCMDMsg);//通知界面
		else
			FCallBackFunction.OnGetFileListByDateAndPathImpl("",false);//通知界面
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
//接收消息文本到客户端(client 调用)
void CMDMsg_Recv_Send_Event_Handler::recvCMDMsgFramServer()
{
	this->sendCMDMsgFrame.type = SEND_MSG_LENTH;
	this->sendCMDMsgFrame.length = 0;
	//发送到服务器
	this->send_back_info((void*)&this->sendCMDMsgFrame,sizeof(CMDMsgFrame));
}
/******************************************************************************
模块名称:  Client_List
模块功能:  在线列表
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      
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
模块名称:  DirHelper
模块功能:  目录操作类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      GetFileNameByPath调用后需要使用delete[]删除缓存
/******************************************************************************/
char* DirHelper::FtpServerMainDir = new char[MAX_CMD_SIZE];

bool DirHelper::InitMainDir(const char* pzMainDir)
{
	char* buff = new char[MAX_CMD_SIZE];
	ACE_OS::memset(DirHelper::FtpServerMainDir,0,MAX_CMD_SIZE);
	ACE_OS::memset(buff,0,MAX_CMD_SIZE);

	ACE_OS::getcwd(DirHelper::FtpServerMainDir,MAX_CMD_SIZE);	//获取当前工作目录
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
//(strtok_r会修改参数的值!!!!!!!,使用new申请空间会有释放问题，这里使用栈空间来处理)	
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

	//路径正确性检查，有多个\\\\时直接替换成一个
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
//设置Linux路径(将//符号替换成\符号)
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
//(strtok_r会修改参数的值!!!!!!!,使用new申请空间会有释放问题，这里使用栈空间来处理)	
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


//(strtok_r会修改参数的值!!!!!!!,使用new申请空间会有释放问题，这里使用栈空间来处理)	
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
		ACE_OS::strcpy(fileName,token);	//取到最后一个就是文件名;
		token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
	}
	return fileName;
}
//(strtok_r会修改参数的值!!!!!!!,使用new申请空间会有释放问题，这里使用栈空间来处理)	
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

		if(token != NULL)//除了最后一个其他的都是路径
			ACE_OS::strcat(retDirPath,DirName);
	}
	return retDirPath;
}

