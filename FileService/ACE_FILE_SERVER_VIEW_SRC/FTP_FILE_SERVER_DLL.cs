using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace ACE_FILE_SERVER_VIEW
{
    public class FTP_FILE_SERVER_DLL
    {

        //CMDMsg这里通过指针转换得到返回的字符串,直接使用string有时会出现内存访问错误，如"没有足够的内存继续执行程序"
        /*//使用时使用Marshal.PtrToStringAnsi(sender.CMDMsg);转换
        [StructLayout(LayoutKind.Sequential)]
        public class FWndCallBackObject
        {
            //[MarshalAs(UnmanagedType.LPStr)]            
            public IntPtr CMDMsg;			//一般是消息返回值或错误信息
            //public string CMDMsg;
            public  int     iProgress;			//下载/上传进度(0~100)
            public  bool    isSucessed;			//CMD是否执行成功(如创建目录，删除文件/目录等)
            public  int     HANDLE;				//对应返回的句柄值(如send和recv文件方法的返回句柄值)
        }

        //[System.Runtime.InteropServices.UnmanagedFunctionPointerAttribute(System.Runtime.InteropServices.CallingConvention.StdCall)]
        //[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void FWndCallBackHandle(int callBackType,ref FWndCallBackObject sender);

        public const int CALLBACK_TYPE_SEND_PROGRESS    =1001;
        public const int CALLBACK_TYPE_SEND_SUCESSED	=1002;
        public const int CALLBACK_TYPE_SEND_FAILED	    =1003;
        public const int CALLBACK_TYPE_RECV_PROGRESS    =1004;
        public const int CALLBACK_TYPE_RECV_SUCESSED	=1005;
        public const int CALLBACK_TYPE_RECV_FAILED	    =1006;
        public const int CALLBACK_TYPE_RECV_FILELIST    =1007;
        public const int CALLBACK_TYPE_RECV_FILEDEL     =1008;
        public const int CALLBACK_TYPE_RECV_DIRCREATE   =1009;
        public const int CALLBACK_TYPE_RECV_DIRDEL      =1010;
        public const int CALLBACK_TYPE_SEND_ISOVERWRITE = 1011;
        public const int CALLBACK_TYPE_RECV_ISOVERWRITE = 1012;*/

        /*---------------------------------------------------------------------------------
        模块名称:	发送文件到服务器
        参数说明:	IP:文件服务器IP
			        port:文件服务器登录端口
			        fileNamePath:为相对路径,例如:/录相管理/10024/2012-05-21.avi
			        callback:回调函数
        返回值:		失败-1/成功返回句柄
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern int FTPsendFileToServer(
								        string LocalfileNamePath,
								        string RemoteDirPath,
                                        ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);

        /*---------------------------------------------------------------------------------
        模块名称:	接收文件到本地
        参数说明:	IP:文件服务器IP
			        port:文件服务器登录端口
			        RemotefileNamePath:为服务器相对路径,例如:/录相管理/10024/2012-05-21.avi
			        LocalDirPath:本地接收文件夹
			        callback:回调函数
        返回值:		失败-1/成功返回句柄
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern int FTPrecvFileFramServer(
								          string RemotefileNamePath,
								          string LocalDirPath,
                                          ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  获取文件列表
        参数说明:  
        返回值:		失败-1/成功则返回0(消息内容通过CALLBACK_TYPE_RECV_FILELIST返回
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern int FTPrecvFileListFramServer(
									          string path,
                                              ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  根据时间段获取文件列表
        参数说明:  beginDate/endDate时间格式:20121206141817	
        返回值:		失败-1/成功则返回0(消息内容通过CALLBACK_TYPE_RECV_FILELIST返回
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern int FTPrecvFileListByDateFramServer(
                                              string path,
                                              string beginDate,
                                              string endDate,
                                              ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  创建目录
        参数说明:  
        返回值:		成功/失败
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern bool FTPcreateDirFramServer(
								           string RemoteDirPath,
                                           ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  删除目录
        参数说明:  
        返回值:		成功/失败
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern bool FTPdelDirFramServer(
								           string RemoteDirPath,
                                           ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  删除文件
        参数说明:  
        返回值:		成功/失败
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern bool FTPdelFileFramServer(
								        string RemoteFilePath,
                                        ACE_FILE_SERVER_DLL.FWndCallBackHandle callback);
        /*---------------------------------------------------------------------------------
        模块名称:  停止发送或接收文件
        模块功能:  
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern void FTPstopFileSendOrRecvByHandle(int Handle);
        /*---------------------------------------------------------------------------------
        模块名称:  保持连接
        模块功能:  成功返回连接数/失败返回-1(FTP返回的不是服务器连接数)
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern int FTPgetServerConnetedCount();
        /*---------------------------------------------------------------------------------
        模块名称:  初始化连接
        模块功能:  成功返回连接数/失败返回-1
        /*---------------------------------------------------------------------------------*/
        [DllImport("FTP_Connecter.dll")]
        public static extern void FTPInitConnected(string IP, int Port, string userName, string password);
    }
}
