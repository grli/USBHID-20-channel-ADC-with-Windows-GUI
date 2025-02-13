#pragma once
#include "CMemDC.h"

#define TAB_IMAGE_SIZE      26
#define TAB_STRING_DIFF_X   ( TAB_IMAGE_SIZE )
#define TAB_STRING_DIFF_Y   6

#define TAB_FRAME_SIGN      1
#define IMG_TAB_TEXT_HEIGHT 14

class CImageTAB : public CButton
{
private:
    CUserImage m_imgNormal, m_imgCheck, m_imgTitle;
    BOOL blCheck;

    COLORREF m_TextColor;
    BOOL m_UserColor;

public:
    CImageTAB();
    void InitImage( const char *pcNormal, const char *pcCheck, const char *pcTitle );
    void DrawItemImg(LPDRAWITEMSTRUCT lpDIS);
    void ReDrawItem( CMemDC *pdc, int diff_x, int diff_y );
    BOOL GetCheck();
    void SetCheck( BOOL state );
    void GetClientRect( LPRECT lpRect );
    void SetTextColor( COLORREF color );

protected:
    DECLARE_MESSAGE_MAP()

public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
};

