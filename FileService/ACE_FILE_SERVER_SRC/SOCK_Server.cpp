#include "SOCK_Server.h"

/******************************************************************************
模块名称:  文件接收服务器
模块功能:  Server
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      Server:服务器类,处理Client的连接

流程:建立连接-->发送文件目录->发送文件名-->发送文件大小-->发送文件内容
其他:这里为了方便在Linux下进行编译，将多个类实现放到了一个文件里;
******************************************************************************/
Server::~Server() 
{ 
	this->handle_close();
}
//打开端口注册，并开始监听，默认端口号
int Server::open(int port)
{
	ACE_INET_Addr addr(port);		

	if(this->peer_acceptor_.open(addr) == -1)
	{
		ACE_DEBUG((LM_DEBUG,"Starting server filed [port:%d]\n",port));
		return -1;
	}
	else if(ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK) == -1)
	{
		ACE_ERROR((LM_DEBUG,"Starting server filed [port:%d]\n",port));		
		return -1;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"Starting server at port:%d\n",addr.get_port_number()));
	}
	return 0;
}
//监听连接
int Server::handle_input(ACE_HANDLE)
{
	ACE_Time_Value		timeout (ACE_DEFAULT_TIMEOUT); 
	ACE_INET_Addr		clientAddr;
	ACE_SOCK_Stream		sock;

	if(this->peer_acceptor_.accept(sock,&clientAddr,&timeout) == -1)
	{
		return 0;
	}
	else
	{
		sock.get_remote_addr(clientAddr);
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("client connected:(%s:%d)\n"),clientAddr.get_host_addr(),clientAddr.get_port_number() ));			

		CMD_Event_Handler *eh = new CMD_Event_Handler();
		eh->open(sock);
		return 0;
	}
}

int Server::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
	if (this->peer_acceptor_.get_handle () != ACE_INVALID_HANDLE)
	{
		ACE_Reactor_Mask m = ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::DONT_CALL;
		this->reactor()->remove_handler (this, m);
		this->peer_acceptor_.close();

		ACE_DEBUG((LM_DEBUG,"Server Closed\n"));
	}
	return 0;
}
ACE_HANDLE Server::get_handle(void) const 
{ 
	return this->peer_acceptor_.get_handle(); 

} 
/******************************************************************************
模块名称:  CMD_Event_Handler
模块功能:  命令处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理Client的CMD		   
*****************************************************************************/
int CMD_Event_Handler::svc()
{
	int recvSize = peer_sock_.recv_n(&clientCMDFrame, sizeof(CilentCMDFrame)) ;
	if(0 == recvSize)
	{
		return -1;//正常退出
	}
	else if(-1 ==recvSize || recvSize != sizeof(CilentCMDFrame))
	{
		ACE_DEBUG((LM_ERROR,L"[CMD]Receive cmd error!(%s)\n",client_Addr_.get_host_addr())); 
		return 0;
	}

	//包安全性检查
	if(this->clientCMDFrame.length > MAX_CMD_SIZE)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]recv error,clientCMDFrame size too large[IP:%s]\n"),client_Addr_.get_host_addr()));
		return -1;
	}

	//发送回执给Client
	this->serverCMDRetFrame.type = SERVER_ERROR;
	this->serverCMDRetFrame.length = 0;
	try
	{
		switch(clientCMDFrame.type)
		{
		case CLIENT_FILE_SEND:				//文件发送操作
			if(this->recvFileRecvCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_FILE_SEND[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CLIENT_FILE_RECV:				//文件接收操作
			if(this->recvFileSendCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_FILE_RECV[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CLIENT_FILE_GET_LIST:			//获取文件目录
			if(this->recvFileGetListCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_FILE_GET_LIST[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CLIENT_FILE_GET_LIST_BYDATE:
			if(this->recvFileListByDateCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_FILE_GET_LIST_BYDATE[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CLIENT_FILE_DEL:				//文件删除
			if(this->recvFileDelCMD())
				this->serverCMDRetFrame.type = SERVER_OK;	
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_FILE_DEL[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CLIENT_DIR_CREATE:				//目录创建
			if(this->recvDirCreateCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_DIR_CREATE[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		case CREATE_DIR_DEL:				//目录删除
			if(this->recvDirDelCMD())
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CREATE_DIR_DEL[IP:%s]\n"),client_Addr_.get_host_addr()));
		case CLIENT_GETCONNECTEDCOUNT:
			if(recvGetConnectedCountCMD())  //连接数
				this->serverCMDRetFrame.type = SERVER_OK;
			ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]CLIENT_GETCONNECTEDCOUNT[IP:%s]\n"),client_Addr_.get_host_addr()));
			break;
		}	
	}	
	catch(...)
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[CMD]Exception Error[IP:%s]\n"),client_Addr_.get_host_addr()));
		this->serverCMDRetFrame.type = SERVER_ERROR;
		this->serverCMDRetFrame.length = 0;
	}

	//发送回执
	int sendSize = this->peer_sock_.send_n((void*)&this->serverCMDRetFrame, sizeof(CilentCMDFrame));
	if(-1 == sendSize)	//发送数据出错
	{
		ACE_DEBUG((LM_DEBUG,ACE_TEXT("[send_back]Can not Send feedback Info(%s)\n"),client_Addr_.get_host_addr()));
		this->peer_sock_.close();
		return -1;
	}

	//事件循环;
	if(FEvent)
	{
		FEvent->reactor()->run_reactor_event_loop();	
		delete FEvent;
	}
	return -1;
}

bool CMD_Event_Handler::recvFileSendCMD()
{
	//取消事件处理(发送/接收文件,以及发送CMD文本时需要注销事件，让其他派生类来自己处理连接;
	this->FEvent = new File_Send_Event_Handler(new ACE_Reactor());
	return this->FEvent->Open(peer_sock_) == 0;
}
bool CMD_Event_Handler::recvFileRecvCMD()
{
	//取消事件处理(发送/接收文件,以及发送CMD文本时需要注销事件，让其他派生类来自己处理连接;
	this->FEvent = new File_Recv_Event_Handler(new ACE_Reactor());
	return this->FEvent->Open(peer_sock_) == 0;
}
//返回文件列表
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

bool CMD_Event_Handler::recvFileGetListCMD()
{
	try
	{
		this->clientCMDFrame.CMD[this->clientCMDFrame.length] = '\0';
		char* serverDir = DirHelper::GetServerLocalDirPath(this->clientCMDFrame.CMD);

		ACE_DEBUG((LM_DEBUG,L"[FileListPath]%s[IP:%s]\n",this->clientCMDFrame.CMD,this->client_Addr_.get_host_addr())); 
#ifdef ACE_WIN32
		if(ACE_OS::strstr(serverDir,"\\..") != NULL)
		{
			delete serverDir;
			return false;
		}		
#else
		if(ACE_OS::strstr(serverDir,"/..") != NULL)
		{
			delete serverDir;
			return false;
		}		
#endif	

		ACE_DIR *root = ACE_OS::opendir(serverDir);	
		if(NULL == root)
		{
			return false;
			return false;	
		}

		char* mainDir = DirHelper::GetServerLocalDirPath("");
		char buff[MAX_CMD_SIZE];
		ACE_OS::memset(buff,0,MAX_CMD_SIZE);
		string dirlist = "";
		if(root)
		{  
			dirlist = "<DList>";

#ifdef ACE_WIN32
			while (ACE_OS::readdir(root))
			{
				if(ACE_OS::strcmp(root->dirent_->d_name,"..") == 0 || ACE_OS::strcmp(root->dirent_->d_name,".") == 0)
					continue;

				if (root->fdata_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//win32的
				{
					ACE_OS::sprintf(buff,"<Ditm><dp>%s</dp><ct>%s</ct></Ditm>\n", root->dirent_->d_name, "yyy-MM-dd hh:mm");
					dirlist += buff;
				}
			}
#else
			ACE_DIRENT*   dirp;   
			char time[30];
			while((dirp = ACE_OS::readdir(root))!=NULL)   
			{
				struct stat st;
				ACE_OS::sprintf(buff,"%s/%s",serverDir,dirp->d_name);

				if(ACE_OS::lstat(buff,&st) == -1)
					continue;

				if(ACE_OS::strcmp(dirp->d_name,"..") == 0 || ACE_OS::strcmp(dirp->d_name,".") == 0)
					continue;

				ACE_OS::memset(buff,0,MAX_CMD_SIZE);
				if(S_ISDIR(st.st_mode))
				{
					struct tm* t = ACE_OS::localtime(&st.st_ctime);
					ACE_OS::sprintf(time,"%4d-%02d-%02d- %02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min); 
					ACE_OS::sprintf(buff,"<Ditm><dp>%s</dp><ct>%s</ct></Ditm>\n", dirp->d_name,time);
					dirlist += buff;		
				}
			}
#endif			
			ACE_OS::closedir(root);
			dirlist += "</DList>";		
		}

		root = ACE_OS::opendir(serverDir);	
		if(root)
		{
			dirlist += "<FList>";
#ifdef ACE_WIN32
			while (ACE_OS::readdir(root))
			{
				if (!(root->fdata_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//win32的
				{
					long _totalsize = (long)root->fdata_.nFileSizeHigh * ((long)MAXDWORD +(long)1) + (long)root->fdata_.nFileSizeLow;
					long _size = _totalsize / (long)1024.0 / (long)1024.0;
					char tmp[20];
					if (_size > (long)1.0)
						ACE_OS::sprintf(tmp,"%dMB", _size);
					else
						ACE_OS::sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);

					ACE_OS::sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", root->dirent_->d_name,tmp,"yyy-MM-dd hh:mm");
					dirlist += buff;
				}
			}		
#else
			ACE_DIRENT*   dirp;   
			char tmp[20];
			char time[30];
			while((dirp = ACE_OS::readdir(root))!=NULL)   
			{
				struct stat st;
				ACE_OS::sprintf(buff,"%s/%s",serverDir,dirp->d_name);
				if(ACE_OS::lstat(buff,&st) == -1)
					continue;

				ACE_OS::memset(buff,0,MAX_CMD_SIZE);
				if(!S_ISDIR(st.st_mode))
				{
					long _totalsize = st.st_size;
					long _size = _totalsize / (long)1024.0 / (long)1024.0;
					if (_size > (long)1.0)
						ACE_OS::sprintf(tmp,"%dMB", _size);
					else
						ACE_OS::sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);			

					struct tm* t = ACE_OS::localtime(&st.st_ctime);
					ACE_OS::sprintf(time,"%4d-%02d-%02d- %02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min); 
					ACE_OS::sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", dirp->d_name,tmp,time);
					dirlist += buff;		
				}
			}
#endif
			ACE_OS::closedir(root);
			dirlist += "</FList>";
		}

		delete[] serverDir;
		delete[] mainDir;

		if(dirlist != "")
		{
			dirlist = "<DFList>" + dirlist + "<DFList>";
			this->FEvent = new CMDMsg_Recv_Send_Event_Handler(this->clientCMDFrame.type,new ACE_Reactor(),dirlist.c_str());
			return this->FEvent->Open(peer_sock_) == 0;
		}	
	}catch(const char* error)
	{
		ACE_DEBUG((LM_DEBUG,L"[FileListPat]catch Error:%s[IP:%s]\n",error,this->client_Addr_.get_host_addr())); 
	}	
	return false;
}


time_t ConvToDate(char* date)
{
	//char* date = "20121206141817";
	if(strlen(date) != 14)
		return time_t();

	try
	{
		char yyyy[10];
		char MM[10];
		char dd[10];
		char hh[10];
		char mm[10];
		char ss[10];
		memcpy(yyyy,date,4);
		yyyy[4] = '\0';
		memcpy(MM,&date[4],2);
		MM[2] = '\0';
		memcpy(dd,&date[6],2);
		dd[2] = '\0';
		memcpy(hh,&date[8],2);
		hh[2] = '\0';
		memcpy(mm,&date[10],2);
		mm[2] = '\0';
		memcpy(ss,&date[12],2);	
		ss[2] = '\0';

		struct tm t;
		t.tm_year = atol(yyyy)-1900;
		t.tm_mon = atol(MM)-1;
		t.tm_mday = atol(dd);	

		t.tm_hour = atol(hh);
		t.tm_min = atol(mm);
		t.tm_sec = atol(ss);

		//char strTime[80];
		//strftime(strTime, sizeof(strTime), "%Y/%m/%d %X",&t);
		//printf("%s\n",strTime);  
		return mktime(&t);
	}
	catch (const char* error)
	{
		ACE_DEBUG((LM_DEBUG,L"[ConvToDate]catch Error:%s\n",error)); 
	}
	return time_t();	
}

/*XML格式
</ DFList >
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

//通过时间范围获取录相文件列表
bool CMD_Event_Handler::recvFileListByDateCMD()
{
	try
	{
		this->clientCMDFrame.CMD[this->clientCMDFrame.length] = '\0';
		ACE_DEBUG((LM_DEBUG,L"[FileListPathByDate]%s[IP:%s]\n",this->clientCMDFrame.CMD,this->client_Addr_.get_host_addr())); 

		char* path = NULL;
		char* beginDate = NULL;
		char* endDate = NULL;

		char __buff[MAX_CMD_SIZE];
		ACE_OS::memset(__buff,0,MAX_CMD_SIZE);
		ACE_OS::strcpy(__buff,this->clientCMDFrame.CMD);
		char* outer_ptr = NULL;
		char* spliter = "?";	
		char* token = ACE_OS::strtok_r(__buff,spliter,&outer_ptr);
		while(token != NULL)		
		{
			if(!path)
				path = token;
			else if(!beginDate)
				beginDate = token;
			else if(!endDate)
				endDate = token;
			token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
		}	
		if(!path || !beginDate || !endDate)
			return false;
		ACE_DEBUG((LM_DEBUG,L"[FileListPathByDate][path:%s][beginDate:%s][endDate:%s]\n",path,beginDate,endDate)); 

		char* serverDir = DirHelper::GetServerLocalDirPath(path);	
#ifdef ACE_WIN32
		if(ACE_OS::strstr(serverDir,"\\..") != NULL)
		{
			delete serverDir;
			return false;
		}		
#else
		if(ACE_OS::strstr(serverDir,"/..") != NULL)
		{
			delete serverDir;
			return false;
		}		
#endif	

		ACE_DIR *root = ACE_OS::opendir(serverDir);	
		if(NULL == root)
			return false;

		char* mainDir = DirHelper::GetServerLocalDirPath("");
		char buff[MAX_CMD_SIZE];
		ACE_OS::memset(buff,0,MAX_CMD_SIZE);



		time_t beginTime = ConvToDate(beginDate);
		time_t endTime = ConvToDate(endDate);

		if(!beginTime || !endTime)
			return false;

		char* EngineerID = NULL;
		char* FileDate = NULL;
		string dirlist = "";
		spliter = "-";
		char tmp[20];
		if(root)
		{
			dirlist += "<FList>";
#ifdef ACE_WIN32
			while (ACE_OS::readdir(root))
			{
				EngineerID = NULL;
				FileDate = NULL;
				if (!(root->fdata_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//win32的
				{
					//解析时间参数;
					ACE_OS::memset(__buff,0,MAX_CMD_SIZE);
					ACE_OS::strcpy(__buff,root->dirent_->d_name);
					char* token = ACE_OS::strtok_r(__buff,spliter,&outer_ptr);
					while(token != NULL)		
					{
						if(!EngineerID)
							EngineerID = token;
						else if(!FileDate)
							FileDate = token;
						token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
					}
					if(!EngineerID || !FileDate)
						continue;

					//19-20121128111451  [工程师ID-日期]
					time_t fileTime = ConvToDate(FileDate);
					if(!fileTime)
						continue;

					if(ACE_OS::difftime(fileTime,beginTime) >= 0.0 && ACE_OS::difftime(fileTime,endTime) <= 0)			
					{
						long _totalsize = (long)root->fdata_.nFileSizeHigh * ((long)MAXDWORD +(long)1) + (long)root->fdata_.nFileSizeLow;
						long _size = _totalsize / (long)1024.0 / (long)1024.0;						
						if (_size > (long)1.0)
							ACE_OS::sprintf(tmp,"%dMB", _size);
						else
							ACE_OS::sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);

						ACE_OS::sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", root->dirent_->d_name,tmp,"yyy-MM-dd hh:mm");
						dirlist += buff;
					}
				}
			}		
#else
			ACE_DIRENT*   dirp;   
			char tmp[20];
			char time[30];
			while((dirp = ACE_OS::readdir(root))!=NULL)   
			{
				struct stat st;
				ACE_OS::sprintf(buff,"%s/%s",serverDir,dirp->d_name);
				if(ACE_OS::lstat(buff,&st) == -1)
					continue;

				ACE_OS::memset(buff,0,MAX_CMD_SIZE);
				if(!S_ISDIR(st.st_mode))
				{
					//解析时间参数;
					ACE_OS::memset(__buff,0,MAX_CMD_SIZE);
					ACE_OS::strcpy(__buff,dirp->d_name);
					char* token = ACE_OS::strtok_r(__buff,spliter,&outer_ptr);
					while(token != NULL)		
					{
						if(!EngineerID)
							EngineerID = token;
						else if(!FileDate)
							FileDate = token;
						token = ACE_OS::strtok_r(NULL,spliter,&outer_ptr);
					}
					if(!EngineerID || !FileDate)
						continue;

					//19-20121128111451  [工程师ID-日期]
					time_t fileTime = ConvToDate(FileDate);
					if(!fileTime)
						continue;

					if(ACE_OS::difftime(fileTime,beginTime) >= 0.0 && ACE_OS::difftime(fileTime,endTime) <= 0)		
					{
						long _totalsize = st.st_size;
						long _size = _totalsize / (long)1024.0 / (long)1024.0;
						if (_size > (long)1.0)
							ACE_OS::sprintf(tmp,"%dMB", _size);
						else
							ACE_OS::sprintf(tmp,"%dKB", _totalsize / (long)1024 == 0 ? 1 : _totalsize / (long)1024);			

						struct tm* t = ACE_OS::localtime(&st.st_ctime);
						ACE_OS::sprintf(time,"%4d-%02d-%02d- %02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min); 
						ACE_OS::sprintf(buff,"<Fitm><fp>%s</fp><size>%s</size><ct>%s</ct></Fitm>\n", dirp->d_name,tmp,time);
						dirlist += buff;
					}
				}
			}
#endif
			ACE_OS::closedir(root);
			dirlist += "</FList>";
		}

		delete[] serverDir;
		delete[] mainDir;

		if(dirlist != "")
		{
			dirlist = "<DFList>" + dirlist + "<DFList>";
			this->FEvent = new CMDMsg_Recv_Send_Event_Handler(this->clientCMDFrame.type,new ACE_Reactor(),dirlist.c_str());
			return this->FEvent->Open(peer_sock_) == 0;
		}	
	}catch(const char* error)
	{
		ACE_DEBUG((LM_DEBUG,L"[FileListPatByDate]catch Error:%s[IP:%s]\n",error,this->client_Addr_.get_host_addr())); 
	}	
	return false;
}
//删除文件
bool CMD_Event_Handler::recvFileDelCMD()
{	
	this->clientCMDFrame.CMD[this->clientCMDFrame.length] = '\0';
	char* serverFileName = DirHelper::GetServerLocalDirPath(this->clientCMDFrame.CMD);

	bool ret = LocalFile::fileExist(serverFileName);
	if(ret)
	{
		ret = LocalFile::deleteFile(serverFileName);
		ACE_DEBUG((LM_DEBUG,L"[CMD]Del file %s(%s)[IP:%s]\n",ret ? "success":"failed",
															this->clientCMDFrame.CMD,
															this->client_Addr_.get_host_addr())); 		
	}else
	{
		ACE_DEBUG((LM_DEBUG,L"[CMD]file notExist%s(%s)[IP:%s]\n",ret ? "success":"failed",
															this->clientCMDFrame.CMD,
															this->client_Addr_.get_host_addr())); 	
	}
	return ret;
}
//目录创建
bool CMD_Event_Handler::recvDirCreateCMD()
{
	this->clientCMDFrame.CMD[this->clientCMDFrame.length] = '\0';	
	bool ret = DirHelper::CreateServerDirPath(this->clientCMDFrame.CMD);

	ACE_DEBUG((LM_DEBUG,L"[CMD]Dir create %s(%s)[IP:%s]\n",ret ? "success":"failed",
															this->clientCMDFrame.CMD,
															this->client_Addr_.get_host_addr())); 
	return ret;
}
//删除目录
bool CMD_Event_Handler::recvDirDelCMD()
{
	this->clientCMDFrame.CMD[this->clientCMDFrame.length] = '\0';
	char* serverDir = DirHelper::GetServerLocalDirPath(this->clientCMDFrame.CMD);

	bool ret = (ACE_OS::rmdir(serverDir) == 0);
	ACE_DEBUG((LM_DEBUG,L"[CMD]Dir delete %s(%s)[IP:%s]\n",ret ? "success":"failed",
													this->clientCMDFrame.CMD,
													this->client_Addr_.get_host_addr())); 
	delete[] serverDir;
	return ret;	
}
//当前连接数
bool CMD_Event_Handler::recvGetConnectedCountCMD()
{
	ACE_OS::sprintf(this->serverCMDRetFrame.CMD,"%d",Client_List::Instens()->GetCount());
	this->serverCMDRetFrame.length = ACE_OS::strlen(this->serverCMDRetFrame.CMD);
	return true;
}

