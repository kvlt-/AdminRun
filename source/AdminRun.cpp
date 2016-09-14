
// AdminRun.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "taskdefs.h"
#include "Shortcut.h"
#include "AdminRunDlg.h"
#include "AdminRunCLI.h"
#include "AdminRun.h"

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")


CAdminRunApp theApp;


// CAdminRunApp initialization


CAdminRunApp::CAdminRunApp()
{
    m_bComInitialized = FALSE;
    m_pRedirection = NULL;
}

BOOL CAdminRunApp::InitInstance()
{
    CWinApp::InitInstance();

    CAdminRunCLI cli;
    ParseCommandLine(cli);

    switch (cli.m_mode) {

    // TASK mode
    // admin    -> launch app
    // user     -> fail
    case CAdminRunCLI::MODE_TASK:
    {
        if (RequireAdminPrivileges()) {
            LaunchProcess(cli.m_iAppID, cli.m_csPath);
        }
        break;
    }

    // LAUNCH mode
    // admin    -> launch app
    // user     -> run task with database ID as argument
    case  CAdminRunCLI::MODE_LAUNCH:
    {
        if (CheckAdminPrivileges()) {
            LaunchProcess(cli.m_iAppID, cli.m_csPath);
        }
        else {
            LaunchTask(cli.m_iAppID);
        }
        break;
    }

    // ADD/REMOVE modes
    // admin    -> add/remove app to/from database
    // user     -> fail
    case CAdminRunCLI::MODE_ADD:
    {
        if (RequireAdminPrivileges()) {
            AddShortcut(cli.m_csPath);
        }
        break;
    }
    case CAdminRunCLI::MODE_REMOVE:
    {
        if (RequireAdminPrivileges()) {
            RemoveShortcut(cli.m_iAppID, cli.m_csPath);
        }
        break;
    }

    // INSTALL/UNINSTALL modes
    // admin    -> install/uninstall task, create/destroy database and unassign shortcuts
    // user     -> fail
    case CAdminRunCLI::MODE_INSTALL:
    {
        if (RequireAdminPrivileges()) {
            Install();
        }
        break;
    }
    case CAdminRunCLI::MODE_UNINSTALL:
    {
        if (RequireAdminPrivileges()) {
            Uninstall();
        }
        break;
    }

    // HELP mode
    // displays help
    case CAdminRunCLI::MODE_HELP:
    {
        DisplayHelp();
        break;
    }

    // GUI mode
    // admin    -> check if task/db are installed, run GUI
    // user     -> fail
    case CAdminRunCLI::MODE_GUI:
    {
        if (RequireAdminPrivileges()) {
            LaunchGUI();
        }
        break;
    }

    }

	return FALSE;
}

int CAdminRunApp::ExitInstance()
{
    if (m_bComInitialized) CoUninitialize();

    return CWinApp::ExitInstance();
}


BOOL CAdminRunApp::CheckAdminPrivileges()
{
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            bRet = Elevation.TokenIsElevated;
        }
    }

    if (hToken) CloseHandle(hToken);

    return bRet;
}

BOOL CAdminRunApp::RequireAdminPrivileges()
{
    BOOL bRet = CheckAdminPrivileges();
    if (!bRet) {
        DisplayError(_T("AdminRun must be run with Administrator privileges."));
    }

    return bRet;
}

CString CAdminRunApp::GetCurrentUserName()
{
    CString csUserName;
    DWORD dwLength = 128;
    GetUserName(csUserName.GetBuffer(dwLength), &dwLength);
    csUserName.ReleaseBuffer();

    return csUserName;
}

CString CAdminRunApp::GetExePath()
{
    CString csPath;
    GetModuleFileNameW(NULL, csPath.GetBuffer(MAX_PATH), MAX_PATH);
    csPath.ReleaseBuffer();

    return csPath;
}

BOOL CAdminRunApp::CheckTask(BOOL bCreate)
{
    CComPtr<IRegisteredTask> pRegisteredTask;
    HRESULT hr = S_OK;

    if (!InitializeCom()) return FALSE;

    do {
        CComPtr<ITaskService> pService;
        CComPtr<ITaskFolder> pRootFolder;

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
        if (FAILED(hr)) break;

        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) break;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) break;

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) break;

        hr = pRootFolder->GetTask(_bstr_t(DEF_TASK_NAME), &pRegisteredTask);
        if (!bCreate) {
            if (FAILED(hr)) DisplayError(_T("Failed to get task."));
            break;
        }
            
        // (re)create new task
        if (pRegisteredTask != NULL) {
            pRegisteredTask.Release();
            hr = pRootFolder->DeleteTask(_bstr_t(DEF_TASK_NAME), 0);
            if (FAILED(hr)) break;
        }

        CComPtr<ITaskDefinition>pTaskDef;
        pService->NewTask(0, &pTaskDef);

        // registration info
        {
            CComPtr<IRegistrationInfo> pRegInfo;
            pTaskDef->get_RegistrationInfo(&pRegInfo);
            pRegInfo->put_Author(_bstr_t(DEF_TASK_AUTHOR));
            pRegInfo->put_Description(_bstr_t(DEF_TASK_DESCRIPTION));
        }
        // security principals
        {
            CComPtr<IPrincipal> pPrincipal;
            pTaskDef->get_Principal(&pPrincipal);
            pPrincipal->put_Id(_bstr_t(GetCurrentUserName()));
            pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
            pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        }
        // task settings
        {
            CComPtr<ITaskSettings> pSettings;
            pTaskDef->get_Settings(&pSettings);
            pSettings->put_Enabled(TRUE);
            pSettings->put_AllowDemandStart(TRUE);
            pSettings->put_AllowHardTerminate(TRUE);
        }
        // action definition
        {
            CComPtr<IActionCollection> pActions;
            CComPtr<IExecAction> pAction;
            pTaskDef->get_Actions(&pActions);
            pActions->Create(TASK_ACTION_EXEC, (IAction **)&pAction);
            pAction->put_Path(_bstr_t(GetExePath()));
            pAction->put_Arguments(_bstr_t(DEF_TASK_ARGUMENTS));
        }

        hr = pRootFolder->RegisterTaskDefinition(_bstr_t(DEF_TASK_NAME), pTaskDef, TASK_CREATE, _variant_t(), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(), &pRegisteredTask);
        if (FAILED(hr)) {
            DisplayError(_T("Failed to create task."));
            break;
        }
    } while (0);

    return pRegisteredTask != NULL;
}

void CAdminRunApp::DeleteTask()
{
    HRESULT hr = S_OK;

    if (!InitializeCom()) return;

    do {
        CComPtr<ITaskService> pService;
        CComPtr<ITaskFolder> pRootFolder;

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
        if (FAILED(hr)) break;

        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) break;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) break;

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) break;

        hr = pRootFolder->DeleteTask(_bstr_t(DEF_TASK_NAME), 0);
        if (FAILED(hr)) break;
    } while (0);

}

void CAdminRunApp::LaunchTask(INT iAppID)
{
    BOOL bErr = TRUE;

    if (!InitializeCom()) return;

    do {
        HRESULT hr = S_OK;
        CComPtr<ITaskService> pService;
        CComPtr<ITaskFolder> pRootFolder;
        CComPtr<IRegisteredTask> pRegisteredTask;
        CComPtr<IRunningTask> pRunningTask;
        CComVariant vAppID(iAppID);

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
        if (FAILED(hr)) break;

        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) break;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) break;

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) break;

        hr = pRootFolder->GetTask(_bstr_t(DEF_TASK_NAME), &pRegisteredTask);
        if (FAILED(hr)) break;

        hr = pRegisteredTask->RunEx(vAppID, TASK_RUN_AS_SELF, 0, _bstr_t(GetCurrentUserName()), &pRunningTask);
        if (FAILED(hr)) break;

        bErr = FALSE;
    } while (0);

    if (bErr) DisplayError(_T("Failed to launch task: %d."), iAppID);
}

void CAdminRunApp::LaunchProcess(INT iAppID, LPCTSTR szFilePath)
{
    CAppDB db;
    if (!db.Open()) {
        DisplayError(_T("Failed to open app database."));
        return;
    }

    CShortcut sc(iAppID, szFilePath);
    if (!db.GetItem(sc)) {
        DisplayError(_T("Failed to get shortcut info:\r\n(%d) %s"), iAppID, szFilePath);
        return;
    }

    Wow64DisableWow64FsRedirection(&m_pRedirection);

    TCHAR szTarget[MAX_PATH], szArgs[MAX_PATH], szDir[MAX_PATH];
    ExpandEnvironmentStrings(sc.GetTarget(), szTarget, _countof(szTarget));
    ExpandEnvironmentStrings(sc.GetArguments(), szArgs, _countof(szArgs));
    ExpandEnvironmentStrings(sc.GetWorkingDir(), szDir, _countof(szDir));

    if (!*szDir) {
        _tcscpy_s(szDir, _countof(szDir), szTarget);
        LPTSTR szSlash = _tcsrchr(szDir, _T('\\'));
        if (szSlash) *szSlash = _T('\0');
    }

    SHELLEXECUTEINFO si = {0};
    si.cbSize = sizeof(SHELLEXECUTEINFO);
    si.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | SEE_MASK_FLAG_LOG_USAGE;
    si.lpVerb = _T("open");
    si.lpFile = szTarget;
    si.lpParameters = szArgs;
    si.lpDirectory = szDir;
    si.nShow = SW_SHOW;

    InitializeCom(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!ShellExecuteEx(&si)) {
        DisplayError(_T("Failed to launch process:\r\n%s"), szTarget);
    }

    Wow64RevertWow64FsRedirection(m_pRedirection);
}

void CAdminRunApp::AddShortcut(LPCTSTR szFilePath)
{
    if (!InitializeCom()) return;

    CAppDB db;
    if (!db.Open()) {
        DisplayError(_T("Failed to open app database."));
        return;
    }

    CShortcut sc(szFilePath);
    if (!sc.Resolve()) {
        DisplayError(_T("Failed to resolve shortcut:\r\n%s"), szFilePath);
        return;
    }
    if (!db.InsertItem(sc)) {
        DisplayError(_T("Failed to add shortcut to database:\r\n%s"), szFilePath);
        return;
    }
    if (!sc.ChangeTargetToAdminRun()) {
        db.DeleteItem(sc);
        DisplayError(_T("Failed to set shortcut to AdminRun target:\r\n%s"), szFilePath);
        return;
    }
}

void CAdminRunApp::RemoveShortcut(INT iAppID, LPCTSTR szFilePath)
{
    if (!InitializeCom()) return;

    CAppDB db;
    if (!db.Open()) {
        DisplayError(_T("Failed to open app database."));
        return;
    }

    CShortcut sc(iAppID, szFilePath);
    if (!db.GetItem(sc)) {
        DisplayError(_T("Failed to get shortcut info:\r\n(%d) %s"), iAppID, szFilePath);
        return;
    }
    if (!db.DeleteItem(sc)) {
        DisplayError(_T("Failed to remove shortcut from database:\r\n(%d) %s"), iAppID, szFilePath);
        return;
    }
    if (sc.Exists()) {
        if (!sc.ChangeTargetToOriginal()) {
            DisplayError(_T("Failed to set shortcut to original target:\r\n%s"), szFilePath);
            return;
        }
    }
}

void CAdminRunApp::Install()
{
    if (!CheckTask(TRUE)) return;

    CAppDB db;
    if (!db.Create()) {
        DisplayError(_T("Failed to create app database."));
        return;
    }
}

void CAdminRunApp::Uninstall()
{
    DeleteTask();

    CAppDB db;
    if (db.Open()) {
        CShortcutArray shortcuts;
        db.GetAllItems(shortcuts);
        shortcuts.ChangeAllTargetsToOriginal();
    }
    db.Delete();
}

void CAdminRunApp::LaunchGUI()
{
    if (!CheckTask(FALSE)) return;

    CAppDB db;
    if (!db.Open()) {
        DisplayError(_T("Failed to open app database."));
        return;
    }

    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CShellManager *pShellManager = new CShellManager;
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    CAdminRunDlg dlg(&db);
    m_pMainWnd = &dlg;
    dlg.DoModal();

    if (pShellManager) delete pShellManager;
}

BOOL CAdminRunApp::InitializeCom(DWORD dwCoInit)
{
    return m_bComInitialized || (m_bComInitialized = CoInitializeEx(0, dwCoInit) == S_OK);
}

void CAdminRunApp::DisplayError(LPCTSTR szText, ...)
{
    TCHAR szContent[2048];

    va_list args;
    va_start(args, szText);
    _vstprintf_s(szContent, _countof(szContent), szText, args);
    va_end(args);

    MessageBox(NULL, szContent, _T("AdminRun"), MB_ICONERROR | MB_OK);
}

void CAdminRunApp::DisplayHelp()
{
    static LPCTSTR szHelp = 
        _T("AdminRun.exe\r\n-  opens AdminRun link editor GUI\r\n\r\n")
        _T("AdminRun.exe /task 10\r\n - launch link with ID 10 as Admin\r\n\r\n")
        _T("AdminRun.exe /launch 10\r\n - launch link with ID 10 if Admin, otherwise use task to do it\r\n\r\n")
        _T("AdminRun.exe /launch \"C:\\my folder\\MyLink.lnk\"\r\n - same as above with link path specified\r\n\r\n")
        _T("AdminRun.exe /add \"C:\\my folder \\MyLink.lnk\"\r\n - modify link for AdminRun use, assign ID\r\n\r\n")
        _T("AdminRun.exe /remove 10\r\n - modify link with ID 10 for its original use, unassign ID\r\n\r\n")
        _T("AdminRun.exe /remove \"C:\\my folder \\MyLink.lnk\"\r\n - same as above with link path specified\r\n\r\n")
        _T("AdminRun.exe /install\r\n - installs AdminRun\r\n\r\n")
        _T("AdminRun.exe /uninstall\r\n - uninstalls AdminRun, unassigns shortcuts\r\n\r\n")
        _T("AdminRun.exe /help\r\n - show this help");

    MessageBox(NULL, szHelp, _T("AdminRun Usage"), MB_ICONINFORMATION | MB_OK);
}
