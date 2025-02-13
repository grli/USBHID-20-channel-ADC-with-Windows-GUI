#include "stdafx.h"
#include "CMemDC.h"
#include "imgbutton.h"

BEGIN_MESSAGE_MAP(CImgButton, CButton)
	//{{AFX_MSG_MAP(CDerEditDlg)
	ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CImgButton::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default
    return TRUE;
    //return CButton::OnEraseBkgnd(pDC);
}

CImgButton::CImgButton()
{
    m_diff_y = 0;
    m_imgDiff_X = 0;
    m_UserColor = FALSE;
    bimgEraseFrame = FALSE;

	m_imgNormal = NULL;
	m_imgDisable = NULL;
	m_imgFocus = NULL;
	m_imgPress = NULL;
	m_imgBackGround = NULL;
}

void CImgButton::InitImage( COLORREF *pcNormal, COLORREF *pcDisable, COLORREF *pcFocus, COLORREF *pcPress )
{
	if(m_Tooltip.m_hWnd == NULL )
    {
        m_Tooltip.Create(this);
        m_Tooltip.Activate(TRUE);
    }

    m_imgNormal = pcNormal;
    m_imgDisable = pcDisable;
    m_imgFocus = pcFocus;
    m_imgPress = pcPress;

    CString str;
    GetWindowText( str );

    CWindowDC dc(this);

    CFont font, *pfont;
    pfont = GetFont();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    pfont->GetLogFont( &lf );
    lf.lfHeight = IMG_BUTTON_TEXT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = dc.SelectObject(&font);
    CSize cs = dc.GetTextExtent( str );
    dc.SelectObject(def_font);
    font.DeleteObject();

    if( m_imgNormal != NULL )
    {
        if( (LONG)m_imgNormal[0] < cs.cx )
        {
            SetWindowPos( NULL, 0, 0, cs.cx, m_imgNormal[1], SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE );
            m_imgDiff_X = ( cs.cx - m_imgNormal[0] )/2;
        }
        else
        {
            SetWindowPos( NULL, 0, 0, m_imgNormal[0], m_imgNormal[1], SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE );
            m_imgDiff_X = 0;
        }
    }
}

void CImgButton::SetTextColor( COLORREF color )
{
    m_UserColor = TRUE;
    m_TextColor = color;
}

void CImgButton::SetWindowText( int diff_y, char *str )
{
    if( str == "" )
    {
        m_diff_y = -1;
    }
    else
    {
        m_diff_y = diff_y;
    }

    CButton::SetWindowText( str );

    CWindowDC dc(this);

    CFont font, *pfont;
    pfont = GetFont();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    pfont->GetLogFont( &lf );
    lf.lfHeight = IMG_BUTTON_TEXT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = dc.SelectObject(&font);
    CSize cs = dc.GetTextExtent( str );
    dc.SelectObject(def_font);
    font.DeleteObject();

	if( m_imgNormal != NULL )
    {
        if( (LONG)m_imgNormal[0] < cs.cx )
        {
            SetWindowPos( NULL, 0, 0, cs.cx, m_imgNormal[1], SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE );
            m_imgDiff_X = ( cs.cx - m_imgNormal[0] )/2;
        }
        else
        {
            SetWindowPos( NULL, 0, 0, m_imgNormal[0], m_imgNormal[1], SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE );
            m_imgDiff_X = 0;
        }
    }
//    if( *str != '\0'&& m_Tooltip.m_hWnd != NULL)
//    {
//        m_Tooltip.AddTool(this,str);
//    }
}

void CImgButton::SetWindowText( char *str )
{
    m_diff_y = -1;

	CString cstr;
	cstr = str;
    CButton::SetWindowText( cstr );
//    if( *str != '\0'&& m_Tooltip.m_hWnd != NULL)
//    {
//        m_Tooltip.AddTool(this,str);
//    }
}

void DrawMemory( CMemDC *dcMemory, int x, int y, COLORREF *pixel )
{
	if( pixel == NULL )
	{
		return;
	}

	for( int h = 0; h < (int)pixel[ 1 ]; h++ )
	{
		for( int w = 0; w < (int)pixel[ 0 ]; w++ )
		{
			//if( pixel[ h*pixel[ 0 ] + w ] != 0xffffff )
			dcMemory->SetPixel( x + w, y + h, pixel[ h*pixel[ 0 ] + w ] );
		}
	}

	dcMemory->SetPixel( 0, 0, pixel[ 2 ] );
	dcMemory->SetPixel( 1, 0, pixel[ 2 ] );
}

void DrawMemory1( CMemDC *dcMemory, int x, int y, COLORREF *pixel )
{
	if( pixel == NULL )
	{
		return;
	}

	for( int h = 0; h < (int)pixel[ 1 ]; h++ )
	{
		for( int w = 0; w < (int)pixel[ 0 ]; w++ )
		{
			dcMemory->SetPixel( x + w, y + h, pixel[ 2 ] );
		}
	}
}

extern COLORREF img_ground[];

void CImgButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    CWindowDC dc(this);
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);
    CMemDC dcMemory( pDC, lpDIS->rcItem );

	m_imgBackGround = img_ground;
	DrawMemory1( &dcMemory, lpDIS->rcItem.left, lpDIS->rcItem.top, m_imgBackGround );

    if( lpDIS->itemState & ODS_SELECTED )
    {
        if( m_imgPress )
        {
			DrawMemory( &dcMemory, lpDIS->rcItem.left + m_imgDiff_X, lpDIS->rcItem.top, m_imgPress );
        }
    }
    else if ( lpDIS->itemState & ODS_FOCUS )
    {
        if( m_imgFocus )
        {
			DrawMemory( &dcMemory, lpDIS->rcItem.left + m_imgDiff_X, lpDIS->rcItem.top, m_imgFocus );
        }
        else
        {
            if( m_imgNormal )
            {
				DrawMemory( &dcMemory, lpDIS->rcItem.left + m_imgDiff_X, lpDIS->rcItem.top, m_imgNormal );
            }
        }
    }
    else if ( lpDIS->itemState & ODS_DISABLED )
    {
        if( m_imgDisable )
        {
			DrawMemory( &dcMemory, lpDIS->rcItem.left + m_imgDiff_X, lpDIS->rcItem.top, m_imgDisable );
        }
    }
    else
    {
        if( m_imgNormal )
        {
			DrawMemory( &dcMemory, lpDIS->rcItem.left + m_imgDiff_X, lpDIS->rcItem.top, m_imgNormal );
        }
    }

    if( lpDIS->itemState & ODS_SELECTED )
    {
    }
    else
    {
        if ((lpDIS->itemAction | ODA_FOCUS)
            && (lpDIS->itemState & ODS_FOCUS))
        {
            if( bimgEraseFrame == FALSE )
            {
		        CBrush br( GetSysColor(COLOR_HIGHLIGHT) );
                RECT tmp;
                tmp.left = lpDIS->rcItem.left + 2;
                tmp.top = lpDIS->rcItem.top + 2;
                tmp.right = lpDIS->rcItem.right -2;
                tmp.bottom = lpDIS->rcItem.bottom - 2;
                dcMemory.FrameRect(&tmp, &br);
            }
        }
    }

    CString str;
    CButton::GetWindowText( str );

    if( str.IsEmpty() == FALSE )
    {
        dcMemory.SetBkMode( TRANSPARENT );
        if( m_UserColor == TRUE )
        {
            dcMemory.SetTextColor( m_TextColor );
        }

        CFont font, *pfont;
        pfont = GetFont();

        LOGFONT lf;
        memset(&lf, 0, sizeof(LOGFONT));
        pfont->GetLogFont( &lf );

        lf.lfHeight = IMG_BUTTON_TEXT_HEIGHT;
        VERIFY(font.CreateFontIndirect(&lf));
        CFont *def_font = dcMemory.SelectObject(&font);

        CSize cs = dcMemory.GetTextExtent( str );
        int mid = lpDIS->rcItem.right - cs.cx;
        if( mid < 0 )
        {
            mid = 0;
        }

        if( m_diff_y >= 0 )
        {
            dcMemory.TextOut( lpDIS->rcItem.left + mid/2,
                lpDIS->rcItem.top + m_diff_y, str );
        }
        else
        {
            dcMemory.TextOut( lpDIS->rcItem.left + mid/2,
                lpDIS->rcItem.top + (lpDIS->rcItem.bottom - cs.cy)/2, str );
        }

        dcMemory.SelectObject(def_font);
        font.DeleteObject();
    }
}

void CImgButton::ReDrawItem( CMemDC *pdc, int diff_x, int diff_y )
{
    if( CButton::IsWindowVisible() == FALSE )
    {
        return;
    }

    if( m_imgNormal == NULL )
    {
        return;
    }

    RECT rect;
    GetClientRect( &rect );

    rect.left+=diff_x;
    rect.right+=diff_x;
    rect.top+=diff_y;
    rect.bottom+=diff_y;

    if( CButton::GetState() & BST_FOCUS )
    {
        if( m_imgFocus )
        {
//            m_imgFocus.BitBlt( *pdc, rect.left + m_imgDiff_X, rect.top, SRCCOPY);
			DrawMemory( pdc, rect.left + m_imgDiff_X, rect.top, m_imgNormal );
        }
    }
    else
    {
        if( CButton::IsWindowEnabled() == FALSE )
        {
            if( m_imgDisable )
            {
 //               m_imgDisable.BitBlt( *pdc, rect.left + m_imgDiff_X, rect.top, SRCCOPY);
				DrawMemory( pdc, rect.left + m_imgDiff_X, rect.top, m_imgDisable );
            }
        }
        else
        {
            if( m_imgNormal )
            {
 //               m_imgNormal.BitBlt( *pdc, rect.left + m_imgDiff_X, rect.top, SRCCOPY);.
				DrawMemory( pdc, rect.left + m_imgDiff_X, rect.top, m_imgNormal );
            }
        }
    }

    CString str;
    CButton::GetWindowText( str );
    if( str.IsEmpty() == FALSE )
    {
        GetClientRect( &rect );

        pdc->SetBkMode( TRANSPARENT );
        if( m_UserColor == TRUE )
        {
            pdc->SetTextColor( m_TextColor );
        }

        CFont font, *pfont;
        pfont = GetFont();

        LOGFONT lf;
        memset(&lf, 0, sizeof(LOGFONT));
        pfont->GetLogFont( &lf );

        lf.lfHeight = IMG_BUTTON_TEXT_HEIGHT;
        VERIFY(font.CreateFontIndirect(&lf));
        CFont *def_font = pdc->SelectObject(&font);

        CSize cs = pdc->GetTextExtent( str );
        int mid = rect.right - cs.cx;
        if( mid < 0 )
        {
            mid = 0;
        }

        if( m_diff_y >= 0 )
        {
            pdc->TextOut( rect.left + mid/2 + diff_x,
                rect.top + diff_y + m_diff_y, str );
        }
        else
        {
            pdc->TextOut( rect.left + mid/2 + diff_x,
                rect.top + diff_y + (rect.bottom - cs.cy)/2, str );
        }

        pdc->SelectObject(def_font);
        font.DeleteObject();
    }
}

void CImgButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch( nChar )
    {
        case VK_SPACE:
        case VK_RETURN:
            SendMessage( WM_LBUTTONDOWN, 0, 0 );
            SendMessage( WM_LBUTTONUP, 0, 0 );
            break;

        default:
        	::SendMessage( GetParent()->m_hWnd, WM_KEYDOWN, nChar, nRepCnt);
            break;
    }
}

void CImgButton::EaseBtnFrame( BOOL isEraseFrame )
{
    bimgEraseFrame = isEraseFrame;
}

void CImgButton::SetBtnTip(LPCTSTR lpszTip)
{
    if( m_Tooltip.m_hWnd != NULL)
    {
        m_Tooltip.AddTool(this, lpszTip);
    }
}

BOOL CImgButton::PreTranslateMessage(MSG* pMsg)
{
    if(m_Tooltip.m_hWnd != NULL)
    {
        switch( pMsg->message )
        {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MOUSEMOVE:
                m_Tooltip.RelayEvent(pMsg);
                break;
            default:
                break;
        }
    }
    return CButton::PreTranslateMessage(pMsg);
}


