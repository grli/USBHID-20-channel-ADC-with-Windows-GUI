// MyScroll.cpp : 实现文件
//

#include "stdafx.h"
#include "MyScroll.h"
#include ".\myscroll.h"


// CMyScroll

IMPLEMENT_DYNAMIC(CMyScroll, CScrollBar)
CMyScroll::CMyScroll()
{
}

CMyScroll::~CMyScroll()
{
}


BEGIN_MESSAGE_MAP(CMyScroll, CScrollBar)
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CMyScroll 消息处理程序


void CMyScroll::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (m_VTMember.m_bDrag == true)
    {
        m_VTMember.m_bDrag = false;
    }
    CScrollBar::OnLButtonUp(nFlags, point);
}
