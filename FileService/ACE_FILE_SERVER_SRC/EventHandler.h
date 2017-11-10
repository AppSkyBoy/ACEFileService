/******************************************************************************
模块名称:  公有类对象(server和client共用对象)
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      

Common:公用数据结构(server包,client包,包类型)
LocalFile.本地文件对象;  用于Server的文件写操作及Client的文件读操作
Bass_Event_Handler:事件处理基类
File_Recv_Event_Handler:文件接收处理类
File_Send_Event_Handler:文件发送处理类

流程:建立连接-->发送文件目录->发送文件名-->发送文件大小-->发送文件内容
******************************************************************************/
#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Stream.h" 
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"
#include "ace/FILE_IO.h"
#include "ace/Reactor.h"
#include "ace/Free_List.h"

#include "ace/Map_Manager.h"
#include "ace/Null_Mutex.h"
#include "ace/Task.h"

#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"
//#include "ace/Containers_T.h"

#include "ace/OS.h"

//#include <ace/Lock.h>
#include <ace/Synch.h>

#include "LocalFile.h"
#include "Common.h"
#include "ClientMothod.h"

#include <fstream>
#include <string>
using namespace std;
#define UNICODE


/******************************************************************************
模块名称:  CallBackFunction:
模块功能:  事件回调事件(Client比较关注的事件)
创建者:    夏
创建日期:  2012/10
更新日期:
备注:		1.这里触发的事件会更新到Client注册的事件中
			2.Bass_Event_Handler派生类都可以使用这个对象更新界面

因为c++不支持类函数回调，就没有在Bass_Event_Handler的派生类中实现回调函数，client
需要的事件是通过CallBackFunction里面的方法通知界面改变的(文件发送进度，成功与否的事件
都是这个对象来通知界面的(整个实现是基于异步机制)

//以下为回调类型，具体设置在ClientMothod.h头文件中
#define CALLBACK_TYPE_SEND_PROGRESS 1001
#define CALLBACK_TYPE_SEND_SUCESSED	1002
#define CALLBACK_TYPE_SEND_FAILED	1003
#define CALLBACK_TYPE_RECV_PROGRESS 1004
#define CALLBACK_TYPE_RECV_SUCESSED	1005
#define CALLBACK_TYPE_RECV_FAILED	1006
#define CALLBACK_TYPE_RECV_FILELIST 1007
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
		void OnGetFileListByPathImpl(char* text,bool isSucessed = true);
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
		void OnGetFileListByDateAndPathImpl(char* text,bool isSucessed = true);
};
/******************************************************************************
模块名称:  Bass_Event_Handler:
模块功能:  事件处理基类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      抽象sock的一般处理流程及回包处理
******************************************************************************/
class Bass_Event_Handler : public ACE_Event_Handler
{
public:
	Bass_Event_Handler(ACE_Reactor *r): ACE_Event_Handler(r){timeout.sec(ACE_DEFAULT_TIMEOUT);}
	virtual ~Bass_Event_Handler(){}
protected:
	ACE_INET_Addr		client_Addr_;
	ACE_SOCK_Stream		peer_sock_;
	//是否注册了WRITE_MASK消息
	bool isRegWRITE_MASK;
	//缓存需要发送的消息队列
	ACE_Message_Queue<ACE_NULL_SYNCH> output_queue;	
	//回调对象
	CallBackFunction	FCallBackFunction;
	ACE_Time_Value		timeout;
public:
	//向Reactor注册READ_TASK事件，开始监测peer_sock上的数据输入(并注册界面回调事件)
	virtual int Open(ACE_SOCK_Stream& sock,FWndCallBackHandle	FWndcallback = NULL);
	//往客户端发送数据
	virtual int handle_output(ACE_HANDLE = ACE_INVALID_HANDLE);
	//注销事件处理
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//当前sock对象
	virtual ACE_SOCK_Stream& peer() ;
	//指定当前sock对象
	ACE_SOCK_Stream& setpeer(ACE_SOCK_Stream& sock) ;
	ACE_INET_Addr& get_addr();
	//句柄
	virtual ACE_HANDLE get_handle (void) const;
protected:
	//发送回包
	virtual int send_back_info(void* pSendFrame,int sizeof_FrameType);
};
/******************************************************************************
模块名称:  File_Recv_Event_Handler
模块功能:  文件接收处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理文件接收操作
			Server:建立连接-->获取文件名-->获取文件大小-->获取文件内容
			Client:建立连接-->获取指定的文件-->(服务器存在时)获取文件大小-->获取文件内容
			Client接收文件时，服务器通过FILE_RECV_CLIENT_GET_FILENAME消息判断是否存在要接收的文件
			存在则直接下发文件大小，按正常流程走;
****************************************************************************/
class File_Recv_Event_Handler : public Bass_Event_Handler
{
private:
	//本地文件对象
	LocalFile localFile;	
	//来自客户端的数据帧
	FileSendFrame recvFrame;
	//发送到客户端的数据帧
	FileRecvFrame sendFrame;	
	//文件接收进度
	int	iLastPrgress;
	//当前总共接收的大小
	long iTotalRecvSize;
	//要接收的文件大小
	long RemotefileSize;
public:
	File_Recv_Event_Handler(ACE_Reactor *r);
	~File_Recv_Event_Handler(){}
	//接收客户端的数据
	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//从服务器接收指定文件(client调用,开始则返回sock句柄，失败返回-1)
	int  recvFileFromServer(const char* RemotefileNamePath,
							const char* LocalDirPath);
private:
	//显示接收进度日志
	void showProgress(int recvSize);
	//收到文件名
	void recvFileNameMsg();
	//收到文件大小
	void recvFileSizeMsg();
	//收到文件数据
	void recvFileDataMsg();
	//收到文件发送结束消息
	void recvFileSendMsg();
	//收到文件发送端出错消息
	void recvFileSendErrorMsg();
	//收到发送端对同名文件的覆盖消息
	void recvOverWriteSameFileMsg();
};

/******************************************************************************
模块名称:  File_Send_Event_Handler
模块功能:  文件发送处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理文件发送的相关事件
		   Server:建立连接->发送文件名-->发送文件大小-->发送文件内容	
		   Client:建立连接->发送文件名-->发送文件大小-->发送文件内容	
*****************************************************************************/
class File_Send_Event_Handler : public Bass_Event_Handler
{
private:
	FileSendFrame sendFrame;
	FileRecvFrame recvFrame;
	//本地文件对象
	LocalFile localFile;
	//本地文件大小
	long iLocalFileSize;
	//发送进度
	int	 iSendPrgress;
	//已发送大小
	long iTotalSendSize;
public:
	File_Send_Event_Handler(ACE_Reactor *r);
	~File_Send_Event_Handler(){}

	//接收文件接收端的数据
	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//发送本地文件到服务器(client调用,开始则返回sock句柄，失败返回-1)
	int  sendFileToServer(const char* LocalfileNamePath,
						  const char* RemoteDirPath);
private:
	//显示发送进度
	void showSendProgress(int readSize);
	//(消息发送函数)
	//发送文件名
	void sendFileName();
	//发送文件大小
	void sendFileSize();
	//发送文件数据
	void sendFileData();
	//是否覆盖同名文件
	void sendIsOverWriteSameFile();
	//返回Client获取server里面指定文件的回执
	void checkedLocalFileIsExistAndSendResult();
};

/******************************************************************************
模块名称:  CMDMsg_Recv_Send_Event_Handler
模块功能:  消息文本的发送和接收类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      当有消息文本发送到Client时通过这个类进行处理(Client也通过这个类接收文本)
*****************************************************************************/
class CMDMsg_Recv_Send_Event_Handler:public Bass_Event_Handler
{
private:
	CMDMsgFrame	sendCMDMsgFrame;
	CMDMsgFrame	recvCMDMsgFrame;

	char*	RecvCMDMsg;
	char*	SendCMDMsg;
	//已接收消息长度
	int	 iCMDMsgRecvLenth;
	//已发送消息长度
	int	 iCMDMsgSendLenth;
	//总消息长充
	long iCMDMsgTotalLenth;
	//当前的消息类型
	CilentCMDFrameType	CMDFrameType;
public:
	//sendCMDMsg为要发送的内容;由服务器设定
	CMDMsg_Recv_Send_Event_Handler(CilentCMDFrameType type,ACE_Reactor *r,const char* sendCMDMsg = NULL);
	~CMDMsg_Recv_Send_Event_Handler(){}

	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask mask = 0);
	//接收消息文本到客户端(client 调用)
	void recvCMDMsgFramServer();
};
/******************************************************************************
模块名称:  Client_List
模块功能:  在线列表
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      
/******************************************************************************/
class Client_List
{
private:
	ACE_Map_Manager<ACE_HANDLE,Bass_Event_Handler*,ACE_Thread_Mutex> Fmap;		
public:
	static Client_List* Instens();
private:
	Client_List(){}
	~Client_List(){Fmap.unbind_all();}
public:
	void	AddEvent(ACE_HANDLE Handle,Bass_Event_Handler* ptr);	
	Bass_Event_Handler* GetEventByHandle(ACE_HANDLE Handle);
	Bass_Event_Handler* RemoveEventByHandle(ACE_HANDLE Handle);
	int		GetCount();	
	void	Clear();
};
/******************************************************************************
模块名称:  DirHelper
模块功能:  目录操作类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      strtok_r是linux平台下的strtok函数的线程安全版

		   GetFileNameByPath调用后需要使用delete[]删除缓存
/******************************************************************************/
class DirHelper
{
private:
	static char* FtpServerMainDir;	
public:
	static bool  InitMainDir(const char* pzMainDir);

	//服务器调用:Client操作的都是服务器的相对路径,服务器的真实路径 = FtpServerMainDir + 相对路径;
	//(调用后使用delete[]删除缓存)
	static char* GetServerLocalDirPath(const char* pzPath);

	//检查所有路径的合法性，不存在则自动建立(!!!参数为服务器相对路径)
	static bool  CreateServerDirPath(const char* pzPath);
	//检查目录是否存在(参数为服务器相对路径)
	static bool  CheckedServerDirPathIsExist(const char* pzPath);
	//检查指定目录是否存在(参数为绝对路径)
	static bool	 CheckedDirPathIsExist(const char* pzPath);

	//以下2个方法使用strtok_r分隔路径(参数可以为相对路径,也可以为绝对路径)
	//获取路径(调用后使用delete[]删除缓存)
	static char* GetFileNameByPath(const char* pzPath);
	//获取文件名(调用后使用delete[]删除缓存)
	static char* GetFilePathByName(const char*  pzFullFileName);
	//设置Linux路径(将//符号替换成\符号)
	static void  SetLinuxPath(char* outPath);
};
#endif

