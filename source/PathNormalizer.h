#pragma once

#define REG_PROGRAMFILES_KEY        _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define REG_PROGRAMFILES_VALUE      _T("ProgramFilesDir")

class CPathNormalizer
{
public:
    CPathNormalizer() : m_bInit(FALSE) {};

    CString Do(LPCTSTR szPath)
    {
        Init();

        CString csUnresolved;
        BOOL bToResolve = _tcsncmp(szPath, m_csProgramFiles32, m_csProgramFiles32.GetLength()) == 0;
        if (bToResolve && GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES) {
            csUnresolved.Format(_T("%s%s"), m_csProgramFiles64, szPath + m_csProgramFiles32.GetLength());
        }
        else {
            csUnresolved = szPath;
        }

        return csUnresolved;
    }

protected:
    BOOL m_bInit;
    CString m_csProgramFiles32;
    CString m_csProgramFiles64;

protected:
    void Init()
    {
        if (m_bInit) return;

        CRegKey key;
        ULONG ulLen;

        if (key.Open(HKEY_LOCAL_MACHINE, REG_PROGRAMFILES_KEY, KEY_READ) == ERROR_SUCCESS) {
            ulLen = MAX_PATH;
            key.QueryStringValue(REG_PROGRAMFILES_VALUE, m_csProgramFiles32.GetBuffer(ulLen), &ulLen);
            m_csProgramFiles32.ReleaseBuffer();
            key.Close();
        }

        if (key.Open(HKEY_LOCAL_MACHINE, REG_PROGRAMFILES_KEY, KEY_WOW64_64KEY | KEY_READ) == ERROR_SUCCESS) {
            ulLen = MAX_PATH;
            key.QueryStringValue(REG_PROGRAMFILES_VALUE, m_csProgramFiles64.GetBuffer(ulLen), &ulLen);
            m_csProgramFiles64.ReleaseBuffer();
            key.Close();
        }

        m_bInit = TRUE;
    }
};