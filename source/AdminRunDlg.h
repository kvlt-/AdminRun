
// AdminRunDlg.h : header file
//

#pragma once

#include "resource.h"
#include "AppListCtrl.h"
#include "AppDB.h"

// CAdminRunDlg dialog
class CAdminRunDlg : public CDialog
{
// Construction
public:
	CAdminRunDlg(CAppDB *pDB, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ADMINRUN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support



// Implementation
protected:
	HICON m_hIcon;
    CAppDB *m_pDB;
    CShortcutArray m_shortcuts;

    CAppListCtrl m_ctlListApp;
    CMFCShellListCtrl m_ctlListDir;
    CMFCEditBrowseCtrl m_ctlBrowseApp;
    CMFCEditBrowseCtrl m_ctlBrowseDir;
    CEdit m_ctlEditArgs;
    CButton m_ctlBtnAdd;
    CButton m_ctlBtnApply;
    CButton m_ctlBtnDelete;
    CStatic m_ctlLabelName;
    CFont m_oNameFont;

	// Generated message map functions
	virtual BOOL OnInitDialog();

    afx_msg void OnAppSelection();
    afx_msg void OnAppApply();
    afx_msg void OnAppDelete();
    afx_msg void OnAppAdd();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
    void ReloadData(int iSelection, LPCTSTR szName = NULL);
};
