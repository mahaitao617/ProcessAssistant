
// ProcessAssistant.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CProcessAssistantApp: 
// �йش����ʵ�֣������ ProcessAssistant.cpp
//

class CProcessAssistantApp : public CWinApp
{
public:
	CProcessAssistantApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CProcessAssistantApp theApp;