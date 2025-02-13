#pragma once
#include "CMemDC.h"

#define CHECK_FONT_HEIGHT    13
#define CHECK_IMAGE_SIZE     15
#define CHECK_STRING_DIFF_X  ( CHECK_IMAGE_SIZE + 3 )
#define CHECK_STRING_DIFF_Y  ( CHECK_IMAGE_SIZE + 1 - CHECK_FONT_HEIGHT )/2

#define BUTTON_FRAME_SIGN    1

class CImageCheckBox : public CButton
{
private:
	CUserImage m_imgNormal, m_imgCheck, m_imgBack;
    BOOL blCheck;

    COLORREF m_TextColor;
    BOOL m_UserColor;

public:
    CImageCheckBox();
    void InitImage( const char *pcNormal, const char *pcCheck );
    void DrawItemImg(LPDRAWITEMSTRUCT lpDIS);
    void ReDrawItem( CMemDC *pdc, int diff_x, int diff_y );
    void GetClientRect( LPRECT lpRect );
    void SetWindowPos( int x, int y );
    BOOL GetCheck();
    void SetCheck( BOOL state );
    void SetTextColor( COLORREF color );

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
};

