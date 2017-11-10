using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ACE_FILE_SERVER_VIEW
{
    /******************************************************************************
    模块名称:  工厂类
    模块功能:  根据需要升成具体的传输对象;
    创建者:    夏
    创建日期:  2012/12
    更新日期:
    ******************************************************************************/
    public class FileServerFactory
    {
        public static FileServerBase CreateAceFileServer()
        {
            return new ACEFileServer();
        }
        public static FileServerBase CreateFtpFileServer()
        {
            return new FTPFileServer();
        }
    }
    /******************************************************************************
    模块名称:  传输层基类
    创建者:    夏
    创建日期:  2012/12
    更新日期:
    备注:      后期需要支持FTP和SOCKET两种方式这里将传输层进行了抽象
    ******************************************************************************/
    public abstract class FileServerBase
    {
        protected string IP;
        protected int Port;
        protected string userName;
        protected string passWord;
        public virtual void InitConnected(string IP, int Port, string userName, string password)
        {
            this.IP = IP;
            this.Port = Port;
            this.userName = userName;
            this.passWord = password;
        }
        public abstract int  sendFileToServer(string LocalfileNamePath,string RemoteDirPath);
        public abstract int  recvFileFramServer(string RemotefileNamePath,string LocalDirPath);
        public abstract int  recvFileListFramServer(string RemoteDirPath);
        public abstract int recvFileListByDateFramServer(string RemoteDirPath,string beginDate,string endDate);
        public abstract bool delDirFramServer(string RemoteDirPath);
        public abstract bool createDirFramServer(string RemoteDirPath);
        public abstract bool delFileFramServer(string RemoteFilePath);
        public abstract void stopFileSendOrRecvByHandle(int Handle);
        public abstract int connectServer();
    }
    /******************************************************************************
    模块名称:  ACE传输层(Socket)
    创建者:    夏
    创建日期:  2012/12
    更新日期:
    备注:      
    ******************************************************************************/
    public class ACEFileServer : FileServerBase
    {
        public ACE_FILE_SERVER_DLL.FWndCallBackHandle OnFWndCallBack;
        public override int sendFileToServer(string LocalfileNamePath, string RemoteDirPath)
        {
            return ACE_FILE_SERVER_DLL.sendFileToServer(this.IP, this.Port, LocalfileNamePath, RemoteDirPath, OnFWndCallBack);
        }
        public override int recvFileFramServer(string RemotefileNamePath, string LocalDirPath)
        {
            return ACE_FILE_SERVER_DLL.recvFileFramServer(this.IP, this.Port, RemotefileNamePath, LocalDirPath, OnFWndCallBack);
        }
        public override int recvFileListFramServer(string RemoteDirPath)
        {
            return ACE_FILE_SERVER_DLL.recvFileListFramServer(this.IP, this.Port, RemoteDirPath, OnFWndCallBack);
        }
        public override int recvFileListByDateFramServer(string RemoteDirPath, string beginDate, string endDate)
        {
            return ACE_FILE_SERVER_DLL.recvFileListByDateFramServer(this.IP, this.Port, RemoteDirPath,beginDate,endDate, OnFWndCallBack);
        }
        public override bool delDirFramServer(string RemoteDirPath)
        {
            return ACE_FILE_SERVER_DLL.delDirFramServer(this.IP, this.Port, RemoteDirPath, OnFWndCallBack);
        }
        public override bool delFileFramServer(string RemoteFilePath)
        {
            return ACE_FILE_SERVER_DLL.delFileFramServer(this.IP, this.Port, RemoteFilePath, OnFWndCallBack);
        }
        public override void stopFileSendOrRecvByHandle(int Handle)
        {
            ACE_FILE_SERVER_DLL.stopFileSendOrRecvByHandle(Handle);
        }
        public override bool createDirFramServer(string RemoteDirPath)
        {
            return ACE_FILE_SERVER_DLL.createDirFramServer(this.IP, this.Port, RemoteDirPath, OnFWndCallBack);
        }
        public override int connectServer()
        {
            return ACE_FILE_SERVER_DLL.getServerConnetedCount(this.IP, this.Port);
        }
    }
    /******************************************************************************
    模块名称:  FTP传输层
    创建者:    夏
    创建日期:  2012/12
    更新日期:
    备注:      
    ******************************************************************************/
    public class FTPFileServer : FileServerBase
    {
        public ACE_FILE_SERVER_DLL.FWndCallBackHandle OnFWndCallBack;
        public override int sendFileToServer(string LocalfileNamePath, string RemoteDirPath)
        {
            return FTP_FILE_SERVER_DLL.FTPsendFileToServer(LocalfileNamePath, RemoteDirPath, OnFWndCallBack);
        }
        public override int recvFileFramServer(string RemotefileNamePath, string LocalDirPath)
        {
            return FTP_FILE_SERVER_DLL.FTPrecvFileFramServer(RemotefileNamePath, LocalDirPath, OnFWndCallBack);
        }
        public override int recvFileListFramServer(string RemoteDirPath)
        {
            return FTP_FILE_SERVER_DLL.FTPrecvFileListFramServer(RemoteDirPath, OnFWndCallBack);
        }
        public override int recvFileListByDateFramServer(string RemoteDirPath, string beginDate, string endDate)
        {
            return FTP_FILE_SERVER_DLL.FTPrecvFileListByDateFramServer(RemoteDirPath, beginDate, endDate, OnFWndCallBack);
        }
        public override bool delDirFramServer(string RemoteDirPath)
        {
            return FTP_FILE_SERVER_DLL.FTPdelDirFramServer(RemoteDirPath, OnFWndCallBack);
        }
        public override bool delFileFramServer(string RemoteFilePath)
        {
            return FTP_FILE_SERVER_DLL.FTPdelFileFramServer(RemoteFilePath, OnFWndCallBack);
        }
        public override void stopFileSendOrRecvByHandle(int Handle)
        {
            FTP_FILE_SERVER_DLL.FTPstopFileSendOrRecvByHandle(Handle);
        }
        public override int connectServer()
        {
            return FTP_FILE_SERVER_DLL.FTPgetServerConnetedCount();
        }
        public override void InitConnected(string IP, int Port, string userName, string password)
        {
            base.InitConnected(IP, Port, userName, password);
            FTP_FILE_SERVER_DLL.FTPInitConnected(IP, Port, userName, password);
        }
        public override bool createDirFramServer(string RemoteDirPath)
        {
            return FTP_FILE_SERVER_DLL.FTPcreateDirFramServer(RemoteDirPath, OnFWndCallBack);
        }
    }
}
