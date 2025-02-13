#include "stdafx.h"
#include "CMemDC.h"
#include "ImgTAB.h"
#include ".\ImgTAB.h"

BEGIN_MESSAGE_MAP(CImageTAB, CButton)
	//{{AFX_MSG_MAP(CDerEditDlg)
    ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CImageTAB::CImageTAB()
{
    m_UserColor = FALSE;
    blCheck = FALSE;
}

BOOL CImageTAB::OnEraseBkgnd( CDC* pDC )
{
    return TRUE;
    // return CButton::OnEraseBkgnd(pDC);
}

void CImageTAB::InitImage( const char *pcNormal, const char *pcCheck, const char *pcTitle )
{
    m_imgNormal.Load( pcNormal );
    m_imgCheck.Load( pcCheck );
    m_imgTitle.Load( pcTitle );

    if( !m_imgNormal.IsNull() )
    {
        SetWindowPos( NULL, 0,0, m_imgNormal.GetWidth(), m_imgNormal.GetHeight(), SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE );
    }

#if _DEBUG
    if( m_imgNormal.IsNull() && pcNormal != "" )
    {
        MessageBox( pcNormal );
    }
    if( m_imgCheck.IsNull() && pcCheck != "" )
    {
        MessageBox( pcCheck );
    }
    if( m_imgTitle.IsNull() && pcTitle != "" )
    {
        MessageBox( pcTitle );
    }
#endif
}

void CImageTAB::SetTextColor( COLORREF color )
{
    m_UserColor = TRUE;
    m_TextColor = color;
}

BOOL CImageTAB::GetCheck()
{
    return blCheck;
}

void CImageTAB::SetCheck( BOOL state )
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

void CImageTAB::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    CWindowDC dc(this);
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

    DrawItemImg(lpDIS);
}

void CImageTAB::DrawItemImg(LPDRAWITEMSTRUCT lpDIS)
{
    CWindowDC dc(this);
	CDC *pDC = CDC::FromHandle(lpDIS->hDC);

    RECT rect = lpDIS->rcItem;

    if ((lpDIS->itemAction | ODA_FOCUS)
        && (lpDIS->itemState & ODS_FOCUS))
    {
        if( blCheck != TRUE )
        {
            PostMessage( WM_LBUTTONDOWN, 0, ( rect.top << 16 ) | rect.left );
            PostMessage( WM_LBUTTONUP, 0, ( rect.top << 16 ) | rect.left );
        }
    }

    if( GetCheck() == TRUE )
    {
        if( !m_imgCheck.IsNull() )
        {
            m_imgCheck.BitBlt( dc, rect.left, rect.top, SRCCOPY);
        }

        if( !m_imgTitle.IsNull() )
        {
            dc.TransparentBlt(0, 3, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(),
                CDC::FromHandle(m_imgTitle.GetDC()),0,0,m_imgTitle.GetWidth(),
                m_imgTitle.GetHeight(), RGB(255,255,255) );

            m_imgTitle.ReleaseDC();
        }
    }
    else
    {
        if( !m_imgNormal.IsNull() )
        {
            m_imgNormal.BitBlt( dc, rect.left, rect.top, SRCCOPY);
        }

        if( !m_imgTitle.IsNull() )
        {
            dc.TransparentBlt(0, 0, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(),
                CDC::FromHandle(m_imgTitle.GetDC()),0,0,m_imgTitle.GetWidth(),
                m_imgTitle.GetHeight(), RGB(255,255,255) );

            m_imgTitle.ReleaseDC();
        }
    }

    if( !m_imgTitle.IsNull() )
    {
        //m_imgTitle.BitBlt( dc, 0,0,SRCCOPY );
    }

    if ((lpDIS->itemAction | ODA_FOCUS)
        && (lpDIS->itemState & ODS_FOCUS))
    {
#if TAB_FRAME_SIGN
		CBrush br( GetSysColor(COLOR_HIGHLIGHT) );
        lpDIS->rcItem.left += 6;
        lpDIS->rcItem.top  += 4;
        lpDIS->rcItem.right -= 6;
        lpDIS->rcItem.bottom -= 4;
        dc.FrameRect(&lpDIS->rcItem, &br);
#endif
    }
    else
    {
        if( GetCheck() == TRUE )
        {
            lpDIS->rcItem.left += 6;
            lpDIS->rcItem.top  += 4;
            lpDIS->rcItem.right -= 6;
            lpDIS->rcItem.bottom -= 4;
        }
    }

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
    lf.lfHeight = IMG_TAB_TEXT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = dc.SelectObject(&font);
    CSize cs = dc.GetTextExtent( str );
    dc.TextOut( lpDIS->rcItem.left + TAB_STRING_DIFF_X, lpDIS->rcItem.top + TAB_STRING_DIFF_Y, str );
    dc.SelectObject(def_font);
    font.DeleteObject();
}

void CImageTAB::ReDrawItem( CMemDC *pdc, int diff_x, int diff_y )
{
    RECT rect;
    GetClientRect( &rect );

    rect.left+=diff_x;
    rect.right+=diff_x;
    rect.top+=diff_y;
    rect.bottom+=diff_y;

    if( GetCheck() == TRUE )
    {
        if( !m_imgCheck.IsNull() )
        {
            m_imgCheck.BitBlt( *pdc, rect.left, rect.top, SRCCOPY);
        }
    }
    else
    {
        if( !m_imgNormal.IsNull() )
        {
            m_imgNormal.BitBlt( *pdc, rect.left, rect.top, SRCCOPY);
        }
    }

    if( !m_imgTitle.IsNull() )
    {
	    CBitmap  *m_bitmap;
	    m_bitmap = new CBitmap;
	    m_bitmap->CreateCompatibleBitmap( pdc, rect.right, rect.bottom );

        CDC mem;
        mem.CreateCompatibleDC( pdc );
        mem.SelectObject( m_bitmap );
        m_imgTitle.BitBlt( mem, rect.left, rect.top, SRCCOPY );

        if( GetCheck() == TRUE )
        {
            pdc->TransparentBlt( rect.left, rect.top + 3, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(), &mem,
                0,0, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(), RGB( 255,255,255 ));
        }
        else
        {
            pdc->TransparentBlt( rect.left, rect.top, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(), &mem,
                0,0, m_imgTitle.GetWidth(), m_imgTitle.GetHeight(), RGB( 255,255,255 ));
        }

        delete m_bitmap;
    }

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
    lf.lfHeight = IMG_TAB_TEXT_HEIGHT;
    VERIFY(font.CreateFontIndirect(&lf));
    CFont *def_font = pdc->SelectObject(&font);
    pdc->TextOut( rect.left + TAB_STRING_DIFF_X, rect.top + TAB_STRING_DIFF_Y, str );
    pdc->SelectObject(def_font);
    font.DeleteObject();
}

void CImageTAB::GetClientRect( LPRECT lpRect )
{
    lpRect->left = 0;
    lpRect->top = 0;

    if( !m_imgNormal.IsNull() )
    {
        lpRect->bottom = m_imgNormal.GetHeight();
        lpRect->right = m_imgNormal.GetWidth();
    }

    //CPaintDC dc(this);
    //CSize sz;

    //CString str;
    //GetWindowText( str );

    //sz = dc.GetTextExtent( str );

    //lpRect->left = 0;
    //lpRect->top = 0;
    //lpRect->right = sz.cx + 5;

    //if( !m_imgTitle.IsNull() )
    //{
    //    lpRect->right += m_imgTitle.GetWidth();
    //}

    //if( !m_imgNormal.IsNull() )
    //{
    //    lpRect->bottom = m_imgNormal.GetHeight();
    //}
    //else
    //{
    //    lpRect->bottom = lpRect->right;
    //}
}

BOOL CImageTAB::PreTranslateMessage(MSG* pMsg)
{
    // TODO: 在此添加专用代码和/或调用基类
    if( pMsg->message == WM_KEYDOWN )
    {
        switch(pMsg->wParam)
        {
            case VK_TAB:
                ::PostMessage( GetParent()->m_hWnd, WM_SETSUBFOCUS, 0, 0 );
                return true;
                break;

            case VK_LEFT:
                ::PostMessage( GetParent()->m_hWnd, WM_TAB_LEFT, 0, 0 );
                return true;
                break;

            case VK_RIGHT:
                ::PostMessage( GetParent()->m_hWnd, WM_TAB_RIGHT, 0, 0 );
                return true;
                break;

            case VK_ESCAPE:
                m_DialForm.CancelDialInfo();
                return true;
                break;
        }
    }

    return CButton::PreTranslateMessage( pMsg );
}

