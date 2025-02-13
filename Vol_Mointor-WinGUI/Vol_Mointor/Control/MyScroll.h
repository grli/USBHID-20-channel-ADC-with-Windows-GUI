#pragma once


// CMyScroll

class CMyScroll : public CScrollBar
{
	DECLARE_DYNAMIC(CMyScroll)

public:
	CMyScroll();
	virtual ~CMyScroll();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


