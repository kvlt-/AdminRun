#pragma once

// Parse command line

class CAdminRunCLI : public CCommandLineInfo
{
public:
    typedef enum _MODE_T
    {
        MODE_GUI,
        MODE_TASK,
        MODE_LAUNCH,
        MODE_ADD,
        MODE_REMOVE,
        MODE_INSTALL,
        MODE_UNINSTALL,
        MODE_HELP
    } MODE_T;

public:
    CAdminRunCLI() : CCommandLineInfo(), m_mode(MODE_GUI), m_iAppID(-1) {};

    virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
    {
        if (bFlag) {
            for (int i = 0; m_args[i].szArg; i++) {
                if (_tcscmp(pszParam, m_args[i].szArg) == 0) {
                    m_mode = m_args[i].mode;
                    break;
                }
            }
        }
        else {
            LPTSTR szEnd = NULL;
            long id = _tcstol(pszParam, &szEnd, 10);
            if (szEnd && !*szEnd) {
                m_iAppID = id;
            }
            else {
                m_csPath = pszParam;
            }
        }
    }

public:
    MODE_T m_mode;
    INT m_iAppID;
    CString m_csPath;

protected:
    typedef struct _ARG_T
    {
        LPCTSTR szArg;
        CAdminRunCLI::MODE_T mode;
    } ARG_T;

    static const ARG_T m_args[];
};

const CAdminRunCLI::ARG_T CAdminRunCLI::m_args[] =
{
    {_T("task"),    MODE_TASK},
    {_T("launch"),  MODE_LAUNCH},
    {_T("add"),     MODE_ADD},
    {_T("remove"),  MODE_REMOVE},
    {_T("install"), MODE_INSTALL},
    {_T("uninstall"), MODE_UNINSTALL},
    {_T("help"),    MODE_HELP},
    {NULL,          MODE_GUI}
};
