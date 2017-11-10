#ifndef CLIENTMOTHOD_H
#define CLIENTMOTHOD_H

//#define VER_SERVER	//文件服务器端(Linux版本需要在Linux下重新编译)  (设置生成名为SOCK_Service.dll)
//#define VER_CLIENT    //客户端ＣＭＤ版本,用于测试使用 (设置SOCK_Server_Client.exe，生成后用startClient.bat执行测试)
#define VER_DLL         //客户端连接接口,用于二次开发  (设置生成名为SOCK_Connecter.dll)



/******************************************************************************
模块名称:  DLL导出方法
模块功能:  
创建者:    夏
创建日期:  2012/10
更新日期:
备注:     
备注:      生成dll: 1.修改ClientMothod.h的宏为VER_DLL	
					2.修改项目属性生成为.dll
					3.修改生成的dll文件名为SOCK_Connecter.dll,再编译即可
		   生成exe; 1.修改ClientMothod.h的宏为VER_SERVER	
					2.修改项目属性生成为.exe
					3.修改生成名为SOCK_Server.exe再编译即可;
 		   生成Linux版本的服务器:
					1.修改ClientMothod.h的宏为VER_SERVER
					2.需要在Linux下重新编译
*****************************************************************************/
struct FWndCallBackObject
{
	char*	CMDMsg;				//一般是消息返回值或错误信息
	int		iProgress;			//下载/上传进度(0~100)
	bool	isSucessed;			//CMD是否执行成功(如创建目录，删除文件/目录等)
	int		HANDLE;				//对应返回的句柄值(如send和recv文件方法的返回句柄值)
};

#ifdef VER_SERVER
typedef void( *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

#ifdef VER_CLIENT
typedef void(__stdcall *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

#ifdef VER_DLL
typedef void(__stdcall *FWndCallBackHandle)(int callBackType,FWndCallBackObject* object);
#endif

/*---------------------------------------------------------------------------------
说明:  回调类型
/*---------------------------------------------------------------------------------*/
#define CALLBACK_TYPE_SEND_PROGRESS 1001
#define CALLBACK_TYPE_SEND_SUCESSED	1002
#define CALLBACK_TYPE_SEND_FAILED	1003
#define CALLBACK_TYPE_RECV_PROGRESS 1004
#define CALLBACK_TYPE_RECV_SUCESSED	1005
#define CALLBACK_TYPE_RECV_FAILED	1006
#define CALLBACK_TYPE_RECV_FILELIST 1007
#define CALLBACK_TYPE_RECV_FILEDEL  1008
#define CALLBACK_TYPE_RECV_DIRCREATE 1009
#define CALLBACK_TYPE_RECV_DIRDEL    1010
//同名文件处理(提示信息在CMDMsg中,根据isSucessed返回判断结果)
#define CALLBACK_TYPE_SEND_ISOVERWRITE	1011
#define CALLBACK_TYPE_RECV_ISOVERWRITE	1012
#define CALLBACK_TYPE_RECV_FILELIST_BYDATE 1013

#ifdef VER_DLL
/*---------------------------------------------------------------------------------
模块名称:	发送文件到服务器
参数说明:	IP:文件服务器IP
			port:文件服务器登录端口
			fileNamePath:为相对路径,例如:/录相管理/10024/2012-05-21.avi
			callback:回调函数
返回值:		失败-1/成功返回句柄
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int sendFileToServer(const char* ip,
								int port,
								const char* LocalfileNamePath,
								const char* RemoteDirPath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:	接收文件到本地
参数说明:	IP:文件服务器IP
			port:文件服务器登录端口
			RemotefileNamePath:为服务器相对路径,例如:/录相管理/10024/2012-05-21.avi
			LocalDirPath:本地接收文件夹
			callback:回调函数
返回值:		失败-1/成功返回句柄
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileFramServer(const char* ip,
								  int port,
								  const char* RemotefileNamePath,
								  const char* LocalDirPath,
								  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  获取文件列表
参数说明:  
返回值:		失败-1/成功则返回0(消息内容通过CALLBACK_TYPE_RECV_FILELIST返回
//DirFileListXml的XML结构如下:
/*
<DFList>
     <DList>
        <Ditm>
              <dp></ dp >  //dirname
			  <tp></ tp >   //type
		      <ct></ ct>   //changename
	</ Ditm >
…
     </ DList >
     < FList >
        <Fitm>
            <fp></fp>     //filename
            <size></size >  //filesize
			<tp></tp>     //type
			<ct></ct>     //changename
        </ Fitm >
        … 
     </FList>
</ DFList >
*/
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListFramServer(const char* ip,
									  int port,
									  const char* path,
									  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  根据时间短获取文件列表
参数说明:  beginDate/endDate时间格式:20121206141817			
返回值:		失败-1/成功则返回0(消息内容通过CALLBACK_TYPE_RECV_FILELIST返回
//DirFileListXml的XML结构如下:
/*
<DFList>
     < FList >
        <Fitm>
            <fp></fp>     //filename
            <size></size >  //filesize
			<tp></tp>     //type
			<ct></ct>     //changename
        </ Fitm >
        … 
     </FList>
</ DFList >
*/
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int recvFileListByDateFramServer(const char* ip,
									  int port,
									  const char* path,
									  const char* beginDate,
									  const char* endDate,
									  FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  创建目录
参数说明:  
返回值:		成功/失败
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool createDirFramServer(const char* ip,
								   int port,
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  删除目录
参数说明:  
返回值:		成功/失败
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delDirFramServer(const char* ip,
								   int port,
								   const char* RemoteDirPath,
								   FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  删除文件
参数说明:  
返回值:		成功/失败
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) bool delFileFramServer(const char* ip,
								int port,
								const char* RemoteFilePath,
								FWndCallBackHandle callback);
/*---------------------------------------------------------------------------------
模块名称:  停止发送或接收文件
模块功能:  
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) void stopFileSendOrRecvByHandle(int Handle);
/*---------------------------------------------------------------------------------
模块名称:  获取服务器连接数
模块功能:  成功返回连接数/失败返回-1
/*---------------------------------------------------------------------------------*/
extern "C" _declspec(dllexport) int getServerConnetedCount(const char* ip,
														   int port);
#endif

#endif

