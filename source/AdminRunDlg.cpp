
// AdminRunDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdminRunDlg.h"
#include "afxdialogex.h"


// CAdminRunDlg dialog



CAdminRunDlg::CAdminRunDlg(CAppDB *pDB, CWnd* pParent /*=NULL*/)
	: CDialog(CAdminRunDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_UAC_SMALL);
    m_pDB = pDB;
}

void CAdminRunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_APP, m_ctlListApp);
    DDX_Control(pDX, IDC_LIST_DIR, m_ctlListDir);
    DDX_Control(pDX, IDC_BROWSE_TARGETAPP, m_ctlBrowseApp);
    DDX_Control(pDX, IDC_BROWSE_TARGETDIR, m_ctlBrowseDir);
    DDX_Control(pDX, IDC_EDIT_ARGS, m_ctlEditArgs);
    DDX_Control(pDX, IDC_BTN_ADD, m_ctlBtnAdd);
    DDX_Control(pDX, IDC_BTN_APPLY, m_ctlBtnApply);
    DDX_Control(pDX, IDC_BTN_DELETE, m_ctlBtnDelete);
    DDX_Control(pDX, IDC_LABEL_APPNAME, m_ctlLabelName);
}

BEGIN_MESSAGE_MAP(CAdminRunDlg, CDialog)
    ON_LBN_SELCHANGE(IDC_LIST_APP, OnAppSelection)
    ON_BN_CLICKED(IDC_BTN_APPLY, OnAppApply)
    ON_BN_CLICKED(IDC_BTN_DELETE, OnAppDelete)
    ON_BN_CLICKED(IDC_BTN_ADD, OnAppAdd)
    ON_EN_CHANGE(IDC_BROWSE_TARGETAPP, OnEditChange)
    ON_EN_CHANGE(IDC_BROWSE_TARGETDIR, OnEditChange)
    ON_EN_CHANGE(IDC_EDIT_ARGS, OnEditChange)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CAdminRunDlg message handlers

BOOL CAdminRunDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // app name font
    {
        LOGFONT lf;
        m_ctlLabelName.GetFont()->GetLogFont(&lf);
        lf.lfHeight *= 2;

        m_oNameFont.CreateFontIndirect(&lf);
        m_ctlLabelName.SetFont(&m_oNameFont);
    }

    {
        m_ctlListDir.SetItemTypes(SHCONTF_FOLDERS | SHCONTF_NONFOLDERS);
    }

    // load data and fill app list
    ReloadData();

	return TRUE;
}

void CAdminRunDlg::OnAppSelection()
{
    int sel = m_ctlListApp.GetCurSel();

    if (sel < 0) {
        m_ctlLabelName.SetWindowText(_T(""));
        m_ctlBrowseApp.SetWindowText(_T(""));
        m_ctlBrowseDir.SetWindowText(_T(""));
        m_ctlEditArgs.SetWindowText(_T(""));
    }
    else {
        CShortcut * pItem = reinterpret_cast<CShortcut *>(m_ctlListApp.GetItemData(sel));

        m_ctlLabelName.SetWindowText(pItem->GetName());
        m_ctlBrowseApp.SetWindowText(pItem->GetTarget());
        m_ctlBrowseDir.SetWindowText(pItem->GetWorkingDir());
        m_ctlEditArgs.SetWindowText(pItem->GetArguments());
    }

    CRect rc;
    m_ctlLabelName.GetWindowRect(rc);
    this->ScreenToClient(rc);
    this->InvalidateRect(rc);

    BOOL bEnable = sel >= 0;
    m_ctlBtnApply.EnableWindow(FALSE);
    m_ctlBtnDelete.EnableWindow(bEnable);
    m_ctlBrowseApp.EnableWindow(bEnable);
    m_ctlBrowseDir.EnableWindow(bEnable);
    m_ctlEditArgs.EnableWindow(bEnable);
}

void CAdminRunDlg::OnAppApply()
{
    int sel = m_ctlListApp.GetCurSel();

    if (sel >= 0) {
        TCHAR buf[MAX_PATH];
        CShortcut * pItem = reinterpret_cast<CShortcut *>(m_ctlListApp.GetItemData(sel));

        m_ctlBrowseApp.GetWindowText(buf, _countof(buf));
        pItem->SetTarget(buf);
        m_ctlBrowseDir.GetWindowText(buf, _countof(buf));
        pItem->SetWorkingDir(buf);
        m_ctlEditArgs.GetWindowText(buf, _countof(buf));
        pItem->SetArguments(buf);

        if (m_pDB->UpdateItem(*pItem))
            m_ctlBtnApply.EnableWindow(FALSE);
    }
}

void CAdminRunDlg::OnAppDelete()
{
    int sel = m_ctlListApp.GetCurSel();

    if (sel >= 0) {
        CShortcut * pItem = reinterpret_cast<CShortcut *>(m_ctlListApp.GetItemData(sel));

        if (m_pDB->DeleteItem(*pItem)) {
            pItem->ChangeTargetToOriginal();

            ReloadData(sel);
        }
    }
}

void CAdminRunDlg::OnAppAdd()
{
    do {
        int idx = m_ctlListDir.GetSelectionMark();
        if (idx < 0)
            break;

        CString path;
        if (!m_ctlListDir.GetItemPath(path, idx))
            break;

        CShortcut item(path);
        if (!item.Resolve())
            break;
        
        if (!m_pDB->InsertItem(item))
            break;

        if (!item.ChangeTargetToAdminRun()) {
            m_pDB->DeleteItem(item);
            break;
        }

        CString name = item.GetName();
        ReloadData(0, name);
    } while (0);
}

void CAdminRunDlg::OnEditChange()
{
    m_ctlBtnApply.EnableWindow(TRUE);
}

void CAdminRunDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAdminRunDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAdminRunDlg::ReloadData(int iSelection, LPCTSTR szName)
{
    m_ctlListApp.ResetContent();
    m_shortcuts.RemoveAll();

    if (m_pDB->GetAllItems(m_shortcuts)) {
        int index = -1;
        for (int i = 0; i < (int)m_shortcuts.GetCount(); i++) {
            m_ctlListApp.InsertString(i, _T(""));
            m_ctlListApp.SetItemData(i, reinterpret_cast<DWORD_PTR>(m_shortcuts[i]));
            if (szName && _tcscmp(m_shortcuts[i]->GetName(), szName) == 0)
                iSelection = i;
        }

        int count = m_ctlListApp.GetCount();
        if (count == 0) {
            m_ctlListApp.SetCurSel(-1);
        }
        else if (count > iSelection) {
            m_ctlListApp.SetCurSel(iSelection);
        }
        else {
            m_ctlListApp.SetCurSel(count - 1);
        }

        OnAppSelection();
    }
}

