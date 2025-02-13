#pragma once
#include "CMemDC.h"

#define IMG_BUTTON_TEXT_HEIGHT 12

class CImgButton : public CButton
{
private:
    int m_diff_y;
    int m_imgDiff_X;

	COLORREF *m_imgNormal, *m_imgDisable, *m_imgFocus, *m_imgPress;
	COLORREF *m_imgBackGround;
    COLORREF m_TextColor;
    BOOL m_UserColor;
    BOOL bimgEraseFrame;
    CToolTipCtrl m_Tooltip;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
	CImgButton();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    void ReDrawItem( CMemDC *pdc, int diff_x, int diff_y );
	void InitImage( COLORREF *pcNormal, COLORREF *pcDisable, COLORREF *pcFocus, COLORREF *pcPress );
    void InitBackground( const char *pcBack );
    void SetTextColor( COLORREF color );
    void EaseBtnFrame( BOOL isEraseFrame );

    void SetWindowText( int diff_y, char *str );
    void SetWindowText( char *str );
    void SetBtnTip(LPCTSTR lpszTip);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

};

