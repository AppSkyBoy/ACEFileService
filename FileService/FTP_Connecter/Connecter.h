// FTP_Connecter.h : FTP_Connecter DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFTP_ConnecterApp
// �йش���ʵ�ֵ���Ϣ������� FTP_Connecter.cpp
//

class CFTP_ConnecterApp : public CWinApp
{
public:
	CFTP_ConnecterApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
