#pragma once


class CShortcut
{
public:
    CShortcut() : CShortcut(-1, NULL) {};
    CShortcut(INT id) : CShortcut(id, NULL) {};
    CShortcut(LPCTSTR szFilePath) : CShortcut(-1, szFilePath) {};
    CShortcut(INT id, LPCTSTR szFilePath);
    ~CShortcut();

    DECLARE_GETTER_VAL(int, GetID, m_id);
    DECLARE_GETTER_VAL(LPCTSTR, GetName, m_csName);
    DECLARE_GETTER_VAL(LPCTSTR, GetFilePath, m_csFilePath);
    DECLARE_GETTER_VAL(size_t, GetFilePathLength, m_csFilePath.GetLength());
    DECLARE_GETTER_VAL(LPCTSTR, GetTarget, m_csTarget);
    DECLARE_GETTER_VAL(LPCTSTR, GetArguments, m_csArguments);
    DECLARE_GETTER_VAL(LPCTSTR, GetWorkingDir, m_csWorkingDir);
    
    DECLARE_SETTER_VAL(int, SetID, m_id);
    DECLARE_SETTER_VAL(LPCTSTR, SetName, m_csName);
    DECLARE_SETTER_VAL(LPCTSTR, SetFilePath, m_csFilePath);
    DECLARE_SETTER_VAL(LPCTSTR, SetTarget, m_csTarget);
    DECLARE_SETTER_VAL(LPCTSTR, SetArguments, m_csArguments);
    DECLARE_SETTER_VAL(LPCTSTR, SetWorkingDir, m_csWorkingDir);

    HICON GetIcon();
    BOOL Exists();

    BOOL Resolve();
    BOOL ChangeTargetToAdminRun();
    BOOL ChangeTargetToOriginal();

protected:
    INT m_id;
    CString m_csName;
    CString m_csFilePath;
    CString m_csTarget;
    CString m_csArguments;
    CString m_csWorkingDir;
    HICON m_hIcon;
    BOOL m_bDestroyIcon;

protected:
    BOOL ChangeTarget(LPCTSTR szTarget, LPCTSTR szArguments);
};


class CShortcutArray
{
public:
    CShortcutArray() {}
    ~CShortcutArray() { RemoveAll(); }

    __inline BOOL ChangeAllTargetsToOriginal()
    {
        BOOL bRet = TRUE;
        for (size_t i = 0; i < m_items.GetCount(); i++) {
            bRet &= m_items[i]->ChangeTargetToOriginal();
        }
        return bRet;
    }
    __inline size_t GetCount()
    {
        return m_items.GetCount();
    }
    __inline void SetCount(size_t count)
    {
        m_items.SetCount(count);
        ZeroMemory(m_items.GetData(), count * sizeof(CShortcut *));
    }
    __inline void RemoveAll()
    {
        for (size_t i = 0; i < m_items.GetCount(); i++) {
            if (m_items[i]) delete m_items[i];
        }
        m_items.RemoveAll();
    }
    __inline CShortcut * &operator[](size_t i)
    {
        return m_items[i];
    }

protected:
    CAtlArray<CShortcut *> m_items;

};