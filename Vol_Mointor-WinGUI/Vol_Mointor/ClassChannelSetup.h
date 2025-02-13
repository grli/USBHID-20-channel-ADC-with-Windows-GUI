#pragma once
#include "afxwin.h"


// CClassChannelSetup dialog

class CClassChannelSetup : public CDialog
{
	DECLARE_DYNAMIC(CClassChannelSetup)

public:
	CClassChannelSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClassChannelSetup();

// Dialog Data
	enum { IDD = IDD_CHANNEL_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
public:
	CEdit m_edit_ch[8];
public:
	CComboBox m_ComboChannel_Pos[BAT_NUM];
	//CComboBox m_ComboChannel_Neg[8];
};
