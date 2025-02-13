// ClassChannelSetup.cpp : implementation file
//

#include "stdafx.h"
#include "VolMointor.h"
#include "ClassChannelSetup.h"

// CClassChannelSetup dialog

IMPLEMENT_DYNAMIC(CClassChannelSetup, CDialog)

CClassChannelSetup::CClassChannelSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CClassChannelSetup::IDD, pParent)
{
}

CClassChannelSetup::~CClassChannelSetup()
{
}

extern CString ChannelName[ BAT_NUM ];
extern BOOL blConnected;
extern int ChannelPos[ BAT_NUM ];

void CClassChannelSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
#if 0
	DDX_Control(pDX, IDC_EDIT_CH0, m_edit_ch[0]);
	DDX_Control(pDX, IDC_EDIT_CH1, m_edit_ch[1]);
	DDX_Control(pDX, IDC_EDIT_CH2, m_edit_ch[2]);
	DDX_Control(pDX, IDC_EDIT_CH3, m_edit_ch[3]);
	DDX_Control(pDX, IDC_EDIT_CH4, m_edit_ch[4]);
	DDX_Control(pDX, IDC_EDIT_CH5, m_edit_ch[5]);
	DDX_Control(pDX, IDC_EDIT_CH6, m_edit_ch[6]);
	DDX_Control(pDX, IDC_EDIT_CH7, m_edit_ch[7]);

	for( int i = 0; i < BAT_NUM; i++ )
	{
		m_edit_ch[ i ].SetWindowText( ChannelName[ i ] );
		

		DDX_Control(pDX, IDC_COMBO_POS_0 + i, m_ComboChannel_Pos[ i ]);
		DDX_Control(pDX, IDC_COMBO_NEG_0 + i, m_ComboChannel_Neg[ i ]);

		m_ComboChannel_Pos[ i ].AddString( "P1.0" );
		m_ComboChannel_Pos[ i ].AddString( "P1.1" );
		m_ComboChannel_Pos[ i ].AddString( "P1.2" );
		m_ComboChannel_Pos[ i ].AddString( "P1.3" );
		m_ComboChannel_Pos[ i ].AddString( "P1.4" );
		m_ComboChannel_Pos[ i ].AddString( "P1.5" );
		m_ComboChannel_Pos[ i ].AddString( "P1.6" );
		m_ComboChannel_Pos[ i ].AddString( "P1.7" );

		m_ComboChannel_Pos[ i ].AddString( "P2.0" );
		m_ComboChannel_Pos[ i ].AddString( "P2.1" );
		m_ComboChannel_Pos[ i ].AddString( "P2.2" );
		m_ComboChannel_Pos[ i ].AddString( "P2.3" );
		m_ComboChannel_Pos[ i ].AddString( "P2.4" );
		m_ComboChannel_Pos[ i ].AddString( "P2.5" );
		m_ComboChannel_Pos[ i ].AddString( "P2.6" );
		m_ComboChannel_Pos[ i ].AddString( "P2.7" );
		m_ComboChannel_Pos[ i ].AddString( "Sensor" );
		m_ComboChannel_Pos[ i ].AddString( "VDD" );

		switch( ChannelPos[ i ] )
		{
		case 0x1e:
			m_ComboChannel_Pos[ i ].SetCurSel( 16 );
		case 0x1f:
			m_ComboChannel_Pos[ i ].SetCurSel( 17 );
			break;
		default:
			m_ComboChannel_Pos[ i ].SetCurSel( ChannelPos[ i ] );
			break;
		}

		m_ComboChannel_Neg[ i ].AddString( "P1.0" );
		m_ComboChannel_Neg[ i ].AddString( "P1.1" );
		m_ComboChannel_Neg[ i ].AddString( "P1.2" );
		m_ComboChannel_Neg[ i ].AddString( "P1.3" );
		m_ComboChannel_Neg[ i ].AddString( "P1.4" );
		m_ComboChannel_Neg[ i ].AddString( "P1.5" );
		m_ComboChannel_Neg[ i ].AddString( "P1.6" );
		m_ComboChannel_Neg[ i ].AddString( "P1.7" );

		m_ComboChannel_Neg[ i ].AddString( "P2.0" );
		m_ComboChannel_Neg[ i ].AddString( "P2.1" );
		m_ComboChannel_Neg[ i ].AddString( "P2.2" );
		m_ComboChannel_Neg[ i ].AddString( "P2.3" );
		m_ComboChannel_Neg[ i ].AddString( "P2.4" );
		m_ComboChannel_Neg[ i ].AddString( "P2.5" );
		m_ComboChannel_Neg[ i ].AddString( "P2.6" );
		m_ComboChannel_Neg[ i ].AddString( "P2.7" );
		m_ComboChannel_Neg[ i ].AddString( "VREF" );
		m_ComboChannel_Neg[ i ].AddString( "GND" );

		switch( ChannelNeg[ i ] )
		{
		case 0x1e:
			m_ComboChannel_Neg[ i ].SetCurSel( 16 );
			break;
		case 0x1f:
			m_ComboChannel_Neg[ i ].SetCurSel( 17 );
			break;
		default:
			m_ComboChannel_Neg[ i ].SetCurSel( ChannelNeg[ i ] );
			break;
		}
	}
#endif
}


BEGIN_MESSAGE_MAP(CClassChannelSetup, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CClassChannelSetup::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CClassChannelSetup::OnBnClickedOk)
END_MESSAGE_MAP()


// CClassChannelSetup message handlers
void CClassChannelSetup::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CClassChannelSetup::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
#if 0	
	int tmp;

	for( int i = 0; i < BAT_NUM; i++ )
	{
		m_edit_ch[ i ].GetWindowText( ChannelName[ i ] );

		tmp = m_ComboChannel_Pos[ i ].GetCurSel();
		switch( tmp )
		{
		case 16:
			ChannelPos[ i ] = 0x1e;
		case 17:
			ChannelPos[ i ] = 0x1f;
			break;
		default:
			ChannelPos[ i ] = tmp;
			break;
		}

		tmp = m_ComboChannel_Neg[ i ].GetCurSel();
		switch( tmp )
		{
		case 16:
			ChannelNeg[ i ] = 0x1e;
		case 17:
			ChannelNeg[ i ] = 0x1f;
			break;
		default:
			ChannelNeg[ i ] = tmp;
			break;
		}
	}
#endif
	OnOK();
}
