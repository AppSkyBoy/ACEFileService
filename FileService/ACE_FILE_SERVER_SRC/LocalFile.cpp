#include "LocalFile.h"

LocalFile::LocalFile(const char *fileName)
{
	if(fileName != NULL)
	{
		this->fileAddr.set(fileName);
		this->open(fileName);
	}
}

LocalFile::~LocalFile()
{
	this->close();
}

//�ж��ļ��Ƿ����
bool LocalFile::fileExist(const char *fileName)
{
	if(NULL == fileName)
		return false;

	ACE_FILE_Connector conn;
	ACE_FILE_IO dfile;
	bool ret = conn.connect(dfile, ACE_FILE_Addr(fileName),
							0, ACE_Addr::sap_any, 0, O_RDWR,
							ACE_DEFAULT_FILE_PERMS
							) == 0;
	dfile.close();
	return ret;
}

//�����ļ�
bool LocalFile::createFile(const char *fileName)
{
	if(NULL == fileName)
		return false;

	ACE_FILE_Connector conn;
	ACE_FILE_IO dfile;
	return conn.connect(dfile, ACE_FILE_Addr(fileName)) == 0;
}

//ɾ���ļ�
bool LocalFile::deleteFile(const char *fileName)
{
	if(NULL == fileName)
		return false;

	bool ret = false;

	ACE_FILE_Connector conn;
	ACE_FILE_IO dfile;
	if(conn.connect(dfile, ACE_FILE_Addr(fileName)) == -1)
		ret = false;
	else
		ret = dfile.remove() == 0;
	return ret;
}

//���ļ�
bool LocalFile::open(const char *fileName)
{
	if(NULL == fileName)
		return false;

	ACE_FILE_Connector conn;
	this->fileAddr.set(fileName);
	this->cacheFileName(fileName);
	return conn.connect(this->fileIO, this->fileAddr) == 0;
}
//�ر��ļ�
void LocalFile::close()
{
	this->fileIO.close();
}

//���ļ����������
int LocalFile::write(char *data, int length)
{
	return this->fileIO.send_n(data, length);
}

//���ļ��ж�ȡ����
int LocalFile::read(char *data, int length)
{
	return this->fileIO.recv(data, length);
}

//���治��·���Ĵ��ļ���
void LocalFile::cacheFileName(const char *fileName)
{
	int len = strlen(fileName);
	this->pureFileName = "";
	for(int i = len - 1; i >= 0; i--)
	{
		if('\\' == fileName[i])
			break;
		else
			this->pureFileName.insert(0, string(1,fileName[i]));
	}
}

long LocalFile::getFileSize() 
{
	ACE_FILE_Info fileInfo;
	if(this->fileIO.get_info(fileInfo) != -1)
		return fileInfo.size_;
	else 
		return 0;
}

