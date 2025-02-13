// VolMointorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VolMointor.h"
#include "VolMointorDlg.h"
#include "math.h"
#include "image.c"
#include "SLABHIDDevice.h"

#pragma comment (lib, "SLABHIDDevice.lib")

#define VID								0x10C4
#define PID								0xEAC9

// HID Report IDs
#define ID_IN_CONTROL					0xFE
#define ID_OUT_CONTROL					0xFD
#define ID_IN_DATA						0x01
#define ID_OUT_DATA						0x02

// String Lengths
#define MAX_VID_LENGTH					5
#define MAX_PID_LENGTH					5
#define MAX_PATH_LENGTH					MAX_PATH
#define MAX_SERIAL_STRING_LENGTH		256
#define MAX_MANUFACTURER_STRING_LENGTH	256
#define MAX_PRODUCT_STRING_LENGTH		256
#define MAX_INDEXED_STRING_LENGTH		256
#define MAX_STRING_LENGTH				260

#define SCREEN_REFRESH_TIME 20

#define DIFF_Y 80  // button region

#define DIFF_GRID_X  50  // left remain
#define DIFF_GRID_Y  20  // bottom remain

#define GRID_LINE_COLOR  RGB(0x4f,0x4f,0x40)
#define GRID_TEXT_COLOR  RGB(0x4f,0x4f,0x40)

#define GRID_VOLTAGE  0.01

#define RATIO  4095.0   // 12 bit adc

int ref__vol = 3300;
#define REF_VOLTAGE (ref__vol/1000./8.)

#define TOTAL_SAMPLE_NUM (50*1024*1024)

CString ChannelName[ BAT_NUM ];
int sample_delay = 0;

//int ChannelPos[ BAT_NUM ] = { 20, 21, 22, 23, 8, 9, 10, 11, 12, 13, 14, 15 };

BOOL blNeedRefresh = FALSE;
BOOL blNeedResign = TRUE;
double g_SamplePerPoint = 12.61;

int horizontal_grid_num = 10;
int vertical_grid_num = 10;

BOOL blIncreaseDisplay = FALSE;
BOOL blDispVSel = TRUE, blDispHSel = TRUE;
BOOL blSelect_pos = FALSE;
double Select_Pos1 = 0, Select_Pos2 = 0;
double Select_Vol1 = 0, Select_Vol2 = 0;
CString Select_str_Pos1 ="", Select_str_Pos2 = "";

#define SELECT_POS1_COLOR RGB(0x80,0x80,0x00)
#define SELECT_POS2_COLOR RGB(0x00,0x7f,0x00)

typedef struct
{
	char Signature[10];
	int version;
	int ChannelNum;
	char Enable[ BAT_NUM ];
	char ChannelName[ BAT_NUM ][20];
	DWORD GetDataPos;
}CommonHeader_s;

volatile struct
{
	CommonHeader_s info;
	short GetData[ BAT_NUM ][ TOTAL_SAMPLE_NUM ];
	DWORD TimeBase[ TOTAL_SAMPLE_NUM ];
}GroupData;

COLORREF color[ BAT_NUM + 1 ] = {
	RGB(0xff,0xff,0x00),
	RGB(0x00,0xff,0x00),
	RGB(0xff,0x00,0x00),
	RGB(0x00,0xff,0xff),
	RGB(0xff,0x80,0x00),
	RGB(0xff,0x00,0xff),
	RGB(0x00,0x80,0xff),
	RGB(0xe0,0xe0,0xe0),

	RGB(0x7f,0x7f,0x00),
	RGB(0x00,0x7f,0x00),
	RGB(0x7f,0x00,0x00),
	RGB(0x00,0x7f,0x7f),

	RGB(0xff,0xff,0xff),
};

CPoint Old_point(0,0);
CPoint New_point(0,0);
CPoint Diff_point(0,0);
CPoint Diff_step(0,0);

int Pan_X = 1, Pan_Y = 40;
int Pan_X_strange = 0;

#define SCALE_NUM  24
int pan[ SCALE_NUM ] = { 1, 2, 5, 10, 20, 30, 60, 2*60, 5*60, 10*60, 15*60, 20*60, 30*60, 45*60, 60*60, 120*60, 180*60, 240*60, 300*60, 360*60, 420*60, 480*60, 540*60, 600*60 };

#define SCALE_TIME_NUM 7
int divtime[ SCALE_TIME_NUM ] = {1000, 500, 200, 100, 50, 20, 10}; //ms

BOOL blConnected = FALSE;

int GridStep_X, GridStep_Y;
int get_speed = 0;

DWORD start_time = 0, cur_time = 0;

HANDLE Thread = (HANDLE)-1;
DWORD dwThreadId;
DWORD WINAPI ThreadFunc(HANDLE Thread);

BOOL blRealDisplay = TRUE;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char g_ExecFileDir[ _MAX_PATH + _MAX_FNAME + _MAX_EXT ];
BOOL m_CheckBat[ BAT_NUM ] = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE };

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVolMointorDlg 对话框




CVolMointorDlg::CVolMointorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVolMointorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVolMointorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MINIMIZE, m_btnMinmize);
	DDX_Control(pDX, IDC_MAXIMIZE, m_btnMaximize);
	DDX_Control(pDX, ID_APP_EXIT, m_btnExit);
	DDX_Control(pDX, IDC_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_RECORD,  m_btnRecord);
	DDX_Control(pDX, IDC_BATCHECK1, m_btnCheckBat1);
	DDX_Control(pDX, IDC_BATCHECK2, m_btnCheckBat2);
	DDX_Control(pDX, IDC_BATCHECK3, m_btnCheckBat3);
	DDX_Control(pDX, IDC_BATCHECK4, m_btnCheckBat4);
	DDX_Control(pDX, IDC_BATCHECK5, m_btnCheckBat5);
	DDX_Control(pDX, IDC_BATCHECK6, m_btnCheckBat6);
	DDX_Control(pDX, IDC_BATCHECK7, m_btnCheckBat7);
	DDX_Control(pDX, IDC_BATCHECK8, m_btnCheckBat8);
	DDX_Control(pDX, IDC_BATCHECK9, m_btnCheckBat9);
	DDX_Control(pDX, IDC_BATCHECK10, m_btnCheckBat10);
	DDX_Control(pDX, IDC_BATCHECK11, m_btnCheckBat11);
	DDX_Control(pDX, IDC_BATCHECK12, m_btnCheckBat12);
	DDX_Control(pDX, IDC_DEVICELIST, m_comboDevice);
	DDX_Control(pDX, IDC_IDLE, m_btnIdleFocus);
	DDX_Control(pDX, IDC_OPTION, m_btnOption);
	DDX_Control(pDX, IDC_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_IMPORT, m_btnImport);
	DDX_Control(pDX, IDC_VOLTAGE_CURSOR, m_btnVolCur);
	DDX_Control(pDX, IDC_TIME_CURSOR, m_btnTimeCur);
	DDX_Control(pDX, IDC_SAMPLE_RATE, m_ComboSampleRate);
	DDX_Control(pDX, IDC_EDIT_VOL, m_edit_vol_str);
}

BEGIN_MESSAGE_MAP(CVolMointorDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_MAXIMIZE, &CVolMointorDlg::OnBnClickedMaximize)
	ON_BN_CLICKED(IDC_MINIMIZE, &CVolMointorDlg::OnBnClickedMinimize)
	ON_BN_CLICKED(ID_APP_EXIT, &CVolMointorDlg::OnBnClickedAppExit)
	ON_BN_CLICKED(IDC_CONNECT, &CVolMointorDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BATCHECK1, &CVolMointorDlg::OnBnClickedBatcheck1)
	ON_BN_CLICKED(IDC_BATCHECK2, &CVolMointorDlg::OnBnClickedBatcheck2)
	ON_BN_CLICKED(IDC_BATCHECK3, &CVolMointorDlg::OnBnClickedBatcheck3)
	ON_BN_CLICKED(IDC_BATCHECK4, &CVolMointorDlg::OnBnClickedBatcheck4)
	ON_BN_CLICKED(IDC_BATCHECK5, &CVolMointorDlg::OnBnClickedBatcheck5)
	ON_BN_CLICKED(IDC_BATCHECK6, &CVolMointorDlg::OnBnClickedBatcheck6)
	ON_BN_CLICKED(IDC_BATCHECK7, &CVolMointorDlg::OnBnClickedBatcheck7)
	ON_BN_CLICKED(IDC_BATCHECK8, &CVolMointorDlg::OnBnClickedBatcheck8)
	ON_BN_CLICKED(IDC_BATCHECK9, &CVolMointorDlg::OnBnClickedBatcheck9)
	ON_BN_CLICKED(IDC_BATCHECK10, &CVolMointorDlg::OnBnClickedBatcheck10)
	ON_BN_CLICKED(IDC_BATCHECK11, &CVolMointorDlg::OnBnClickedBatcheck11)
	ON_BN_CLICKED(IDC_BATCHECK12, &CVolMointorDlg::OnBnClickedBatcheck12)
	ON_BN_CLICKED(IDC_VOLTAGE_CURSOR, &CVolMointorDlg::OnBnClickedVoltageCursor)
	ON_BN_CLICKED(IDC_TIME_CURSOR, &CVolMointorDlg::OnBnClickedTimeCursor)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_OPTION, &CVolMointorDlg::OnBnClickedOption)
//	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_EXPORT, &CVolMointorDlg::OnBnClickedExport)
	ON_BN_CLICKED(IDC_IMPORT, &CVolMointorDlg::OnBnClickedImport)
	ON_BN_CLICKED(IDC_UPGRADE, &CVolMointorDlg::OnBnClickedUpgrade)
	ON_BN_CLICKED(IDC_RECORD, &CVolMointorDlg::OnBnClickedRecord)
	ON_WM_HELPINFO()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_CBN_KILLFOCUS(IDC_SAMPLE_RATE, &CVolMointorDlg::OnCbnKillfocusSampleRate)
	ON_EN_KILLFOCUS(IDC_EDIT_VOL, &CVolMointorDlg::OnEnKillfocusEditVol)
	ON_BN_CLICKED(IDC_HELP1, &CVolMointorDlg::OnBnClickedHelp1)
END_MESSAGE_MAP()


// CVolMointorDlg 消息处理程序

BOOL CVolMointorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_edit_vol_str.SetWindowText("3300");

	// TODO: 在此添加额外的初始化代码
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	GetModuleFileName( NULL, g_ExecFileDir, _MAX_PATH );
	_splitpath( g_ExecFileDir, drive, dir, fname, ext );
    _makepath( g_ExecFileDir, drive, dir, "", "" );

	m_btnMinmize.InitImage( img_min_norm, img_min_norm, img_min_focu, img_min_focu );
	m_btnMaximize.InitImage( img_max_norm, img_max_norm, img_max_focu, img_max_focu );
	m_btnExit.InitImage( img_exit_nor, img_exit_nor, img_exit_foc, img_exit_foc );
	m_btnConnect.InitImage( img_connect1, img_connect1, img_connect1, img_connect1_a );
	m_btnRecord.InitImage( img_record2, img_record2, img_record2, img_record2_a );
	m_btnOption.InitImage( img_option1, img_option1, img_option1, img_option1_a );

	m_btnExport.InitImage( img_export, img_export, img_export, img_export_a );
	m_btnImport.InitImage( img_import, img_import, img_import, img_import_a );

	m_btnRecord.SetWindowText( "" );
	m_btnConnect.SetWindowText( "" );
	m_btnOption.SetWindowText( "" );
	m_btnExport.SetWindowText( "" );
	m_btnImport.SetWindowText( "" );
	//m_btnRecord.SetWindowText( "暂停" );
	//m_btnConnect.SetWindowText( "连接" );
	//m_btnOption.SetWindowText( "设置" );
	//m_btnExport.SetWindowText( "保存" );
	//m_btnImport.SetWindowText( "导入" );

	m_btnCheckBat1.SetWindowText( "CH0" );
	m_btnCheckBat2.SetWindowText( "CH1" );
	m_btnCheckBat3.SetWindowText( "CH2" );
	m_btnCheckBat4.SetWindowText( "CH3" );
	m_btnCheckBat5.SetWindowText( "CH4" );
	m_btnCheckBat6.SetWindowText( "CH5" );
	m_btnCheckBat7.SetWindowText( "CH6" );
	m_btnCheckBat8.SetWindowText( "CH7" );
	m_btnCheckBat9.SetWindowText( "CH8" );
	m_btnCheckBat10.SetWindowText( "CH9" );
	m_btnCheckBat11.SetWindowText( "CH10" );
	m_btnCheckBat12.SetWindowText( "CH11" );

	// Populate the combo box with USB HID devices
	UpdateDeviceCombo();
	// Register for surprise removal notification
	//RegisterNotification();

	m_ComboSampleRate.ResetContent();
	m_ComboSampleRate.AddString("1Hz");
	m_ComboSampleRate.AddString("2Hz");
	m_ComboSampleRate.AddString("5Hz");
	m_ComboSampleRate.AddString("10Hz");
	m_ComboSampleRate.AddString("20Hz");
	m_ComboSampleRate.AddString("25Hz");
	m_ComboSampleRate.AddString("50Hz");
	m_ComboSampleRate.AddString("100Hz");
	m_ComboSampleRate.AddString("500Hz");
	m_ComboSampleRate.AddString("1000Hz");
	m_ComboSampleRate.SetCurSel(m_ComboSampleRate.GetCount()-1);

	m_tool.Create(this);
    m_tool.AddTool(this,"windows");
    m_tool.SetDelayTime(100);
    m_tool.Activate(true);

	for( int bat = 0; bat < BAT_NUM; bat++ )
	{
#if 0
		for( int i = 0; i < TOTAL_SAMPLE_NUM; i++ )
		{      
			GroupData.GetData[bat][i] = (unsigned short)(500 + bat*100+i/100*sin( i/(90.*(1+bat))*3.14 ));
			GroupData.TimeBase[ i ] = i;
		}
		GroupData.GetDataPos = 10000;
#else
		for( int i = 0; i < 15000; i++ ) // TOTAL_SAMPLE_NUM
		{
			GroupData.GetData[bat][i] = (unsigned short)(300 + bat*300+i/100*sin( i/(90.*(1+bat))*3.14 ))*8;
			GroupData.TimeBase[ i ] = i;
		}
		GroupData.info.GetDataPos = 15000;
#endif
	}

	SetTimer( READ_TIMER, SCREEN_REFRESH_TIME, NULL );
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL CVolMointorDlg::PreTranslateMessage(MSG* pMsg)
{
    if ( (pMsg->message == WM_KEYDOWN))
    {
        if( pMsg->wParam == VK_RETURN
            || pMsg->wParam == VK_ESCAPE )
        {
            return TRUE;
        }
		else if( pMsg->wParam == VK_LEFT )
		{
			if(::GetKeyState(VK_CONTROL) < 0 )
			{
				OnMouseDiff_X( 1 );
			}
			else
			{
				Diff_step.x += Pan_X;
				if( Diff_step.x >= 0 )
				{
					Diff_step.x = 0;
				}
			}
			blNeedRefresh = TRUE;
			return TRUE;
		}
		else if( pMsg->wParam == VK_RIGHT )
		{
			if(::GetKeyState(VK_CONTROL) < 0 )
			{
				OnMouseDiff_X( -1 );
			}
			else
			{
				Diff_step.x -= Pan_X;
			}
			blNeedRefresh = TRUE;
			return TRUE;
		}
		else if( pMsg->wParam == VK_UP )
		{
			if(::GetKeyState(VK_CONTROL) < 0 )
			{
				OnMouseDiff_Y( 1 );
			}
			else
			{
				Diff_step.y ++;
			}
			blNeedRefresh = TRUE;
			return TRUE;
		}
		else if( pMsg->wParam == VK_DOWN )
		{
			if(::GetKeyState(VK_CONTROL) < 0 )
			{
				OnMouseDiff_Y( -1 );
			}
			else
			{
				Diff_step.y--;
				//if( Diff_step.y <= 0 )
				//{
				//	Diff_step.y = 0;
				//}
			}
			blNeedRefresh = TRUE;
			return TRUE;
		}
		else if( pMsg->wParam == 'H' )
		{
			OnChar( 'H', 0, 0 );
			return TRUE;
		}
		else if( pMsg->wParam == 'h' )
		{
			OnChar( 'h', 0, 0 );
			return TRUE;
		}
		else if( pMsg->wParam == 'v' )
		{
			OnChar( 'v', 0, 0 );
			return TRUE;
		}
		else if( pMsg->wParam == 'V' )
		{
			OnChar( 'V', 0, 0 );
			return TRUE;
		}
    }

	m_tool.RelayEvent(pMsg);
    return CDialog::PreTranslateMessage(pMsg);
}

int CVolMointorDlg::SelectString(CComboBox& combo, CString selectString)
{
	int sel = combo.FindStringExact(-1, selectString);
	int newSel;

	if (sel != CB_ERR)
	{
		newSel = combo.SetCurSel(sel);
	}

	newSel = combo.GetCurSel();

	return sel;
}

// Populate the combo box with USB HID devices
void CVolMointorDlg::UpdateDeviceCombo()
{
	static int oldnum = 0;
	// Only update the list if the combo is enabled
	// and if the droplist is closed
	if (m_comboDevice.IsWindowEnabled() &&
		!m_comboDevice.GetDroppedState())
	{
		char deviceString[MAX_PATH_LENGTH];
		int devnum = HidDevice_GetNumHidDevices(VID, PID);
		m_comboDevice.ResetContent();

		for( int i = 0; i < devnum; i++ )
		{
			if( HidDevice_GetHidString(i, VID, PID, HID_PATH_STRING, deviceString, MAX_PATH_LENGTH) == HID_DEVICE_SUCCESS )
			{
				m_comboDevice.AddString(deviceString );
			}
		}

		if( m_comboDevice.GetCount() != 0 )
		{
			// If selText was not found in the combo
			// then select the first item
			m_comboDevice.SetCurSel(0);
		}
	}
}

HID_DEVICE	m_hid = NULL;
BYTE report[512], getreport[512];

BOOL CVolMointorDlg::Connect()
{
	int sel = m_comboDevice.GetCurSel();
	if (sel >= 0)
	{
		BYTE status = HidDevice_Open(&m_hid, 0, VID, PID, MAX_REPORT_REQUEST_XP);
		if( status != HID_DEVICE_SUCCESS )
		{
			AfxMessageBox( "Device open failed" );
			return false;
		}

		HidDevice_SetTimeouts( m_hid, 1000, 500 ); // get/set timeout
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void CVolMointorDlg::Disconnect()
{
	if( Thread != (HANDLE)-1 )
	{
		TerminateThread( Thread, 0 );
		Thread = (HANDLE)-1;
	}
	Sleep(100);
	m_comboDevice.EnableWindow(TRUE);
//	m_ComboSampleRate.EnableWindow(TRUE);
	m_btnConnect.EnableWindow(TRUE);
	if( m_hid != NULL )
		HidDevice_Close(m_hid);
	m_hid = NULL;
	UpdateDeviceCombo();
}

void CVolMointorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
#include "math.h"

void CVolMointorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DrawWave();
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CVolMointorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CVolMointorDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CDialog::OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	rect.bottom = 20;
	pDC->FillSolidRect(rect, RGB(0xff,0xff,0xff));

	GetClientRect(&rect);
	rect.top = 20;
	rect.bottom = DIFF_Y;
	pDC->FillSolidRect(rect, RGB(0xff,0xff,0xff));

	//GetClientRect(&rect);
	//rect.top = DIFF_Y - 6;
	//pDC->FillSolidRect(rect, RGB(0xf,0xf,0xf));
	return TRUE;
}

void CVolMointorDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	if( pRect->right - pRect->left < 750 )
	{
		pRect->right = pRect->left + 750;
	}

	if( pRect->bottom - pRect->top < 480 )
	{
		pRect->bottom = pRect->top + 480;
	}

	CDialog::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
	CRect rect;
	GetClientRect(&rect);
	m_btnMinmize.SetWindowPos( NULL, rect.right - 60, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	m_btnMinmize.RedrawWindow();

	m_btnMaximize.SetWindowPos( NULL, rect.right - 40, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	m_btnMaximize.RedrawWindow();

	m_btnExit.SetWindowPos( NULL, rect.right - 21, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	m_btnExit.RedrawWindow();

	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect rect;
	GetClientRect(&rect);

	if( m_btnMinmize.m_hWnd != NULL )
	{
		m_btnMinmize.SetWindowPos( NULL, rect.right - 60, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		m_btnMinmize.RedrawWindow();
	}

	if( m_btnMaximize.m_hWnd != NULL )
	{
		m_btnMaximize.SetWindowPos( NULL, rect.right - 40, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		m_btnMaximize.RedrawWindow();
	}

	if( m_btnExit.m_hWnd != NULL )
	{
		m_btnExit.SetWindowPos( NULL, rect.right - 21, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		m_btnExit.RedrawWindow();
	}
	blNeedResign = TRUE;
}

void CVolMointorDlg::OnMouseDiff_X( short zDelta )
{
	if( zDelta > 0 )
	{
		if( Pan_X_strange != 0 )
		{
			if( Pan_X_strange > 0 )
			{
				Pan_X_strange--;
				switch( Pan_X_strange )
				{
				case 0:
					Diff_step.x = Diff_step.x/2;
					break;
				case 1:
					Diff_step.x = (LONG)(Diff_step.x/2.5);
					break;
				case 2:
					Diff_step.x = Diff_step.x/2;
					break;
				case 3:
					Diff_step.x = Diff_step.x/2;
					break;
				case 4:
					Diff_step.x = (LONG)(Diff_step.x/2.5);
					break;
				case 5:
					Diff_step.x = Diff_step.x/2;
					break;
				}
			}
		}
		else
		{
			for( int i = 0; i < SCALE_NUM - 1; i++ )
			{
				if( Pan_X == pan[i] )
				{
					Pan_X = pan[i+1];
					break;
				}
			}
		}
	}
	else
	{
		if( Pan_X == pan[0] )
		{
			if( Pan_X_strange < SCALE_TIME_NUM - 1 )
			{
				Pan_X_strange++;
				switch( Pan_X_strange )
				{
				case 1:
					Diff_step.x *= 2;
					break;
				case 2:
					Diff_step.x = (LONG)(Diff_step.x*2.5);
					break;
				case 3:
					Diff_step.x *= 2;
					break;
				case 4:
					Diff_step.x *= 2;
					break;
				case 5:
					Diff_step.x = (LONG)(Diff_step.x*2.5);
					break;
				case 6:
					Diff_step.x *= 2;
					break;
				}
			}
		}
		else
		{
			for( int i = 1; i < SCALE_NUM; i++ )
			{
				if( Pan_X == pan[i] )
				{
					Pan_X = pan[i-1];
					break;
				}
			}
		}
	}

	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnMouseDiff_Y( short zDelta )
{
	if( zDelta > 0 )
	{
		if(::GetKeyState(VK_CONTROL) < 0 )
		{
			Pan_Y+=10;
		}
		else
		{
			Pan_Y++;
		}
	}
	else
	{
		if(::GetKeyState(VK_CONTROL) < 0 )
		{
			Pan_Y-=10;
			if( Pan_Y < 1 ) Pan_Y = 1;
		}
		else
		{
			if( Pan_Y > 1 )
			{
				Pan_Y--;
			}
		}
	}

	blNeedRefresh = TRUE;
}

BOOL CVolMointorDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
    CRect rect;    
	ScreenToClient(&pt);

	// time grid resolution
	GetClientRect( rect );
	rect.top = DIFF_Y; //rect.bottom - DIFF_GRID_Y;
	rect.left = DIFF_GRID_X;
	rect.bottom -= DIFF_GRID_Y;
    if( rect.PtInRect(pt) )
	{
		//int tmp = Diff_step.x*Pan_X - pt.x*Pan_X/GridStep_X;
		OnMouseDiff_X( zDelta );
	}

	// voltage grid resolution
	GetClientRect( rect );	rect.top = DIFF_Y;
	rect.bottom -= DIFF_GRID_Y;
	rect.right = DIFF_GRID_X;
    if( rect.PtInRect(pt) )
	{
		OnMouseDiff_Y( zDelta );
	}

	// modify horizontal grid number
	GetClientRect( rect );
	rect.top = rect.bottom - DIFF_GRID_Y;
	if( rect.PtInRect(pt) )
	{
		if( zDelta > 0 )
		{
			if( horizontal_grid_num < 25 )
				horizontal_grid_num++;
		}
		else
		{
			if( horizontal_grid_num > 5 )
				horizontal_grid_num--;
		}

		blNeedRefresh = TRUE;
	}
	blNeedResign = TRUE;
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CVolMointorDlg::OnBnClickedMaximize()
{
	// TODO: Add your control notification handler code here
    if ( this->IsZoomed() )
    {
        ::PostMessage( this->m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0 );
		m_btnMaximize.InitImage( img_max_norm, img_max_norm, img_max_norm, img_max_focu );
		Invalidate();
    }
    else
    {        
		::PostMessage( this->m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
		m_btnMaximize.InitImage( img_res_norm, img_res_norm, img_res_norm, img_res_focu );
		m_btnMaximize.Invalidate();
    }
}

void CVolMointorDlg::OnBnClickedMinimize()
{
	// TODO: Add your control notification handler code here
	::PostMessage( this->m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0 );
}

void CVolMointorDlg::OnBnClickedAppExit()
{
	// TODO: Add your control notification handler code here
	KillTimer( READ_TIMER );
	OnBnClickedDisconnect();
	::PostMessage( this->m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0 );
}

void CVolMointorDlg::OnBnClickedDisconnect()
{
	// TODO: Add your control notification handler code here
	//KillTimer( READ_TIMER );
	//Command.cmd = 0xff; Command.para1 = 0xff; Command.para2 = 0xff;
	//EC3Write( (BYTE*)&Command, 3 );
	Disconnect();
}

void CVolMointorDlg::OnBnClickedBatcheck1()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[0] == TRUE )
	{
		m_CheckBat[0] = FALSE;
	}
	else
	{
		m_CheckBat[0] = TRUE;
	}

	m_btnCheckBat1.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck2()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[1] == TRUE )
	{
		m_CheckBat[1] = FALSE;
	}
	else
	{
		m_CheckBat[1] = TRUE;
	}

	m_btnCheckBat2.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck3()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[2] == TRUE )
	{
		m_CheckBat[2] = FALSE;
	}
	else
	{
		m_CheckBat[2] = TRUE;
	}

	m_btnCheckBat3.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck4()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[3] == TRUE )
	{
		m_CheckBat[3] = FALSE;
	}
	else
	{
		m_CheckBat[3] = TRUE;
	}

	m_btnCheckBat4.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck5()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[4] == TRUE )
	{	
		m_CheckBat[4] = FALSE;
	}
	else
	{
		m_CheckBat[4] = TRUE;
	}

	m_btnCheckBat5.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck6()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[5] == TRUE )
	{
		m_CheckBat[5] = FALSE;
	}
	else
	{
		m_CheckBat[5] = TRUE;
	}

	m_btnCheckBat6.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck7()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[6] == TRUE )
	{
		m_CheckBat[6] = FALSE;
	}
	else
	{
		m_CheckBat[6] = TRUE;
	}

	m_btnCheckBat7.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck8()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[7] == TRUE )
	{
		m_CheckBat[7] = FALSE;
	}
	else
	{
		m_CheckBat[7] = TRUE;
	}

	m_btnCheckBat8.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck9()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[8] == TRUE )
	{
		m_CheckBat[8] = FALSE;
	}
	else
	{
		m_CheckBat[8] = TRUE;
	}

	m_btnCheckBat9.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck10()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[9] == TRUE )
	{
		m_CheckBat[9] = FALSE;
	}
	else
	{
		m_CheckBat[9] = TRUE;
	}

	m_btnCheckBat10.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck11()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[10] == TRUE )
	{
		m_CheckBat[10] = FALSE;
	}
	else
	{
		m_CheckBat[10] = TRUE;
	}

	m_btnCheckBat11.Invalidate();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnBnClickedBatcheck12()
{
	// TODO: Add your control notification handler code here
	if( m_CheckBat[11] == TRUE )
	{
		m_CheckBat[11] = FALSE;
	}
	else
	{
		m_CheckBat[11] = TRUE;
	}

	m_btnCheckBat12.Invalidate();
	blNeedRefresh = TRUE;
}

#include "ClassChannelSetup.h"
void CVolMointorDlg::OnBnClickedOption()
{
	// TODO: Add your control notification handler code here
	CClassChannelSetup dlg;
	dlg.DoModal();
	blNeedRefresh = TRUE;
}

void CVolMointorDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
    CRect rect;
    GetClientRect( rect );
	rect.bottom = 20;

    if( rect.PtInRect(point) )
    {
        ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));

        if(nFlags & MK_LBUTTON )
        {
            ReleaseCapture();
            ::SendMessage( this->m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            CDialog::OnMouseMove(nFlags, point);
            return;
        }
    }

	GetClientRect(&rect);
	rect.top = DIFF_Y;
	rect.left += DIFF_GRID_X;
	rect.bottom -= DIFF_GRID_Y;
    if( rect.PtInRect(point) )
	{
		point.x = point.x - rect.left;
		point.y = rect.bottom - point.y - 1;
		double vol =(double)point.y/GridStep_Y*GRID_VOLTAGE*Pan_Y + + Diff_step.y*GRID_VOLTAGE*Pan_Y;
		CString str;
		double sec = 0;

		if( Pan_X_strange != 0 )
		{
			sec = ((double)point.x/GridStep_X*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000;
		}
		else
		{
			sec = (double)point.x/GridStep_X*Pan_X - Diff_step.x;
		}

		if( sec < 60 )
		{
			str.Format( "%.3fV @ %.3f\"", vol, sec );
		}
		else if( sec < 60*60 )
		{
			int min = (int)sec/60;
			sec = sec - min*60;
			str.Format( "%.3fV @ %d\' %.3f\"", vol, min, sec );
		}
		else
		{
			int hour = (int)sec/60/60;
			int min = (int)(sec - hour*60*60)/60;
			sec = sec - min*60 - hour*60*60;
			str.Format( "%.3fV @ %dh %d\' %.3f\"", vol, hour, min, sec );
		}

		m_tool.UpdateTipText(str,this);
	}

    //m_btnIdleFocus.SetFocus();
	CDialog::OnMouseMove(nFlags, point);
}

void CVolMointorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
    CRect rect;
    GetClientRect( rect );
	rect.bottom = 20;

    if( rect.PtInRect(point) )
    {
		OnBnClickedMaximize();
	}

	GetClientRect( rect );
	rect.top = rect.bottom - DIFF_GRID_Y;
	rect.right = DIFF_GRID_X;
	if( rect.PtInRect(point) )
	{
		Diff_step = CPoint(0,0);
		Old_point = CPoint(0,0);
		New_point = CPoint(0,0);
		Diff_point = CPoint(0,0);
		
		blNeedRefresh = TRUE;
	}

	GetClientRect( rect );
	rect.top = rect.bottom - DIFF_GRID_Y;
	rect.left = DIFF_GRID_X;
	if( rect.PtInRect(point) )
	{
		if( blIncreaseDisplay == TRUE )
		{
			blIncreaseDisplay = FALSE;
		}
		else
		{
			blIncreaseDisplay = TRUE;
		}

		blNeedRefresh = TRUE;
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CVolMointorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect; 
	GetClientRect(&rect);
	rect.top = DIFF_Y;

	if( rect.PtInRect(point) )
	{
		Old_point = point;
	}
	else
	{
		Old_point.x = 0; Old_point.y = 0;
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CVolMointorDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(&rect);
	rect.top = DIFF_Y;
	rect.left += DIFF_GRID_X;
	rect.bottom -= DIFF_GRID_Y;
    if( rect.PtInRect(point) )
	{
		point.x = point.x - rect.left;
		point.y = rect.bottom - point.y - 1;
		double vol =(double)point.y/GridStep_Y*GRID_VOLTAGE*Pan_Y + + Diff_step.y*GRID_VOLTAGE*Pan_Y;
		CString str;
		double sec = 0;

		if( Pan_X_strange != 0 )
		{
			sec = ((double)point.x/GridStep_X*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000;
		}
		else
		{
			sec = (double)point.x/GridStep_X*Pan_X - Diff_step.x;
		}

		//if(::GetKeyState(VK_CONTROL) < 0 )
		if( blSelect_pos == FALSE )
		{
			Select_Pos2 = sec;
			Select_Vol2 = vol;
		}
		else
		{
			Select_Pos1 = sec;
			Select_Vol1 = vol;
		}

		if( sec < 60 )
		{
			str.Format( "%.3fV @ %.3f\"", vol, sec );
		}
		else if( sec < 60*60 )
		{
			int min = (int)sec/60;
			sec = sec - min*60;
			str.Format( "%.3fV @ %d\' %.3f\"", vol, min, sec );
		}
		else
		{
			int hour = (int)sec/60/60;
			int min = (int)(sec - hour*60*60)/60;
			sec = sec - min*60 - hour*60*60;
			str.Format( "%.3fV @ %dh %d\' %.3f\"", vol, hour, min, sec );
		}

		//if(::GetKeyState(VK_CONTROL) < 0 )
		if( blSelect_pos == FALSE )
		{
			Select_str_Pos2 = str;
		}
		else
		{
			Select_str_Pos1 = str;
		}

		blNeedRefresh = TRUE;
	}

	CDialog::OnRButtonDown(nFlags, point);
}

void CVolMointorDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	New_point = point;
	CRect rect;
	GetClientRect(&rect);
	rect.top = DIFF_Y;

	if( rect.PtInRect(point) && Old_point.x != 0 )
	{
		BOOL blUsed = FALSE;
		CPoint tmp_point = New_point - Old_point;

		if( abs(tmp_point.y) > 3 )
		{
			Diff_point.y = New_point.y - Old_point.y;
			Diff_step.y += Diff_point.y/GridStep_Y;//*GridStep_Y;
			if( Diff_step.y < 0 )
			{
				//Diff_step.y = 0;
			}

			blNeedRefresh = TRUE;
			blUsed = TRUE;
		}

		if( abs(tmp_point.x) > 3 )
		{
			Diff_point.x = New_point.x - Old_point.x;
			if( Pan_X < 60 )
			{
				Diff_step.x += Diff_point.x*Pan_X/GridStep_X;//*GridStep_X;
			}
			else if( Pan_X < 60*60 )
			{
				Diff_step.x += Diff_point.x*Pan_X/GridStep_X/60*60;//*GridStep_X;
			}
			else
			{
				Diff_step.x += Diff_point.x*Pan_X/GridStep_X/60/60*60*60;//*GridStep_X;
			}

			if( Diff_step.x > 0 )
			{
				Diff_step.x = 0;
			}

			blNeedRefresh = TRUE;
			blUsed = TRUE;
		}

		if( blUsed == FALSE )
		{
			blSelect_pos = TRUE;
			OnRButtonDown(nFlags, point);
			blSelect_pos = FALSE;
		}
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CVolMointorDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if( blConnected == FALSE )
	{
		UpdateDeviceCombo();
	}

	if( blNeedRefresh == TRUE )
	{
		blNeedRefresh = FALSE;
		DrawWave();
	}
}

DWORD GetDispPos( DWORD *start_pos, DWORD *end_pos, DWORD start_time, DWORD end_time )
{
	DWORD pos_sta, pos_end;

	start_time = (DWORD)(start_time*1.000); //1.002
	end_time = (DWORD)(end_time*1.000);

	for( pos_sta = 0; pos_sta < GroupData.info.GetDataPos; pos_sta++ )
	{
		if( GroupData.TimeBase[ pos_sta ] >= start_time )
		{
			break;
		}
	}

	for( pos_end = pos_sta; pos_end < GroupData.info.GetDataPos; pos_end++ )
	{
		if( GroupData.TimeBase[ pos_end + 1 ] == 0 )
		{
			*start_pos = pos_sta;
			*end_pos = pos_end;
			return GroupData.TimeBase[ pos_end ] - GroupData.TimeBase[ pos_sta ];
		}

		if( GroupData.TimeBase[ pos_end ] >= end_time )
		{
			break;
		}
	}

	*start_pos = pos_sta;
	*end_pos = pos_end;
	return GroupData.TimeBase[ pos_end ] - GroupData.TimeBase[ pos_sta ];
}

void CVolMointorDlg::OnBnClickedExport()
{
	// TODO: Add your control notification handler code here
	const char *g_Optionfilefilter = "Binary data file (*.BIN)\0*.BIN";

    CFileDialog dlg( FALSE, "BIN", "*.BIN",
        OFN_HIDEREADONLY | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, g_Optionfilefilter, 0 );
    dlg.m_ofn.lpstrInitialDir = g_ExecFileDir;
    if( dlg.DoModal() == IDOK )
    {
        CString filename = dlg.GetPathName();

		FILE *fp;
	    fp = fopen( filename, "wb" );
		if( fp == NULL )
		{
			return;
		}

		strcpy( (char*)GroupData.info.Signature, "BatMon" );

		for( int i = 0; i < BAT_NUM; i++ )
		{
			GroupData.info.Enable[ i ] = m_CheckBat[ i ];

			for( int c = 0; c <  20; c++ )
			{
				GroupData.info.ChannelName[ i ][ c ] = 0;
			}

			for( int c = 0; c <  ChannelName[i].GetLength(); c++ )
			{
				GroupData.info.ChannelName[ i ][ c ] = ChannelName[i].GetAt( c );
			}
		}

		GroupData.info.ChannelNum = BAT_NUM;

		DWORD len =	(DWORD)fwrite( (void*)&GroupData, sizeof(GroupData), 1, fp );
		fclose( fp );
    }
}

void CVolMointorDlg::OnBnClickedImport()
{
	// TODO: Add your control notification handler code here
	const char *g_Optionfilefilter = "Binary data file (*.BIN)\0*.BIN";

    CFileDialog dlg( TRUE, "BIN", "*.BIN",
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, g_Optionfilefilter, 0 );
    dlg.m_ofn.lpstrInitialDir = g_ExecFileDir;
    if( dlg.DoModal() == IDOK )
    {
        CString filename = dlg.GetPathName();

		FILE *fp;
	    fp = fopen( filename, "rb" );
		if( fp == NULL )
		{
			return;
		}

		//DWORD len =	(DWORD)fread( GroupData.GetData[0], 2, TOTAL_SAMPLE_NUM, fp );
		//GroupData.GetDataPos[0] = len;
		DWORD len = (DWORD)fread( (void*)&GroupData, sizeof(GroupData), 1, fp );
		fclose( fp );

		if( strcmp( (char*)GroupData.info.Signature, "BatMon" ) != 0 )
		{
			AfxMessageBox("Unknown file");
			return;
		}

		if( blConnected == TRUE )
		{
			OnBnClickedConnect();
		}

		for( int i = 0; i < BAT_NUM; i++ )
		{
			m_CheckBat[ i ] = GroupData.info.Enable[ i ];

			ChannelName[i] = (char*)GroupData.info.ChannelName[ i ];

			if( ChannelName[i].GetLength() > 20 )
				ChannelName[i].Truncate(20);
		}

		//m_nChannelNum = GroupData.info.ChannelNum;

		OnBnClickedBatcheck1();
		OnBnClickedBatcheck1();
		OnBnClickedBatcheck2();
		OnBnClickedBatcheck2();
		OnBnClickedBatcheck3();
		OnBnClickedBatcheck3();
		OnBnClickedBatcheck4();
		OnBnClickedBatcheck4();
		OnBnClickedBatcheck5();
		OnBnClickedBatcheck5();
		OnBnClickedBatcheck6();
		OnBnClickedBatcheck6();
		OnBnClickedBatcheck7();
		OnBnClickedBatcheck7();
		OnBnClickedBatcheck8();
		OnBnClickedBatcheck8();

		if( blRealDisplay == TRUE )
		{
			OnBnClickedRecord();
		}

		blNeedRefresh = TRUE;
	}
}

int WaitReturn( void )
{
	DWORD bytesReturned = 0;
	BYTE getreport[512];

	if( HID_DEVICE_SUCCESS != HidDevice_GetInputReport_Interrupt( m_hid, getreport, 60, 1, &bytesReturned ))
	{
		return 1;
	}
	//for( int i = 0; i < bytesReturned; i++ )
	//{
		//printf( "%02x", getreport[i] );
	//}
	return 0;
}

BYTE firmware[8192] =
{
	#include "..\EFM8UB1_HIDADC.txt"
};

int firmware_up_pos = 0;
int readdata(void *dst, int elementsize, int count)
{
	memcpy(dst,&firmware[firmware_up_pos],elementsize*count );
	firmware_up_pos+=(elementsize*count);
	return elementsize*count;
}

int upgrade( void )
{
	BYTE report[512];
	firmware_up_pos = 0;

	for( ;; )
	{
		memset( report, 0, 65 );
		report[0] = ID_OUT_CONTROL;

		if( 1 != readdata( &report[ 1 ], 1, 1 ))
		{
			break;
		}

		if( report[ 1 ] != '$' )
		{
			// finished
			break;
		}

		if( 1 != readdata( &report[ 2 ], 1, 1 ))
		{
			break;
		}

		if( report[ 2 ] > 62 + 64 )
		{
			int size = report[ 2 ];

			if( 62 != readdata( &report[ 3 ], 1, 62 ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}

			//if( WaitReturn() != 0 )
			//{
			//	goto Error;
			//}

			size -= 62;
			if( 64 != readdata( &report[ 1 ], 1, 64 ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}
			//if( WaitReturn() != 0 )
			//{
			//	goto Error;
			//}

			size -= 64;
			memset( &report[1], 0, 64 );
			if( size != readdata( &report[ 1 ], 1, size ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}

			if( WaitReturn() != 0 )
			{
				goto Error;
			}
		}
		else if( report[ 2 ] > 62 )
		{
			int size = report[ 2 ];

			if( 62 != readdata( &report[ 3 ], 1, 62 ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}
			//if( WaitReturn() != 0 )
			//{
			//	goto Error;
			//}

			size -= 62;
			if( size != readdata( &report[ 1 ], 1, size ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}
			if( WaitReturn() != 0 )
			{
				goto Error;
			}
		}
		else
		{
			if( report[ 2 ] != readdata( &report[ 3 ], 1, report[ 2 ] ))
			{
				break;
			}

			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
				goto Error;
			}
			if( WaitReturn() != 0 )
			{
				goto Error;
			}
		}
	}

	return 0;
Error:
	return 1;
}

void CVolMointorDlg::OnBnClickedUpgrade()
{
	if( m_hid != NULL )
	{
		AfxMessageBox("Connect error.");
		return;
	}

	if( FALSE == Connect() )
	{
		AfxMessageBox("Connect error");
		return;
	}

	BYTE report[512] = { 0 };

	report[0] = ID_OUT_CONTROL;
	report[ 1 ] = '$'; // Start
	report[ 2 ] = 1;
	report[ 3 ] = 0x48;
	if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
	{
		AfxMessageBox("Send message fail");
	}
	if( WaitReturn() != 0 )
	{
		//goto Error;
	}

	Disconnect();
	for(;;)
	{
		Sleep(100);
		UpdateDeviceCombo();
		if( 0 != HidDevice_GetNumHidDevices(VID, PID))
		{
			break;
		}
	}
	if( FALSE == Connect() )
	{
		AfxMessageBox("Connect error after reboot");
		return;
	}
	
	int err = upgrade();
	OnBnClickedDisconnect();

	if( err == 0 )
		AfxMessageBox("Finished");
	else
		AfxMessageBox("Error");

}

void CVolMointorDlg::OnBnClickedConnect()
{
	if( blConnected == FALSE )
	{
		if( TRUE == Connect() )
		{
			blConnected = TRUE;
			m_comboDevice.EnableWindow(FALSE);
			m_ComboSampleRate.EnableWindow(FALSE);

			m_btnConnect.InitImage( img_disconnect1, img_disconnect1, img_disconnect1, img_disconnect1_a );
			m_btnConnect.RedrawWindow();

			for( int bat = 0; bat < BAT_NUM; bat++ )
			{
				for( int i = 0; i < TOTAL_SAMPLE_NUM; i++ )
				{
					GroupData.GetData[bat][i] = 0;//(unsigned short)(500 + bat*100+i/100*sin( i/(90.*(1+bat))*3.14 ));
					GroupData.TimeBase[ i ] = 0;
				}
				GroupData.info.GetDataPos = 0;
			}

			Diff_step.x = 0;
#if 0
			BYTE report[512] = { 0 };

			report[0] = ID_OUT_CONTROL;
			report[ 1 ] = '$'; // Start
			report[ 2 ] = 32;
			report[ 3 ] = 0x40;

			for( int i = 0; i < BAT_NUM; i++ )
			{
				report[ 4 + i ] = ChannelPos[ i ];
			}
			if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
			{
					AfxMessageBox("Set report error");
					HidDevice_Close(m_hid); m_hid = NULL;
					return;
			}
#endif
			//if( sample_delay == 0 )
			{
				Thread = ::CreateThread(NULL, 0, ThreadFunc, NULL, 0, &dwThreadId);
				SetThreadPriority( Thread, THREAD_PRIORITY_NORMAL ); //THREAD_PRIORITY_ABOVE_NORMAL );THREAD_PRIORITY_HIGHEST
			}
			//else
			//{
			//	StartSampleTimer();
			//}

			if( blRealDisplay == FALSE )
			{
				OnBnClickedRecord();
			}
		}
		else
		{
			UpdateDeviceCombo();
		}
	}
	else
	{
		if( blConnected == TRUE )
		{
			OnBnClickedDisconnect();
			blConnected = FALSE;
		}
		m_btnConnect.InitImage( img_connect1, img_connect1, img_connect1, img_connect1_a );
		m_btnConnect.RedrawWindow();
		m_comboDevice.EnableWindow(TRUE);
//		m_ComboSampleRate.EnableWindow(TRUE);
	}
}

DWORD WINAPI ThreadFunc(HANDLE Thread)
{
	BOOL start = FALSE;
	int count = 0;
	BOOL blFirst = 1;

	LARGE_INTEGER freq, nnstart_time, nncur_time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&nnstart_time);
	QueryPerformanceCounter(&nncur_time);

	start_time = ::GetTickCount();
	GroupData.TimeBase[ GroupData.info.GetDataPos ] = 0;

	for(;;)
	{
		if( GroupData.info.GetDataPos >= TOTAL_SAMPLE_NUM - 2 )
		{
			Sleep(1000);
			continue;
		}

		memset( report, 0, 65 );
		report[0] = ID_OUT_CONTROL;

		report[ 1 ] = '$'; // Start
		report[ 2 ] =  4;  // Number
		report[ 3 ] =  0x41;// Command
		if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
		{
			continue;
		}

		DWORD bytesReturned = 0;
		BYTE err;
		for(;;)
		{
			err = HidDevice_GetInputReport_Interrupt( m_hid, getreport, 512, 1, &bytesReturned );
			if( err == HID_DEVICE_SUCCESS )
			{
				// A good package
				break;
			}
			else
			{
				// Close and open
				if( m_hid != NULL )
				{
					HidDevice_Close(m_hid); m_hid = NULL;
				}

				BYTE status = HidDevice_Open(&m_hid, 0, VID, PID, MAX_REPORT_REQUEST_XP);
				if( status == HID_DEVICE_SUCCESS )
				{
					HidDevice_SetTimeouts( m_hid, 500, 500 ); // get/set timeout
					break;
				}
				else
				{
					continue;
				}
#if 0
				report[0] = ID_OUT_CONTROL;
				report[ 1 ] = '$'; // Start
				report[ 2 ] = 32;
				report[ 3 ] = 0x40;

				for( int i = 0; i < BAT_NUM; i++ )
				{
					report[ 4 + i ] = ChannelPos[ i ];
				}
				if( HidDevice_SetOutputReport_Interrupt( m_hid, report, 65 ) != HID_DEVICE_SUCCESS )
				{
					HidDevice_Close(m_hid); m_hid = NULL;
					continue;
				}
				else
				{
					break;
				}
#endif
			}
		}

		if( bytesReturned != 0 )
		{
			QueryPerformanceCounter(&nncur_time);
			cur_time = ::GetTickCount();
			if( start == FALSE )
			{
				start = TRUE;
			}
			else
			{
				if( cur_time - start_time > 998 )
				{
					start = FALSE;
					get_speed = count;
				}

				if( cur_time - start_time > 98 ) // refresh screen
				{
					blNeedRefresh = TRUE;
				}
			}

			count++;
			if( blFirst == 1 )
			{
				if( count < 150 )
				{
					continue;
				}
				start_time = cur_time;
				blFirst = 0;
			}

			static int ignoresample = 0;
			if( sample_delay != 1000 )
			{
				ignoresample++;
				if( ignoresample < sample_delay )
				{
					ignoresample = 0;
					continue;
				}
			}

			GroupData.TimeBase[ GroupData.info.GetDataPos ] = (DWORD)(1000*(nncur_time.QuadPart - nnstart_time.QuadPart)/freq.QuadPart);

			for( int i = 0; i < BAT_NUM; i++ )
			{
				int a = (getreport[ 2+i*2 ] << 8) | getreport[ 2 + i*2 + 1 ]; // + 4
				if( (a & 0xffff) == 0xffff )
				{
					break;
				}

				GroupData.GetData[ i ][ GroupData.info.GetDataPos ] = a;
				if( GroupData.info.GetDataPos >= TOTAL_SAMPLE_NUM )
				{
					// GroupData.GetDataPos[ i&7 ] = TOTAL_SAMPLE_NUM - 1;
					break;
				}
			}
			GroupData.info.GetDataPos++;
		}

		if( sample_delay != 0 )
		{
			Sleep( sample_delay );
		}
	}

	return 0;
}

void CVolMointorDlg::OnBnClickedRecord()
{
	// TODO: Add your control notification handler code here
	if( blRealDisplay == TRUE )
	{
		blRealDisplay = FALSE;
		m_btnRecord.InitImage( img_record1, img_record1, img_record1, img_record1_a );
		//m_btnRecord.SetWindowText( "自动" );
	}
	else
	{
		blRealDisplay = TRUE;
		m_btnRecord.InitImage( img_record2, img_record2, img_record2, img_record2_a );
		//m_btnRecord.SetWindowText("暂停");
	}
	m_btnRecord.RedrawWindow();
}

BOOL CVolMointorDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
	//return CDialog::OnHelpInfo(pHelpInfo);
}

void CVolMointorDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch( nChar )
	{
		case 'h':
		case 'H':
			blDispVSel = !blDispVSel;
			blNeedRefresh = TRUE;

			if( blDispVSel )
			{
				//m_btnVolCur.InitImage( img_vol_b, img_vol_b, img_vol_b, img_vol_b );
			}
			else
			{
				//m_btnVolCur.InitImage( img_vol_a, img_vol_a, img_vol_a, img_vol_a );
			}
			m_btnVolCur.Invalidate();			
			break;

		case 'v':
		case 'V':
			blDispHSel = !blDispHSel;
			blNeedRefresh = TRUE;

			if( blDispHSel )
			{
				//m_btnTimeCur.InitImage( img_tim_b, img_tim_b, img_tim_b, img_tim_b );			
			}
			else
			{
				//m_btnTimeCur.InitImage( img_tim_a, img_tim_a, img_tim_a, img_tim_a );			
			}
			m_btnTimeCur.Invalidate();
			break;

	}

	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CVolMointorDlg::OnBnClickedVoltageCursor()
{
	// TODO: Add your control notification handler code here
	OnChar( 'h', 0, 0 );
}

void CVolMointorDlg::OnBnClickedTimeCursor()
{
	// TODO: Add your control notification handler code here
	OnChar( 'v', 0, 0 );
}

void CVolMointorDlg::OnCbnKillfocusSampleRate()
{
	// TODO: Add your control notification handler code here
	switch( m_ComboSampleRate.GetCurSel() )
	{
	case 0:
		sample_delay = 1000;
		break;
	case 1:
		sample_delay = 500;
		break;
	case 2:
		sample_delay = 200;
		break;
	case 3:
		sample_delay = 100;
		break;
	case 4:
		sample_delay = 50;
		break;
	case 5:
		sample_delay = 40;
		break;
	case 6:
		sample_delay = 20;
		break;
	case 7:
		sample_delay = 10;
		break;
	case 8:
		sample_delay = 2;
		break;
	default:
		sample_delay = 0;
		break;
	}
}

void CVolMointorDlg::DrawWave()
{
	int i;
	int start_grid_ms = 1000;
	int ms_per_grid = 1000, ms_per_grid_ori = 1000;

	char buf1[100], buf2[100];
	CRect rect;
	CClientDC dc(this);
	GetClientRect(&rect);

	// Fill rect
	rect.top = DIFF_Y;

	CDC *pDC = CDC::FromHandle( dc );
	CMemDC dcMemory( pDC, rect );
	dcMemory.FillSolidRect(rect, RGB(0x0,0x0,0x0));
	dcMemory.SetTextColor( GRID_TEXT_COLOR );
	dcMemory.SetBkMode(TRANSPARENT);

	rect.bottom -= DIFF_GRID_Y;
	GridStep_Y = (rect.bottom - rect.top - 10 )/vertical_grid_num;

	// vertical grid text
	for( i = 0; i <= vertical_grid_num; i++ )
	{
		if( i == vertical_grid_num )
		{
			sprintf( buf1,"%.2f(V)", (float)i*GRID_VOLTAGE*Pan_Y + Diff_step.y*GRID_VOLTAGE*Pan_Y );
		}
		else
		{
			sprintf( buf1,"%.2f", (float)i*GRID_VOLTAGE*Pan_Y + Diff_step.y*GRID_VOLTAGE*Pan_Y );
		}

		CSize siz = dcMemory.GetTextExtent( buf1 );
		if( i == 0 )
		{
			dcMemory.TextOut( DIFF_GRID_X - siz.cx - 3, rect.bottom - i*GridStep_Y - siz.cy, buf1 );
		}
		else
		{
			dcMemory.TextOut( DIFF_GRID_X - siz.cx - 3, rect.bottom - i*GridStep_Y - siz.cy/2, buf1 );
		}

		//if( i == vertical_grid_num )
		//{
		//	dcMemory.TextOut( DIFF_GRID_X - siz.cx - 3, rect.bottom - i*GridStep_Y - siz.cy/2 + siz.cy, "V" );
		//}
	}

	// horizontal grid (voltage)
	for( i = 0; i <= 2*vertical_grid_num; i++ )
	{
		if( (i&1) == 0 )
		{
			for( int a = DIFF_GRID_X; a < rect.right; a++ )
			{
				dcMemory.SetPixel( a, rect.bottom - i*GridStep_Y/2, GRID_LINE_COLOR );
			}
		}
		else
		{
			for( int a = DIFF_GRID_X; a < rect.right; a+=5 ) // dot line
			{
				dcMemory.SetPixel( a, rect.bottom - i*GridStep_Y/2, GRID_LINE_COLOR );
			}
		}
	}

	// Selected voltage
	if( blDispVSel )
	{
		if( Select_Vol1 != 0 )
		{
			double vol = Select_Vol1 - Diff_step.y*GRID_VOLTAGE*Pan_Y;
			int pos = (int)(vol/Pan_Y/GRID_VOLTAGE*GridStep_Y);

			for( int a = DIFF_GRID_X; a < rect.right; a++ )
			{
				dcMemory.SetPixel( a, rect.bottom - pos, SELECT_POS1_COLOR );
			}
		}

		if( Select_Vol2 != 0 )
		{
			double vol = Select_Vol2 - Diff_step.y*GRID_VOLTAGE*Pan_Y;
			int pos = (int)(vol/Pan_Y/GRID_VOLTAGE*GridStep_Y);

			for( int a = DIFF_GRID_X; a < rect.right; a++ )
			{
				dcMemory.SetPixel( a, rect.bottom - pos, SELECT_POS2_COLOR );
			}
		}
	}

	rect.left += DIFF_GRID_X;
	GridStep_X = (int)((rect.right - rect.left)/(horizontal_grid_num + 0.5));
	if( GridStep_X < 0 )
	{
		GridStep_X = 0;
	}

	// horizontal grid text
	#define UNIT_POS horizontal_grid_num

	if( Pan_X_strange != 0 )
	{
		start_grid_ms = -Diff_step.x*divtime[Pan_X_strange];
		ms_per_grid = divtime[Pan_X_strange];
	}
	else
	{
		start_grid_ms = -Diff_step.x*1000;

		//if( start_grid_ms < 0 ) start_grid_ms = 0;

		ms_per_grid = Pan_X*1000;
	}

	for( i = 0; i <= horizontal_grid_num; i++ )
	{
		if( Pan_X_strange != 0 )
		{
			if( i == 0 )
			{
				if( Pan_X_strange >= 4 )
					sprintf( buf1,"%.2f", (-Diff_step.x)*divtime[Pan_X_strange]/1000. );
				else
					sprintf( buf1,"%.1f", (-Diff_step.x)*divtime[Pan_X_strange]/1000. );
			}
			else
			{
				if( blIncreaseDisplay == TRUE )
				{
					if( Pan_X_strange >= 4 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"+%.2f(s)", ((float)i*Pan_X)*divtime[Pan_X_strange]/1000 );
						else
							sprintf( buf1,"+%.2f", ((float)i*Pan_X)*divtime[Pan_X_strange]/1000 );
					}
					else
					{
						if( i == UNIT_POS )
							sprintf( buf1,"+%.1f(s)", ((float)i*Pan_X)*divtime[Pan_X_strange]/1000 );
						else
							sprintf( buf1,"+%.1f", ((float)i*Pan_X)*divtime[Pan_X_strange]/1000 );
					}
				}
				else
				{
					if( Pan_X_strange >= 4 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"%.2f(s)", ((float)i*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000 );
						else
							sprintf( buf1,"%.2f", ((float)i*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000 );
					}
					else
					{
						if( i == UNIT_POS )
							sprintf( buf1,"%.1f(s)", ((float)i*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000 );
						else
							sprintf( buf1,"%.1f", ((float)i*Pan_X - Diff_step.x)*divtime[Pan_X_strange]/1000 );
					}
				}
			}
		}
		else
		{
			if( i == 0 )
			{
				if( Pan_X < 60 )
				{
					sprintf( buf1,"%d", -Diff_step.x );
				}
				else if( Pan_X < 60*60 )
				{
					sprintf( buf1,"%d", -Diff_step.x/60 );
				}
				else
				{
					sprintf( buf1,"%d", -Diff_step.x/60/60 );
				}
			}
			else
			{
				if( blIncreaseDisplay == TRUE )
				{
					if( Pan_X < 60 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"+%d(s)", i*Pan_X );
						else
							sprintf( buf1,"+%d", i*Pan_X );
					}
					else if( Pan_X < 60*60 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"+%d(m)", i*Pan_X/60 );
						else
							sprintf( buf1,"+%d", i*Pan_X/60 );
					}
					else
					{
						if( i == UNIT_POS )
							sprintf( buf1,"+%d(h)", i*Pan_X/60/60 );
						else
							sprintf( buf1,"+%d", i*Pan_X/60/60 );
					}
				}
				else
				{
					if( Pan_X < 60 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"%d(s)", i*Pan_X - Diff_step.x );
						else
							sprintf( buf1,"%d", i*Pan_X - Diff_step.x );
					}
					else if( Pan_X < 60*60 )
					{
						if( i == UNIT_POS )
							sprintf( buf1,"%d(m)", i*Pan_X/60 - Diff_step.x/60 );
						else
							sprintf( buf1,"%d", i*Pan_X/60 - Diff_step.x/60 );
					}
					else
					{
						if( i == UNIT_POS )
							sprintf( buf1,"%d(h)", i*Pan_X/60/60 - Diff_step.x/60/60 );
						else
							sprintf( buf1,"%d", i*Pan_X/60/60 - Diff_step.x/60/60 );
					}
				}
			}
		}

		CSize siz = dcMemory.GetTextExtent( buf1 );
		if( i == 0 )
		{
			siz.cx = 0;
		}

		dcMemory.TextOut( rect.left + i*GridStep_X - siz.cx/2, rect.bottom + 2, buf1 );
	}

	// vertical grid
	for( i = 0; i <= horizontal_grid_num*2; i++ )
	{
		if( (i&1) == 0 )
		{
			for( int a = rect.top; a < rect.bottom; a++ )
			{
				dcMemory.SetPixel( rect.left + i*GridStep_X/2, a, GRID_LINE_COLOR );
			}
		}
		else
		{
			for( int a = rect.top; a < rect.bottom; a+=5 ) // dot line
			{
				dcMemory.SetPixel( rect.left + i*GridStep_X/2, a, GRID_LINE_COLOR );
			}
		}
	}

	// Selected time
	if( blDispHSel )
	{
		if( Select_Pos1 != 0 )
		{
			double time;
			double pos;

			if( Pan_X_strange != 0 )
			{
				time = Select_Pos1 + Diff_step.x*divtime[Pan_X_strange]/1000.;
				pos = time/divtime[Pan_X_strange]*1000.*GridStep_X;
			}
			else
			{
				time = Select_Pos1 + Diff_step.x;
				pos = time/Pan_X*GridStep_X;
			}

			for( int a = rect.top; a < rect.bottom; a++ )
			{
				dcMemory.SetPixel( rect.left + (int)pos, a, SELECT_POS1_COLOR );
			}
		}

		if( Select_Pos2 != 0 )
		{
			double time;
			double pos;

			if( Pan_X_strange != 0 )
			{
				time = Select_Pos2 + Diff_step.x*divtime[Pan_X_strange]/1000.;
				pos = time/divtime[Pan_X_strange]*1000.*GridStep_X;
			}
			else
			{
				time = Select_Pos2 + Diff_step.x;
				pos = time/Pan_X*GridStep_X;
			}

			for( int a = rect.top; a < rect.bottom; a++ )
			{
				dcMemory.SetPixel( rect.left + (int)pos, a, SELECT_POS2_COLOR );
			}
		}
	}

	BOOL moved = FALSE;

	CPen penNew[ BAT_NUM + 1 ];
	for( i = 0; i < BAT_NUM + 1; i++ )
	{
		penNew[i].CreatePen(PS_SOLID, 1, color[i]);
	}

	CPen *Old = dcMemory.SelectObject(&penNew[0]);

	int jumpdiff = FALSE;

	for( int bat = 0; bat < BAT_NUM; bat++ )
	{
		if( m_CheckBat[bat] == FALSE )
		{
			continue;
		}

		dcMemory.SelectObject(&penNew[bat]);

		DWORD pos_sta = 0, pos_end = 0, total_time, total_points;
		total_time = horizontal_grid_num*ms_per_grid + ms_per_grid/2;
		total_points = (DWORD)(GridStep_X*(horizontal_grid_num + 0.5));

		DWORD ret = GetDispPos( &pos_sta, &pos_end, start_grid_ms, start_grid_ms + total_time );

		if( ret == 0 )
		{
			return;
		}
		else
		{
			if( blNeedResign == TRUE )
			{
				g_SamplePerPoint = (pos_end - pos_sta)/(double)total_points * (total_time/(double)ret);
				blNeedResign = FALSE;
			}
		}

		moved = FALSE;
		DWORD sample;
		DWORD gd;
		for( gd = 0; gd < total_points; gd++ )
		{
			sample = pos_sta + (DWORD)(g_SamplePerPoint*gd);

			if( sample >= GroupData.info.GetDataPos )
			{
				break;
			}

			int sample_data = 0;
			sample_data = GroupData.GetData[ bat ][ sample ];

			if( moved == FALSE )
			{
				moved = TRUE;
				dcMemory.MoveTo( DIFF_GRID_X + gd,
				(int)(rect.bottom - (sample_data/RATIO * (GridStep_Y*10))/GRID_VOLTAGE*REF_VOLTAGE/10./Pan_Y + Diff_step.y*GridStep_Y ));
			}
			else
			{
				dcMemory.LineTo( DIFF_GRID_X + gd,
				(int)(rect.bottom - (sample_data/RATIO * (GridStep_Y*10))/GRID_VOLTAGE*REF_VOLTAGE/10./Pan_Y + Diff_step.y*GridStep_Y ));
			}
		}

		if( i + gd > (DWORD)horizontal_grid_num*GridStep_X + GridStep_X/2 - 1 )
		{
			jumpdiff = TRUE;
		}
	}

	if( blRealDisplay == TRUE && blConnected == TRUE )
	{
		if( jumpdiff )
		{
			Diff_step.x = (int)(-(GroupData.TimeBase[GroupData.info.GetDataPos - 1]/1000.));
			
			if( Pan_X_strange )
			{
				Diff_step.x *= (LONG)(1000./ms_per_grid);
			}
			else
			{
				Diff_step.x += ( horizontal_grid_num - 1 )*Pan_X; //horizontal_grid_num

				if( Diff_step.x > 0 ) Diff_step.x = 0;
			}
		}
	}

	int textx = 60;
	CSize siz2 = dcMemory.GetTextExtent( "A" );
	for( i = 0; i < BAT_NUM; i++ )
	{
		if( m_CheckBat[ i ] == FALSE )
		{
			continue;
		}

		dcMemory.SetTextColor( color[i] );	

		if( GroupData.info.GetDataPos > 0 )
		{
			sprintf( buf1,"%d: %.2fV", i, GroupData.GetData[ i ][ GroupData.info.GetDataPos - 1 ]*REF_VOLTAGE/RATIO );
		}
		else
		{
			sprintf( buf1,"%d: %.2fV", i, 0. );
		}

		CSize siz1 = dcMemory.GetTextExtent( buf1 );
		dcMemory.TextOut( textx, 85, buf1 );
		dcMemory.TextOut( textx, 85 + siz2.cy, ChannelName[ i ] );

		textx += siz1.cx;
		textx += 15;
	}

	dcMemory.SetTextColor( 0x808080 );
	double sps = 0;
	if( cur_time - start_time != 0 && GroupData.info.GetDataPos > 1 )
	{
#if 0
		if( sample_delay == 0 )
		{
			//sps = (GroupData.TimeBase[GroupData.info.GetDataPos-1]*1000.)/(cur_time-start_time);
			sps = GroupData.info.GetDataPos*1000/(double)GroupData.TimeBase[ GroupData.info.GetDataPos - 1 ];
		}
		else
		{
			sps = (GroupData.info.GetDataPos-1)*1000./(cur_time-start_time);
		}
#endif
		if( sample_delay == 0 )
			sps = 1000.;
		else
			sps = 1000/sample_delay;

		sprintf( buf1,"%.1fsps/ch   ", sps );
	}
	else
	{
		sprintf( buf1,"" );
	}

	CSize siz1 = dcMemory.GetTextExtent( buf1 );
	dcMemory.TextOut( rect.right - siz1.cx - 10, 85, buf1 ); // print sps
	// percent remain
	if( sps )
	{
		//sps = ((GroupData.info.GetDataPos-1)*1000)/(cur_time-start_time);
		DWORD remain = (DWORD)((TOTAL_SAMPLE_NUM - GroupData.info.GetDataPos)/sps);

		if( remain > 20000*3600 )
		{
			remain = 20000*3600;
			sprintf( buf2,"%.1f%c (>%dh%d'%d\")", 100.*GroupData.info.GetDataPos/(float)TOTAL_SAMPLE_NUM,  '%',
				remain/3600,
				(remain%3600)/60,
				(remain%3600)%60 );
		}
		else
		{
			sprintf( buf2,"%.1f%c (%dh%d'%d\")", 100.*GroupData.info.GetDataPos/(float)TOTAL_SAMPLE_NUM,  '%',
				remain/3600,
				(remain%3600)/60,
				(remain%3600)%60 );
		}
	}
	else
	{
		//sprintf( buf2,"%.1f%c", 100.*GroupData.info.GetDataPos/(float)TOTAL_SAMPLE_NUM,  '%');
		sprintf( buf2,"");
	}

	siz1 = dcMemory.GetTextExtent( buf2 );
	//dcMemory.TextOut( rect.right - siz1.cx - 10, 85, buf1 );
	dcMemory.TextOut( rect.right - siz1.cx - 10, 85 + siz1.cy, buf2 );

	dcMemory.SetTextColor( SELECT_POS1_COLOR );
	siz1 = dcMemory.GetTextExtent( Select_str_Pos1 );
	dcMemory.TextOut( rect.right - siz1.cx - 10, 85 + 2*siz1.cy, Select_str_Pos1 );

	dcMemory.SetTextColor( SELECT_POS2_COLOR );
	siz1 = dcMemory.GetTextExtent( Select_str_Pos2 );
	dcMemory.TextOut( rect.right - siz1.cx - 10, 85 + 3*siz1.cy, Select_str_Pos2 ); //tmpx

	if( Select_str_Pos1 != "" && Select_str_Pos2 != "" )
	{
		dcMemory.SetTextColor( RGB(0x80, 0x00, 0x80) );
		sprintf( buf1,"△%.3fV @ %.3f\"", Select_Vol2 - Select_Vol1, Select_Pos2 - Select_Pos1 );
		CSize sizd = dcMemory.GetTextExtent( buf1 );
		dcMemory.TextOut( rect.right - sizd.cx - 10, 85 + 4*siz1.cy, buf1 );
	}

	dcMemory.SelectObject(Old);
}

void CVolMointorDlg::OnEnKillfocusEditVol()
{
	// TODO: Add your control notification handler code here
	CString str;
	m_edit_vol_str.GetWindowText(str);
	int aaa = atoi(str);
	ref__vol = aaa;
}

void CVolMointorDlg::OnBnClickedHelp1()
{
	// TODO: Add your control notification handler code here
	CString prompt = "\
1. 在波形显示区域鼠标滚动键可以调节一格显示的时间范围；左键和右键点击可以显示两个十字坐标轴，字母按键'H''V'可以打开关闭横纵坐标轴，和=||按钮功能一样。\r\n\
2. 在波形显示区域鼠标可以拖动调整显示区域。\r\n\
3. 在电压区域鼠标滚动键可以调节显示电压范围，按下Ctrl可以加速；鼠标可以拖动显示区域。\r\n\
4. 在时间轴鼠标滚动键可以调节时间轴格数；鼠标可以拖动显示区域。\r\n\
5. 在左下角鼠标左键点击可以复位显示区域。\r\n\
6. 有任何问题发邮件给gr_li@163.com咨询，或者淘宝咨询也可以。不实时守店，答复不及时，所以尽量把问题或想法一次性描述完。\r\n\
1. The mouse scroll keys in the waveform display area can adjust the time range displayed in one grid; Left and right clicking can display two cross coordinate axes, and the letter keys' H '' V 'can turn on and off the horizontal and vertical coordinate axes, just like the=| | button function. \r\n\
2. In the waveform display area, the mouse can be dragged to adjust the display area. \r\n\
3. The mouse scroll keys in the voltage area can adjust the displayed voltage range, and pressing Ctrl can accelerate; The mouse can drag the display area. \r\n\
4. The mouse scroll keys on the timeline can adjust the number of grids on the timeline; The mouse can drag the display area. \r\n\
5. Click the left mouse button in the bottom left corner to reset the display area. \r\n\
6. If you have any questions, please send an email to gr_li@163.com Consultation or Taobao consultation is also available. Not keeping track of the store in real time and not responding promptly, so try to describe the problem or idea in one go.\
";
	 AfxMessageBox(prompt);
}
