/******************************************************************************
ģ������:  ��������
ģ�鹦��:  
������:    ��
��������:  2012/10
��������:
��ע:      
******************************************************************************/
#define UNICODE

#ifndef COMMON_H
#define COMMON_H

//�ļ����Ͷ����ݰ�����󻺴��С
const int MAX_BUFFER_SIZE = 8*1024;

//�ļ����ն����ݰ��������Ϣ��С
const int MAX_INFO_SIZE = 256;

//CMD���ݰ��������Ϣ��С(�Լ�char����Buff��С)
const int MAX_CMD_SIZE = 512;

//CMD��Ϣ�ı����ݰ������Ϣ��С
const int MAX_CMD_MSG_SIZE = 6*1024;

//�ļ����Ͱ���������
enum FileSendFrameType
{
	FILE_SEND_NAME,					//�ļ�����
	FILE_SEND_SIZE,					//�ļ���С
	FILE_SEND_DATA,					//�ļ�����	
	FILE_SEND_END,					//�ļ���β
	FILE_SEND_ERROR,				//�ļ�����
	FILE_SEND_OVERWRITE,			//�����ļ�
	FILE_SEND_DISCONNECT,			//�Ͽ�����
	FILE_SEND_FILE_NOTEXIST,		//�ͻ�������ָ���ļ�ʱ�������˷����ļ�������(���ھ�ֱ�Ӱ������ķ���������)
	FILE_SEND_INVALID,				//��������Чλ
};

//�ļ����ն˰���������
enum FileRecvType
{	
	FILE_RECV_CLIENT_GET_FILENAME,	//��ȡ�ļ���(client���ϴ��ļ���·����server����Ƿ�����ٽ��з���)
	FILE_RECV_GET_FILENAME,			//��ȡ�ļ���
	FILE_RECV_GET_FILESIZE,			//��ȡ�ļ���С
	FILE_RECV_GET_FILEDATA,			//��ȡ�ļ�����(������֤,���Ͷ��յ�����������ļ�����ֱ������)
	FILE_RECV_FILE_EXIST,			//���Ͷ�\�ļ��Ѿ�����
	FILE_RECV_DIR_NOTEXIST,			//���ն�Ŀ¼·��������
	FILE_RECV_FINISHED,				//�ļ����䲢����ɹ�
	FILE_RECV_INVALID,				//��������Чλ����Ҫ�����򻯳���Ŀ����߼�
	FILE_RECV_ERROR,				//����,��ʾ����ʧ��,֪ͨClient���ٷ����ļ�����
};

//CMD��Ϣ�ı����ݰ�
enum CMDMsgFrameType
{
	RECV_MSG_LENTH,			//��Ϣ�ı�����
	RECV_MSG_DATA,			//��Ϣ�ı�����
	RECV_MSG_END,			//��Ϣ�������

	SEND_MSG_LENTH,
	SEND_MSG_DATA,
	SEND_MSG_END,

	MSG_INVALID,		   //��������Чλ
};

//CMD����
enum CilentCMDFrameType
{
	CLIENT_FILE_SEND,		//�ļ�����
	CLIENT_FILE_RECV,		//�ļ�����
	CLIENT_FILE_GET_LIST,	//��ȡ�ļ��б�
	CLIENT_FILE_GET_LIST_BYDATE,//ͨ��ʱ���ȡ�ļ��б�
	CLIENT_FILE_DEL,		//�ļ�ɾ��
	CLIENT_DIR_CREATE,		//����Ŀ¼
	CLIENT_GETCONNECTEDCOUNT,	//��ȡ������������
	CREATE_DIR_DEL,			//ɾ��Ŀ¼

	CLIENT_DISCONNECT,		//�Ͽ�����
	SERVER_INVALID,			//��������Чλ
	SERVER_OK,				//��������׼����(���յ�����Ϣ����ֵ�����ڶ�Ӧ�İ���)
	SERVER_ERROR,			//����������		
};
//-------------------------------------------------
//�ļ��������ݰ�
struct FileSendFrame
{
	FileSendFrameType type;			//����
	int length;						//��ʵ�ʴ�С
	char buffer[MAX_BUFFER_SIZE];	//����
};
//-------------------------------------------------
//�ļ����ն����ݰ�
struct FileRecvFrame
{
	FileRecvType type;			
	int length;					
	char info[MAX_INFO_SIZE];	
};
//-------------------------------------------------
//CMD���ݰ�(client<--->server)
struct CilentCMDFrame
{
	CilentCMDFrameType type;		
	int length;						
	char CMD[MAX_CMD_SIZE];			
};
//-------------------------------------------------
//CMD���ݰ��������ݰ�
struct CMDMsgFrame
{
	CMDMsgFrameType type;		
	int length;						
	char CMDMsg[MAX_CMD_MSG_SIZE];	
};

#endif

