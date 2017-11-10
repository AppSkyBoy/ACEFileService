/******************************************************************************
模块名称:  基础类型
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      
******************************************************************************/
#define UNICODE

#ifndef COMMON_H
#define COMMON_H

//文件发送端数据包的最大缓存大小
const int MAX_BUFFER_SIZE = 8*1024;

//文件接收端数据包的最大信息大小
const int MAX_INFO_SIZE = 256;

//CMD数据包的最大信息大小(以及char类型Buff大小)
const int MAX_CMD_SIZE = 512;

//CMD消息文本数据包最大信息大小
const int MAX_CMD_MSG_SIZE = 6*1024;

//文件发送包数据类型
enum FileSendFrameType
{
	FILE_SEND_NAME,					//文件名称
	FILE_SEND_SIZE,					//文件大小
	FILE_SEND_DATA,					//文件数据	
	FILE_SEND_END,					//文件结尾
	FILE_SEND_ERROR,				//文件出错
	FILE_SEND_OVERWRITE,			//覆盖文件
	FILE_SEND_DISCONNECT,			//断开连接
	FILE_SEND_FILE_NOTEXIST,		//客户端下载指定文件时服务器端返回文件不存在(存在就直接按正常的发送流程走)
	FILE_SEND_INVALID,				//保留的无效位
};

//文件接收端包数据类型
enum FileRecvType
{	
	FILE_RECV_CLIENT_GET_FILENAME,	//获取文件名(client先上传文件名路径，server检查是否存在再进行发送)
	FILE_RECV_GET_FILENAME,			//获取文件名
	FILE_RECV_GET_FILESIZE,			//获取文件大小
	FILE_RECV_GET_FILEDATA,			//获取文件数据(用于验证,发送端收到后持续发送文件数据直到结束)
	FILE_RECV_FILE_EXIST,			//发送端\文件已经存在
	FILE_RECV_DIR_NOTEXIST,			//接收端目录路径不存在
	FILE_RECV_FINISHED,				//文件传输并保存成功
	FILE_RECV_INVALID,				//保留的无效位，主要用来简化程序的控制逻辑
	FILE_RECV_ERROR,				//出错,表示接收失败,通知Client不再发送文件数据
};

//CMD消息文本数据包
enum CMDMsgFrameType
{
	RECV_MSG_LENTH,			//消息文本长度
	RECV_MSG_DATA,			//消息文本内容
	RECV_MSG_END,			//消息接收完成

	SEND_MSG_LENTH,
	SEND_MSG_DATA,
	SEND_MSG_END,

	MSG_INVALID,		   //保留的无效位
};

//CMD类型
enum CilentCMDFrameType
{
	CLIENT_FILE_SEND,		//文件发送
	CLIENT_FILE_RECV,		//文件接收
	CLIENT_FILE_GET_LIST,	//获取文件列表
	CLIENT_FILE_GET_LIST_BYDATE,//通过时间获取文件列表
	CLIENT_FILE_DEL,		//文件删除
	CLIENT_DIR_CREATE,		//创建目录
	CLIENT_GETCONNECTEDCOUNT,	//获取服务器连接数
	CREATE_DIR_DEL,			//删除目录

	CLIENT_DISCONNECT,		//断开连接
	SERVER_INVALID,			//保留的无效位
	SERVER_OK,				//服务器已准备好(接收到的消息返回值保存在对应的包里)
	SERVER_ERROR,			//服务器报错		
};
//-------------------------------------------------
//文件发送数据包
struct FileSendFrame
{
	FileSendFrameType type;			//类型
	int length;						//包实际大小
	char buffer[MAX_BUFFER_SIZE];	//数据
};
//-------------------------------------------------
//文件接收端数据包
struct FileRecvFrame
{
	FileRecvType type;			
	int length;					
	char info[MAX_INFO_SIZE];	
};
//-------------------------------------------------
//CMD数据包(client<--->server)
struct CilentCMDFrame
{
	CilentCMDFrameType type;		
	int length;						
	char CMD[MAX_CMD_SIZE];			
};
//-------------------------------------------------
//CMD数据包返回数据包
struct CMDMsgFrame
{
	CMDMsgFrameType type;		
	int length;						
	char CMDMsg[MAX_CMD_MSG_SIZE];	
};

#endif

