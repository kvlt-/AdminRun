
#pragma once

#include "AppDB.h"

class CAdminRunApp : public CWinApp
{
public:
    CAdminRunApp();

// Overrides
public:
	virtual BOOL InitInstance();
    virtual int ExitInstance();

protected:
    BOOL CheckAdminPrivileges();
    BOOL RequireAdminPrivileges();

    CString GetCurrentUserName();
    CString GetExePath();
    BOOL CheckTask(BOOL bCreate);
    void DeleteTask();

    void LaunchTask(INT iAppID);
    void LaunchProcess(INT iAppID, LPCTSTR szFilePath);
    void AddShortcut(LPCTSTR szFilePath);
    void RemoveShortcut(INT iAppID, LPCTSTR szFilePath);
    void Install();
    void Uninstall();
    void LaunchGUI();

    BOOL InitializeCom(DWORD dwCoInit = COINIT_MULTITHREADED);
    void DisplayError(LPCTSTR szText, ...);
    void DisplayHelp();

protected:
    BOOL m_bComInitialized;
    PVOID m_pRedirection;

};

extern CAdminRunApp theApp;