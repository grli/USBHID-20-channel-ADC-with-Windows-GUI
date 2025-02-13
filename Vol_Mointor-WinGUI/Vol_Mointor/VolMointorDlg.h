// VolMointorDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "imgbutton.h"

#include <vector>
#include <dbt.h>
#include <afxmt.h>

// CVolMointorDlg 对话框
class CVolMointorDlg : public CDialog
{
// 构造
public:
	CVolMointorDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_VolMointor_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnMouseDiff_X( short zDelta );
	void OnMouseDiff_Y( short zDelta );
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedMaximize();
	afx_msg void OnBnClickedMinimize();
	afx_msg void OnBnClickedAppExit();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedBatcheck1();
	afx_msg void OnBnClickedBatcheck2();
	afx_msg void OnBnClickedBatcheck3();
	afx_msg void OnBnClickedBatcheck4();
	afx_msg void OnBnClickedBatcheck5();
	afx_msg void OnBnClickedBatcheck6();
	afx_msg void OnBnClickedBatcheck7();
	afx_msg void OnBnClickedBatcheck8();
	afx_msg void OnBnClickedBatcheck9();
	afx_msg void OnBnClickedBatcheck10();
	afx_msg void OnBnClickedBatcheck11();
	afx_msg void OnBnClickedBatcheck12();
	afx_msg void OnBnClickedOption();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);

public:
	CButton m_btnIdleFocus;
	CImgButton m_btnMinmize;
	CImgButton m_btnMaximize;
	CImgButton m_btnExit;
	CImgButton m_btnConnect;
	CImgButton m_btnRecord;
	CButton m_btnCheckBat1;
	CButton m_btnCheckBat2;
	CButton m_btnCheckBat3;
	CButton m_btnCheckBat4;
	CButton m_btnCheckBat5;
	CButton m_btnCheckBat6;
	CButton m_btnCheckBat7;
	CButton m_btnCheckBat8;
	CButton m_btnCheckBat9;
	CButton m_btnCheckBat10;
	CButton m_btnCheckBat11;
	CButton m_btnCheckBat12;
	CButton m_btnVolCur;
	CButton m_btnTimeCur;
	CImgButton m_btnOption;
	CImgButton m_btnExport;
	CImgButton m_btnImport;

	CComboBox m_comboDevice;
	void UpdateDeviceCombo();
	int SelectString(CComboBox& combo, CString selectString);
	BOOL Connect();
	void Disconnect();
	void DrawWave();
	void DrawWave1();
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedExport();
	afx_msg void OnBnClickedImport();
	CToolTipCtrl m_tool;
	afx_msg void OnBnClickedUpgrade();
	afx_msg void OnBnClickedRecord();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBnClickedVoltageCursor();
	afx_msg void OnBnClickedTimeCursor();
	CComboBox m_ComboSampleRate;
	afx_msg void OnCbnKillfocusSampleRate();
	void StartSampleTimer();
	afx_msg void OnEnKillfocusEditVol();
	CEdit m_edit_vol_str;
	afx_msg void OnBnClickedHelp1();
};

#define READ_TIMER 1
