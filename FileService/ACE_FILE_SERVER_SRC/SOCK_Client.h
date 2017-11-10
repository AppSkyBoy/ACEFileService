/******************************************************************************
模块名称:  客户端
模块功能:  Client
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      Client:客户端类,处理与Server端的通讯,通过指定命令执行操作
Common:公用数据结构(server包,client包,包类型)
LocalFile.本地文件对象;  用于Server的文件写操作及Client的文件读操作

流程:建立连接-->发送文件目录->发送文件名-->发送文件大小-->发送文件内容
//注意:在服务器没有实现长连接，这里每次只能执行一次命令，再将执行需要重新申请对象
******************************************************************************/
#ifndef SOCKCLIENT_H
#define SOCKCLIENT_H

#include "ace/SOCK_Connector.h"
//ACE线程
#include "ace/OS.h"
#include "ace/Task.h"

#include "ClientMothod.h"
#include "Common.h"
#include "EventHandler.h"

/******************************************************************************
模块名称:  ReactorLoopThread:
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      ACE_Reactor的消息循环线程;
******************************************************************************/
class ReactorLoopThread:public ACE_Task_Base
{
private:
	ACE_Reactor* reactor;
public:
	ReactorLoopThread(){this->reactor = NULL;}
public:
	virtual int open(ACE_Reactor* r)
	{
		this->reactor = r;
		return this->activate()	;
	}
	virtual int svc(void)
	{
		if(this->reactor != NULL)
			this->reactor->run_reactor_event_loop();			
		return 0;
	}
};

class Client
{
public:
	Client(){}
	virtual ~Client(){}
private:
	ACE_SOCK_Connector		conn;
	ACE_SOCK_Stream			peer_sock;
	ACE_INET_Addr			serverAddr;	

	CilentCMDFrame   		sendFrame;
	CilentCMDFrame			revcFrame;

	CallBackFunction		FCallBackFunction;
private:
	//发送操作命令到服务器端
	bool sendAndReceive();
public:
	//连接服务器
	bool connectServer(const char *serverName, int port,FWndCallBackHandle callback = NULL);
	//发送文件
	int sendFileToServer(const char* LocalfileNamePath,const char* RemoteDirPath);
	//接收文件
	int recvFileFrameServer(const char* RemotefileNamePath,const char* LocalDirPath);
	//获取目录列表
	int recvFileListFramServer(const char* path);
	//文件删除
	bool FileDel(const char* RemoteFilePath);
	//目录创建
	bool DirCreate(const char* RemoteDirPath);
	//目录删除
	bool DirDel(const char* RemoteDirPath);
	//获取服务器连接数
	int getConnectedCount();
	//通过时间范围获取录相文件列表
	int recvFileListByDateFramServer(const char* path,const char* begindate,const char* enddate);
};
#endif