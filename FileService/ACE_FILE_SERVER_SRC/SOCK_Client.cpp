/******************************************************************************
模块名称:  文件接收客户端
模块功能:  Client
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      Client:客户端类,处理与Server端的通讯
		   Common:公用数据结构(server包,client包,包类型)
		   LocalFile.本地文件对象;  用于Server的文件写操作及Client的文件读操作

		   流程:建立连接-->发送文件目录->发送文件名-->发送文件大小-->发送文件内容
		   其他:这里为了方便在Linux下进行编译，将多个类实现放到了一个文件里;
******************************************************************************/
#include "LocalFile.h"
#include "SOCK_Client.h"




//发送操作命令到服务器端,执行成功后返回值保存在revcFrame里面,可根据需要进行处理
bool Client::sendAndReceive()
{
	int retSize = this->peer_sock.send_n(&(this->sendFrame), sizeof(CilentCMDFrame));
	if(retSize == -1)
	{
		ACE_DEBUG((LM_ERROR,L"Unable to send data\n")); 
		return false;
	}

	retSize = this->peer_sock.recv_n(&(revcFrame), sizeof(CilentCMDFrame));
	if(retSize == -1)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("recv data error\n")));
		return false;
	}
	switch (revcFrame.type)
	{
		case SERVER_OK:
			return true;
		case SERVER_ERROR:
			return false;
	}
	return false;
}
//连接服务器
bool Client::connectServer(const char *serverName, int port,FWndCallBackHandle callback)
{
	//注册界面回调事件	
	this->FCallBackFunction.InitWndCallBack(callback);

	this->serverAddr.set(port, serverName);

	ACE_DEBUG ((LM_ERROR,L"connectServer(%s)\n", serverName)); 

	return this->conn.connect(peer_sock, serverAddr) == 0;
}
//发送文件
int Client::sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath)
{	
	try
	{
		if(ACE_OS::strlen(LocalfileNamePath) >= MAX_CMD_SIZE)
		{
			ACE_DEBUG ((LM_ERROR,L"FileName is is too long(%s)\n", LocalfileNamePath)); 
		}
		else if(LocalFile::fileExist(LocalfileNamePath))
		{
			this->sendFrame.type = CLIENT_FILE_SEND;
			this->sendFrame.length = 0;

			if(this->sendAndReceive())
			{
				File_Send_Event_Handler* fileSend = new File_Send_Event_Handler(new ACE_Reactor());
				fileSend->Open(peer_sock,FCallBackFunction.FWndcallback);
				int iHandle = fileSend->sendFileToServer(LocalfileNamePath,RemoteDirPath);

				if(iHandle != -1)
				{
					//消息循环
					ReactorLoopThread* thread = new ReactorLoopThread();
					thread->open(fileSend->reactor());
					return iHandle;
				}else
				{
					FCallBackFunction.OnFSFailedCallBackImpl();
				}

			}
			else
			{
				FCallBackFunction.OnFSFailedCallBackImpl();
				ACE_DEBUG((LM_DEBUG,L"Failed to send files(%s)\n", LocalfileNamePath)); 
			}
		}
		else
		{
			FCallBackFunction.OnFSFailedCallBackImpl();
			ACE_DEBUG((LM_ERROR,L"The file does not exist(%s)\n", LocalfileNamePath)); 
		}
	}
	catch (const char* error)
	{	
		ACE_DEBUG((LM_ERROR,L"sendFileToServer error:(%s)\n", error)); 
	}
	this->peer_sock.close();
	return -1;
}
//接收文件
int Client::recvFileFrameServer(const char* RemotefileNamePath,
								const char* LocalDirPath)
{
	try
	{
		char localPath[MAX_INFO_SIZE];
		ACE_OS::memset(localPath,0,MAX_INFO_SIZE);
		char* _RemotefilePath = const_cast<char*>(RemotefileNamePath);
		char* fileName = DirHelper::GetFileNameByPath(_RemotefilePath);
		ACE_OS::sprintf(localPath,"%s\\%s",LocalDirPath,fileName);
		delete[] fileName;

		if(ACE_OS::strlen(RemotefileNamePath) >= MAX_CMD_SIZE)
		{
			ACE_DEBUG ((LM_ERROR,L"FileName is is too long(%s)\n", RemotefileNamePath)); 
		}
		else if(LocalFile::fileExist(localPath))
		{
			char buf[MAX_CMD_SIZE];
			ACE_OS::sprintf(buf,"本地已存在文件\"%s\"请确认是否覆盖该文件?:",localPath);
			bool isOverWrite = this->FCallBackFunction.OnFROverWriterCallBackImpl(buf);
			if(isOverWrite)
			{
				if(!LocalFile::deleteFile(localPath))
				{
					ACE_DEBUG((LM_DEBUG,ACE_TEXT("OverWriter files failed[%s]\n"),localPath));	
					return -1;
				}
				ACE_DEBUG((LM_DEBUG,ACE_TEXT("OverWriter files Sucessed[%s]\n"),localPath));			
			}
			else
				return -1;
		}

		this->sendFrame.type = CLIENT_FILE_RECV;
		this->sendFrame.length = 0;
		if(this->sendAndReceive())
		{
			File_Recv_Event_Handler* fileRecv = new File_Recv_Event_Handler(new ACE_Reactor());
			fileRecv->Open(peer_sock,FCallBackFunction.FWndcallback);
			int iHandle = fileRecv->recvFileFromServer(RemotefileNamePath,LocalDirPath);

			if(iHandle != -1)
			{
				//消息循环
				ReactorLoopThread* thread = new ReactorLoopThread();
				thread->open(fileRecv->reactor());
				return iHandle;
			}
			else
				FCallBackFunction.OnFRFailedCallBackImpl();
		}
		else
		{
			FCallBackFunction.OnFRFailedCallBackImpl();
			ACE_DEBUG((LM_DEBUG,L"Failed to receive files(%s)\n", RemotefileNamePath)); 		
		}	
	}
	catch (const char* error)
	{	
		ACE_DEBUG((LM_ERROR,L"recvFile error:(%s)\n", error)); 
	}		
	this->peer_sock.close();
	return -1;
}	
//获取目录列表
int Client::recvFileListFramServer(const char* path)
{
	try
	{
		this->sendFrame.type = CLIENT_FILE_GET_LIST;
		ACE_OS::strcpy(this->sendFrame.CMD, path);
		this->sendFrame.length = ACE_OS::strlen(path);
		if(this->sendAndReceive())
		{
			CMDMsg_Recv_Send_Event_Handler*	cmdMsgRecv = new CMDMsg_Recv_Send_Event_Handler(this->sendFrame.type,new ACE_Reactor());
			cmdMsgRecv->Open(peer_sock,FCallBackFunction.FWndcallback);
			cmdMsgRecv->recvCMDMsgFramServer();

			//消息循环
			ReactorLoopThread* thread = new ReactorLoopThread();
			thread->open(cmdMsgRecv->reactor());
			return 0;
		}
		else
		{
			FCallBackFunction.OnGetFileListByPathImpl("",false);
			ACE_DEBUG((LM_DEBUG,L"Failed to receive filelist(%s)\n", path)); 
			return -1;
		}
	}
	catch(const char* error)
	{
		ACE_DEBUG((LM_ERROR,L"recvFileList error:(%s)\n", error)); 
	}
	FCallBackFunction.OnGetFileListByPathImpl("",false);
	ACE_DEBUG((LM_DEBUG,L"Failed to receive filelist(%s)\n", path)); 
	return -1;
}

//通过时间范围获取录相文件列表
int Client::recvFileListByDateFramServer(const char* path,const char* begindate,const char* enddate)
{
	try
	{
		this->sendFrame.type = CLIENT_FILE_GET_LIST_BYDATE;
		char buff[MAX_INFO_SIZE];
		ACE_OS::memset(buff,0,MAX_INFO_SIZE);
		ACE_OS::sprintf(buff,"%s?%s?%s",path,begindate,enddate);

		ACE_OS::strcpy(this->sendFrame.CMD, buff);
		this->sendFrame.length = ACE_OS::strlen(buff);
		if(this->sendAndReceive())
		{
			CMDMsg_Recv_Send_Event_Handler*	cmdMsgRecv = new CMDMsg_Recv_Send_Event_Handler(this->sendFrame.type,new ACE_Reactor());
			cmdMsgRecv->Open(peer_sock,FCallBackFunction.FWndcallback);
			cmdMsgRecv->recvCMDMsgFramServer();

			//消息循环
			ReactorLoopThread* thread = new ReactorLoopThread();
			thread->open(cmdMsgRecv->reactor());
			return 0;
		}
		else
		{
			FCallBackFunction.OnGetFileListByDateAndPathImpl("",false);
			ACE_DEBUG((LM_DEBUG,L"Failed to receive filelist(%s)\n", path)); 
			return -1;
		}
	}
	catch(const char* error)
	{
		ACE_DEBUG((LM_ERROR,L"recvFileListByDate error:(%s)\n", error)); 
	}
	FCallBackFunction.OnGetFileListByDateAndPathImpl("",false);
	ACE_DEBUG((LM_DEBUG,L"Failed to receive filelist(%s)\n", path)); 
	return -1;
}

//文件删除
bool Client::FileDel(const char* RemoteFilePath)
{
	if(ACE_OS::strlen(RemoteFilePath) > MAX_CMD_SIZE)
	{
		ACE_DEBUG((LM_ERROR,L"Dir create failed,filePath is too long(%s)\n",RemoteFilePath)); 
		return false;
	}

	this->sendFrame.type = CLIENT_FILE_DEL;	
	ACE_OS::strcpy(this->sendFrame.CMD,RemoteFilePath);
	this->sendFrame.length = ACE_OS::strlen(this->sendFrame.CMD);
	bool ret = this->sendAndReceive();	
	FCallBackFunction.OnDelFileImpl(ret);
	ACE_DEBUG((LM_DEBUG,L"Del file failed %s(%s)\n",ret ? "success":"failed",
											this->sendFrame.CMD)); 
	return true;
}
//目录创建
bool Client::DirCreate(const char* RemoteDirPath)
{
	if(ACE_OS::strlen(RemoteDirPath) > MAX_CMD_SIZE)
	{
		ACE_DEBUG((LM_ERROR,L"Dir create failed,Path is too long(%s)\n", RemoteDirPath)); 
		return false;
	}

	this->sendFrame.type = CLIENT_DIR_CREATE;
	ACE_OS::strcpy(this->sendFrame.CMD,RemoteDirPath);
	this->sendFrame.length = ACE_OS::strlen(this->sendFrame.CMD);
	bool ret = this->sendAndReceive();
	FCallBackFunction.OnCreateDirImpl(ret);
	ACE_DEBUG((LM_DEBUG,L"Dir create failed %s(%s)\n",ret ? "success":"failed",
											this->sendFrame.CMD)); 
	return ret;
}
//目录删除
bool Client::DirDel(const char* RemoteDirPath)
{	
	if(ACE_OS::strlen(RemoteDirPath) > MAX_CMD_SIZE)
	{
		ACE_DEBUG((LM_ERROR,L"Dir delete failed,Path is too long(%s)\n", RemoteDirPath)); 
		return false;
	}

	this->sendFrame.type = CREATE_DIR_DEL;
	ACE_OS::strcpy(this->sendFrame.CMD,RemoteDirPath);
	this->sendFrame.length = ACE_OS::strlen(this->sendFrame.CMD);
	bool ret = this->sendAndReceive();
	FCallBackFunction.OnDelDirImpl(ret);
	ACE_DEBUG((LM_DEBUG,L"Dir delete failed %s(%s)\n",ret ? "success":"failed",
							this->sendFrame.CMD)); 
	return ret;
}
//获取服务器连接数
int Client::getConnectedCount()
{
	this->sendFrame.type = CLIENT_GETCONNECTEDCOUNT;
	this->sendFrame.length = 0;
	if(this->sendAndReceive())
	{
		this->revcFrame.CMD[this->revcFrame.length+1] = '\0';
		return ACE_OS::atoi(this->revcFrame.CMD);
	}
	return -1;
}