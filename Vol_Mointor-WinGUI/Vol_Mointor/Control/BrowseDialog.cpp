#include "stdafx.h"
#include "BrowseDialog.h"
#include "shlobj.h"

static int __stdcall BrowseCtrlCallback( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    CBrowseDialog* pBrowseDialogObj = (CBrowseDialog*)lpData;
    if( uMsg == BFFM_INITIALIZED
            && !pBrowseDialogObj->m_SelDir.IsEmpty() )
    {
        ::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)(LPCTSTR)(pBrowseDialogObj->m_SelDir));
    }

    return 0;
}

CBrowseDialog::CBrowseDialog()
{
    m_Path = "";
    m_InitDir = "";
    m_SelDir = "";
    m_Title = "";
}

CBrowseDialog::~CBrowseDialog()
{
}

BOOL CBrowseDialog::DoModal()
{
    LPMALLOC pMalloc;
    if (SHGetMalloc (&pMalloc)!= NOERROR)
    {
        return FALSE;
    }

    BROWSEINFO bInfo;
    LPITEMIDLIST pidl;
    ZeroMemory ( (PVOID) &bInfo,sizeof (BROWSEINFO));

    if (!m_InitDir.IsEmpty ())
    {
        OLECHAR olePath[MAX_PATH];
        ULONG chEaten;
        ULONG dwAttributes;
        HRESULT hr;
        LPSHELLFOLDER pDesktopFolder;

        if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
        {
            MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED, m_InitDir.GetBuffer(MAX_PATH), -1,
                olePath, MAX_PATH);

            m_InitDir.ReleaseBuffer (-1);
            hr = pDesktopFolder->ParseDisplayName(NULL,NULL,olePath,&chEaten,&pidl,&dwAttributes);
            if(FAILED(hr))
            {
                pMalloc ->Free (pidl);
                pMalloc ->Release ();
                return FALSE;
            }

            bInfo.pidlRoot = pidl;
        }
    }

    bInfo.hwndOwner = NULL;
    bInfo.pszDisplayName = m_Path.GetBuffer (MAX_PATH);
    bInfo.lpszTitle = m_Title;
    bInfo.ulFlags = BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS;
    bInfo.lpfn = BrowseCtrlCallback;
    bInfo.lParam = (LPARAM)this;
    if( (pidl = ::SHBrowseForFolder(&bInfo)) == NULL )
    {
        return FALSE;
    }

    m_Path.ReleaseBuffer();
    if( ::SHGetPathFromIDList(pidl,m_Path.GetBuffer(MAX_PATH)) == FALSE )
    {
        pMalloc->Free(pidl);
        pMalloc->Release();
        return FALSE;
    }

    m_Path.ReleaseBuffer();
    pMalloc->Free(pidl);
    pMalloc->Release();
    return TRUE;
}

