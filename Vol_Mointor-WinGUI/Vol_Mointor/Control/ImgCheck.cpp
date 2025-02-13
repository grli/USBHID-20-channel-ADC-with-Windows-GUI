#include "stdafx.h"
#include "CMemDC.h"
#include "ImgCheck.h"
#include ".\ImgCheck.h"

BEGIN_MESSAGE_MAP(CImageCheckBox, CButton)
	//{{AFX_MSG_MAP(CDerEditDlg)
    ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CImageCheckBox::CImageCheckBox()
{
    m_UserColor = FALSE;
}

void CImageCheckBox::SetTextColor( COLORREF color )
{
    m_UserColor = TRUE;
    m_TextColor = color;
}

BOOL CImageCheckBox::OnEraseBkgnd( CDC* pDC )
{
    return TRUE;
    // return CButton::OnEraseBkgnd(pDC);
}

void CImageCheckBox::InitImage( const char *pcNormal, const char *pcCheck )
{
    blCheck = FALSE;

    m_imgNormal.Load( pcNormal );
    m_imgCheck.Load( pcCheck );
    m_imgBack.Load( "ground.bmp" );

#if _DEBUG
    if( m_imgNormal.IsNull() && pcNormal != "" )
    {
        MessageBox( pcNormal );
    }
    if( m_imgCheck.IsNull() && pcCheck != "" )
    {
        MessageBox( pcCheck );
    }
    if( m_imgBack.IsNull() )
    {
        MessageBox( "ground.bmp" );
    }
#endif
}

BOOL CImageCheckBox::GetCheck()
{
    return blCheck;
}

void CImageCheckBox::SetCheck( BOOL state )
{
    if( state == TRUE )
    {
        blCheck = TRUE;
    }
    else
    {
        blCheck = FALSE;
    }

    RedrawWindow( NULL, NULL, RDW_INVALIDATE );
}

void CImageCheckBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    CWindowDC dc(this);
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

    DrawItemImg(lpDIS);
}

void CImageCheckBox::DrawItemImg(LPDRAWITEMSTRUCT lpDIS)
{
    CWindowDC dc(this);
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

    RECT rect = lpDIS->rcItem;
    rect.right = rect.bottom;

	CBitmap  *m_bitmap;
	m_bitmap = new CBitmap;
	m_bitmap->CreateCompatibleBitmap(pDC, lpDIS->rcItem.right, lpDIS->rcItem.bottom);

    CDC mem;
    mem.CreateCompatibleDC(pDC);
    mem.SelectObject(m_bitmap);
    m_imgBack.StretchBlt( mem, lpDIS->rcItem, SRCCOPY );

	pDC->BitBlt(0,0,100,100,&mem,0,0, SRCCOPY );
    delete m_bitmap;

    if( GetCheck() == TRUE )
    {
        if( !m_imgCheck.IsNull() )
        {
            m_imgCheck.StretchBlt( dc, rect, SRCCOPY);
        }
    }
    else
    {
        if( !m_imgNormal.IsNull() )
        {
            m_imgNormal.StretchBlt( dc, rect, SRCCOPY);
        }
    }

#if BUTTON_FRAME_SIGN
    if ((lpDIS->itemAction | ODA_FOCUS)
        && (lpDIS->itemState & ODS_FOCUS))
    {
		CBrush br( GetSysColor(COLOR_HIGHLIGHT) );
        dc.FrameRect(&lpDIS->rcItem, &br);
    }
#endif

    CString str;
    GetWindowText( str );
    dc.SetBkMode(TRANSPARENT);

    if( m_UserColor == TRUE )
    {
        dc.SetTextColor( m_TextColor );
    }
    else
    {
        dc.SetTextColor( GetSysColor(COLOR_WINDOWTEXT) );
    }

    CFont font, *pfont;
    pfont = GetFont();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    pfont->GetLogFont( &lf );
    lf.lfHeight = CHECK_FONT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = dc.SelectObject(&font);
    dc.TextOut( lpDIS->rcItem.left + CHECK_STRING_DIFF_X, lpDIS->rcItem.top + CHECK_STRING_DIFF_Y, str );
    dc.SelectObject(def_font);
    font.DeleteObject();
}

void CImageCheckBox::GetClientRect( LPRECT lpRect )
{
    CPaintDC dc(this);

    CFont font, *pfont;
    pfont = GetFont();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    pfont->GetLogFont( &lf );
    lf.lfHeight = CHECK_FONT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = dc.SelectObject(&font);

    CSize sz;
    CString str;
    GetWindowText( str );

    sz = dc.GetTextExtent( str );

    lpRect->left = 0;
    lpRect->top = 0;
    lpRect->right = CHECK_STRING_DIFF_X + sz.cx + 3;
    lpRect->bottom = CHECK_IMAGE_SIZE + 1;

    dc.SelectObject(def_font);
    font.DeleteObject();
}

void CImageCheckBox::SetWindowPos( int x, int y )
{
    RECT rect;
    GetClientRect( &rect );
    CButton::SetWindowPos( NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOMOVE | SWP_NOZORDER );
    CButton::SetWindowPos( NULL, x, y, x+rect.right, y+rect.bottom, SWP_NOSIZE | SWP_NOZORDER );
}

void CImageCheckBox::ReDrawItem( CMemDC *pdc, int diff_x, int diff_y )
{
    RECT rect;
    GetClientRect( &rect );
    rect.right = rect.bottom;

    rect.left+=diff_x;
    rect.right+=diff_x;
    rect.top+=diff_y;
    rect.bottom+=diff_y;

    if( GetCheck() == TRUE )
    {
        if( !m_imgCheck.IsNull() )
        {
            m_imgCheck.StretchBlt( *pdc, rect, SRCCOPY);
        }
    }
    else
    {
        if( !m_imgNormal.IsNull() )
        {
            m_imgNormal.StretchBlt( *pdc, rect, SRCCOPY);
        }
    }

#if BUTTON_FRAME_SIGN
    if( CButton::GetState() & ODS_FOCUS )
    {
        GetClientRect( &rect );
		CBrush br( GetSysColor(COLOR_HIGHLIGHT) );
        pdc->FrameRect( &rect, &br);
    }
#endif

    CString str;
    GetWindowText( str );

    pdc->SetBkMode(TRANSPARENT);
    if( m_UserColor == TRUE )
    {
        pdc->SetTextColor( m_TextColor );
    }
    else
    {
        pdc->SetTextColor( GetSysColor(COLOR_WINDOWTEXT) );
    }

    CFont font, *pfont;
    pfont = GetFont();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    pfont->GetLogFont( &lf );
    lf.lfHeight = CHECK_FONT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = pdc->SelectObject(&font);
    pdc->TextOut( rect.left + CHECK_STRING_DIFF_X, rect.top + CHECK_STRING_DIFF_Y, str );
    pdc->SelectObject(def_font);
    font.DeleteObject();
}

