#ifndef SOCKSERVER_H
#define SOCKSERVER_H

#include "Common.h"
#include "EventHandler.h"
#include "ace/Task.h"
/******************************************************************************
模块名称:  CMD_Event_Handler
模块功能:  命令处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理Client的CMD		   
*****************************************************************************/
class CMD_Event_Handler: public ACE_Task_Base
{
private:
	CilentCMDFrame	serverCMDRetFrame;
	CilentCMDFrame	clientCMDFrame;

	ACE_INET_Addr			client_Addr_;
	ACE_SOCK_Stream			peer_sock_;
	Bass_Event_Handler*		FEvent;
public:
	CMD_Event_Handler():ACE_Task_Base(){FEvent = NULL;}
	~CMD_Event_Handler(){}

	virtual int open(ACE_SOCK_Stream&	sock)
	{
		this->peer_sock_ = sock;
		this->peer_sock_.get_remote_addr(client_Addr_);
		return this->activate()	;
	}
	virtual int svc(void);
private:
	//发送文件
	bool recvFileSendCMD();
	//文件接收
	bool recvFileRecvCMD();
	//获取文件列表
	bool recvFileGetListCMD();
	//文件删除
	bool recvFileDelCMD();
	//目录创建
	bool recvDirCreateCMD();
	//目录删除
	bool recvDirDelCMD();
	//当前连接数
	bool recvGetConnectedCountCMD();
	//通过时间范围获取录相文件列表
	bool recvFileListByDateCMD();
};

/******************************************************************************
模块名称:  文件服务器
模块功能:  Server
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      Server:服务器类,处理Client的连接

流程:建立连接-->发送文件目录->发送文件名-->发送文件大小-->发送文件内容
其他:这里为了方便在Linux下进行编译，将多个类实现放到了一个文件里;
******************************************************************************/
class Server  : public ACE_Event_Handler
{ 
private: 
	ACE_SOCK_Acceptor	peer_acceptor_; 
	CilentCMDFrame		serverCMDRetFrame;
public: 
	Server(ACE_Reactor *r = ACE_Reactor::instance()) : ACE_Event_Handler(r) {}
	virtual ~Server();

	//打开端口注册，并开始监听，默认端口号是66666
	int open(int port = 66666);
	//监听连接
	int handle_input(ACE_HANDLE);
	int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = 0);
	ACE_HANDLE get_handle(void) const ;
};

#endif

