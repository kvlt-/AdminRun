#include "stdafx.h"
#include "PathNormalizer.h"
#include "Shortcut.h"


static CPathNormalizer g_normalizer;


CShortcut::CShortcut(INT id, LPCTSTR szFilePath)
{
    m_id = id;
    m_csFilePath = szFilePath;
    m_hIcon = NULL;
}

CShortcut::~CShortcut()
{
    if (m_hIcon) DestroyIcon(m_hIcon);
}

HICON CShortcut::GetIcon()
{
    if (m_hIcon) return m_hIcon;
    
    do {
        HRESULT hr = S_OK;

        CComPtr<IShellLink> pLink;
        CComPtr<IPersistFile> pFile;

        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pLink);
        if (FAILED(hr)) break;

        hr = pLink->QueryInterface(IID_IPersistFile, (void **)&pFile);
        if (FAILED(hr)) break;

        hr = pFile->Load(_bstr_t(m_csFilePath), STGM_READ);
        if (FAILED(hr)) break;

        hr = pLink->Resolve(NULL, SLR_NO_UI | SLR_NOUPDATE | SLR_NOSEARCH);
        if (FAILED(hr)) break;

        TCHAR szIconPath[MAX_PATH];
        int iIconIndex;
        pLink->GetIconLocation(szIconPath, _countof(szIconPath), &iIconIndex);
        if (!*szIconPath) {
            _tcscpy_s(szIconPath, _countof(szIconPath), m_csTarget);
        }
        CString csIconPath = g_normalizer.Do(szIconPath);

        m_hIcon = ExtractIcon(AfxGetInstanceHandle(), csIconPath, iIconIndex);
    } while (0);

    return m_hIcon;
}

BOOL CShortcut::Exists()
{
    return GetFileAttributes(m_csFilePath) != INVALID_FILE_ATTRIBUTES;
}

BOOL CShortcut::Resolve()
{
    HRESULT hr = S_OK;

    do {
        CComPtr<IShellLink> pLink;
        CComPtr<IPersistFile> pFile;

        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pLink);
        if (FAILED(hr)) break;

        hr = pLink->QueryInterface(IID_IPersistFile, (void **)&pFile);
        if (FAILED(hr)) break;

        hr = pFile->Load(_bstr_t(m_csFilePath), STGM_READ);
        if (FAILED(hr)) break;

        hr = pLink->Resolve(NULL, SLR_NO_UI|SLR_NOUPDATE|SLR_NOSEARCH);
        if (FAILED(hr)) break;

        TCHAR szValue[MAX_PATH];
        pLink->GetPath(szValue, _countof(szValue), NULL, SLGP_RAWPATH);
        m_csTarget = g_normalizer.Do(szValue);
        pLink->GetArguments(m_csArguments.GetBuffer(MAX_PATH), MAX_PATH);
        m_csArguments.ReleaseBuffer();
        pLink->GetWorkingDirectory(szValue, _countof(szValue));
        m_csWorkingDir = g_normalizer.Do(szValue);

    } while (0);

    return SUCCEEDED(hr);
}

BOOL CShortcut::ChangeTargetToAdminRun()
{
    TCHAR szTarget[MAX_PATH];
    GetModuleFileName(NULL, szTarget, _countof(szTarget));

    TCHAR szArgs[32];
    _stprintf_s(szArgs, _countof(szArgs), _T("/launch %d"), m_id);

    return ChangeTarget(szTarget, szArgs);
}

BOOL CShortcut::ChangeTargetToOriginal()
{
    return ChangeTarget(m_csTarget, m_csArguments);
}

BOOL CShortcut::ChangeTarget(LPCTSTR szTarget, LPCTSTR szArguments)
{
    HRESULT hr = S_OK;

    do {
        CComPtr<IShellLink> pLink;
        CComPtr<IPersistFile> pFile;

        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pLink);
        if (FAILED(hr)) break;

        hr = pLink->QueryInterface(IID_IPersistFile, (void **)&pFile);
        if (FAILED(hr)) break;

        hr = pFile->Load(_bstr_t(m_csFilePath), STGM_READWRITE);
        if (FAILED(hr)) break;

        hr = pLink->Resolve(NULL, SLR_NO_UI | SLR_NOUPDATE | SLR_NOSEARCH);
        if (FAILED(hr)) break;

        TCHAR szIconPath[MAX_PATH];
        int iIconIndex;
        hr = pLink->GetIconLocation(szIconPath, _countof(szIconPath), &iIconIndex);

        if (!*szIconPath) {
            _tcscpy_s(szIconPath, _countof(szIconPath), m_csTarget);
        }
        else if (_tcsicmp(szIconPath, szTarget) == 0) {
            szIconPath[0] = _T('\0');
        }
        hr = pLink->SetIconLocation(_bstr_t(szIconPath), iIconIndex);
        if (FAILED(hr)) break;

        hr = pLink->SetPath(_bstr_t(szTarget));
        if (FAILED(hr)) break;

        hr = pLink->SetArguments(_bstr_t(szArguments));
        if (FAILED(hr)) break;

        hr = pFile->Save(_bstr_t(m_csFilePath), TRUE);
        if (FAILED(hr)) break;
    } while (0);

    return SUCCEEDED(hr);
}
