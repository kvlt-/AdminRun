#pragma once

#include "Shortcut.h"

class CAppDB
{
public:
    CAppDB();
    ~CAppDB();

    BOOL Open();
    void Close();
    BOOL Create();
    void Delete();

    BOOL GetItem(CShortcut & item);
    BOOL GetAllItems(CShortcutArray & items);
    BOOL UpdateItem(CShortcut & item);
    BOOL InsertItem(CShortcut & item);
    BOOL DeleteItem(CShortcut & item);

protected:
    enum
    {
        STMT_COUNT_ALL,
        STMT_SELECT_ALL,
        STMT_SELECT_BY_ID,
        STMT_SELECT_BY_PATH,
        STMT_UPDATE,
        STMT_INSERT,
        STMT_DELETE,
        STMT_MAX
    };

protected:
    sqlite3 *m_pDB;
    sqlite3_stmt *m_stmt[STMT_MAX];
    BOOL m_bMarkForDelete;

protected:
    CStringA GetDBDir();
    CStringA GetDBPath();
    BOOL ResetDBFile(LPCSTR szPath);
};

