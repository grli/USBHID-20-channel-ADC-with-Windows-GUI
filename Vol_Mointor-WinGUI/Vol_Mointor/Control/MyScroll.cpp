// MyScroll.cpp : ʵ���ļ�
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



// CMyScroll ��Ϣ�������


void CMyScroll::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if (m_VTMember.m_bDrag == true)
    {
        m_VTMember.m_bDrag = false;
    }
    CScrollBar::OnLButtonUp(nFlags, point);
}
