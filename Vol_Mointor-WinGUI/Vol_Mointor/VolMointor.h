// VolMointor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CVolMointorApp:
// �йش����ʵ�֣������ VolMointor.cpp
//

class CVolMointorApp : public CWinApp
{
public:
	CVolMointorApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CVolMointorApp theApp;

#define BAT_NUM 12
