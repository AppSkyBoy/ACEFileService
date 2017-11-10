/******************************************************************************
模块名称:  LocalFile
模块功能:  文件处理类
创建者:    夏
创建日期:  2012/10
更新日期:
备注:      处理文件的基本操作
*****************************************************************************/

#ifndef LOCAL_FILE_H
#define LOCAL_FILE_H

#include "ace/FILE_Addr.h"
#include "ace/FILE_IO.h"
#include "ace/FILE_Connector.h"

#include <string>
using namespace std;

class LocalFile
{
public:
	//若参数fileName不为空，则创建LocalFile并打开文件
	LocalFile(const char *fileName = NULL);	
	virtual ~LocalFile();

	//判断文件是否存在
	static bool fileExist(const char *fileName);	
	//创建文件
	static bool createFile(const char *fileName);
	//删除文件
	static bool deleteFile(const char *fileName);	
	//打开文件
	bool open(const char *fileName);
	//关闭文件
	void close();
	//往文件中添加数据
	int write(char *data, int length);
	//从文件中读取数据
	int read(char *data, int length);
	//当前文件大小;
	long getFileSize(); 
	//获取文件的路径名
	const char* getPathName() const { return this->fileAddr.get_path_name(); }
	//获取文件名
	const char* getFileName() const { return this->pureFileName.c_str();}

private:
	//缓存不含路径的纯文件名
	void cacheFileName(const char *fileName);

private:
	//文件地址对象
	ACE_FILE_Addr fileAddr;	

	//文件输入输出对象
	ACE_FILE_IO fileIO;

	//不含路径的文件名
	string pureFileName;	
};

#endif

