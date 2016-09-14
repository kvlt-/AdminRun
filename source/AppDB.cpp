#include "stdafx.h"

#include "AppDB.h"

#define SQL_RESET \
"CREATE TABLE apps (\
  id INTEGER PRIMARY KEY,\
  name VARCHAR(260) NOT NULL collate nocase,\
  path VARCHAR(260) NOT NULL collate nocase,\
  target VARCHAR(260) NOT NULL,\
  args VARCHAR(260) NOT NULL,\
  dir VARCHAR(260)) NOT NULL;\
CREATE UNIQUE INDEX idx_path ON apps(path);"
#define SQL_COUNT_ALL   "SELECT count(*) FROM apps;"
#define SQL_SELECT_ALL  "SELECT id,name,path,target,args,dir FROM apps ORDER BY name ASC;"
#define SQL_SELECT_BY_ID    "SELECT id,name,path,target,args,dir FROM apps WHERE id=?;"
#define SQL_SELECT_BY_PATH  "SELECT id,name,path,target,args,dir FROM apps WHERE path=?;"
#define SQL_UPDATE      "UPDATE apps SET name=?,path=?,target=?,args=?,dir=? WHERE id=?;"
#define SQL_INSERT      "INSERT INTO apps(name,path,target,args,dir) VALUES (?,?,?,?,?);"
#define SQL_DELETE      "DELETE FROM apps WHERE id=?;"


CAppDB::CAppDB()
{
    m_pDB = NULL;
    ZeroMemory(m_stmt, sizeof(m_stmt));
    m_bMarkForDelete = FALSE;
}


CAppDB::~CAppDB()
{
    Close();
}

BOOL CAppDB::Open()
{
    return (m_pDB != NULL) || (sqlite3_open(GetDBPath(), &m_pDB) == SQLITE_OK);
}

void CAppDB::Close()
{
    for (int i = 0; i < STMT_MAX; i++) {
        if (m_stmt[i]) {
            sqlite3_finalize(m_stmt[i]);
            m_stmt[i] = NULL;
        }
    }
    if (m_pDB) {
        sqlite3_close(m_pDB);
        m_pDB = NULL;
    }
    if (m_bMarkForDelete) DeleteFileA(GetDBPath());
}

BOOL CAppDB::Create()
{
    BOOL bRet = TRUE;
    CStringA csDB = GetDBPath();

    do {
        bRet = CreateDirectoryA(GetDBDir(), NULL) || (GetLastError() == ERROR_ALREADY_EXISTS);
        if (!bRet) break;

        bRet = ResetDBFile(csDB);
        if (!bRet) break;

        bRet = sqlite3_open(csDB, &m_pDB) == SQLITE_OK;
        if (!bRet) break;

        bRet = sqlite3_exec(m_pDB, SQL_RESET, NULL, NULL, NULL) == SQLITE_OK;
        if (!bRet) break;
    } while (0);

    return bRet;
}

void CAppDB::Delete()
{
    m_bMarkForDelete = TRUE;
}

CStringA CAppDB::GetDBPath()
{
    CStringA csPath;
    ExpandEnvironmentStringsA(DEF_DB_FILE, csPath.GetBuffer(MAX_PATH), MAX_PATH);
    csPath.ReleaseBuffer();

    return csPath;
}

CStringA CAppDB::GetDBDir()
{
    CStringA csPath;
    ExpandEnvironmentStringsA(DEF_DB_DIR, csPath.GetBuffer(MAX_PATH), MAX_PATH);
    csPath.ReleaseBuffer();

    return csPath;
}

BOOL CAppDB::ResetDBFile(LPCSTR szPath)
{
    BOOL bRet = FALSE;
    PACL pDacl = NULL;
    EXPLICIT_ACCESS ea[2] = {0};
    PSID psidAdmin = NULL;
    PSID psidSystem = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PSECURITY_DESCRIPTOR pSD = NULL;

    do {
        DWORD dwSize = SECURITY_MAX_SID_SIZE;
        psidAdmin = LocalAlloc(LMEM_FIXED, dwSize);
        psidSystem = LocalAlloc(LMEM_FIXED, dwSize);
        if (!psidAdmin || !psidSystem) break;
        if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, psidAdmin, &dwSize)) break;
        if (!CreateWellKnownSid(WinLocalSystemSid, NULL, psidSystem, &dwSize)) break;

        ea[0].grfAccessPermissions = GENERIC_ALL;
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance= NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName  = (LPTSTR)psidAdmin;

        CopyMemory(&ea[1], &ea[0], sizeof(ea[0]));
        ea[1].Trustee.ptstrName  = (LPTSTR)psidSystem;
        
        if (SetEntriesInAcl(_countof(ea), ea, NULL, &pDacl) != ERROR_SUCCESS) break;

        pSD = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (!pSD || !InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) break;
        if (!SetSecurityDescriptorDacl(pSD, TRUE, pDacl, FALSE)) break;

        SECURITY_ATTRIBUTES sa = {0};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSD;
        hFile = CreateFileA(szPath, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) break;

        //if (SetSecurityInfo(hFile, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, pDacl, NULL) != ERROR_SUCCESS) break;

        bRet = TRUE;
    } while (0);

    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    if (pSD) LocalFree(pSD);
    if (pDacl) LocalFree(pDacl);
    if (psidAdmin) LocalFree(psidAdmin);
    if (psidSystem) LocalFree(psidSystem);

    return bRet;
}

BOOL CAppDB::GetAllItems(CShortcutArray & items)
{
    int sret = SQLITE_OK;
    sqlite3_stmt* stmtCnt = m_stmt[STMT_COUNT_ALL];
    sqlite3_stmt* stmt = m_stmt[STMT_SELECT_ALL];

    do {
        if (!stmtCnt) {
            sret = sqlite3_prepare(m_pDB, SQL_COUNT_ALL, sizeof(SQL_COUNT_ALL) - 1, &m_stmt[STMT_COUNT_ALL], NULL);
            if (sret != SQLITE_OK) break;
            stmtCnt = m_stmt[STMT_COUNT_ALL];
        }
        if (!stmt) {
            sret = sqlite3_prepare(m_pDB, SQL_SELECT_ALL, sizeof(SQL_SELECT_ALL) - 1, &m_stmt[STMT_SELECT_ALL], NULL);
            if (sret != SQLITE_OK) break;
            stmt = m_stmt[STMT_SELECT_ALL];
        }

        sret = sqlite3_step(stmtCnt);
        if (sret != SQLITE_ROW) break;

        INT cnt = sqlite3_column_int(stmtCnt, 0);
        items.SetCount(cnt);
        for(int i = 0; i < cnt; i++) {
            sret = sqlite3_step(stmt);
            if (sret != SQLITE_ROW) break;

            items[i] = new CShortcut();
            items[i]->SetID((int)sqlite3_column_int(stmt, 0));
            items[i]->SetName(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 1), CP_UTF8));
            items[i]->SetFilePath(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 2), CP_UTF8));
            items[i]->SetTarget(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 3), CP_UTF8));
            items[i]->SetArguments(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 4), CP_UTF8));
            items[i]->SetWorkingDir(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 5), CP_UTF8));
        }
    } while (0);

    if (stmtCnt) sqlite3_reset(stmtCnt);
    if (stmt) sqlite3_reset(stmt);

    if (sret != SQLITE_ROW) items.RemoveAll();

    return sret == SQLITE_ROW;
}


BOOL CAppDB::GetItem(CShortcut & item)
{
    int sret = SQLITE_OK;
    sqlite3_stmt* stmt = NULL;
    CHAR szPathA[MAX_PATH];

    do {
        if (item.GetID() >= 0) {
            if (!m_stmt[STMT_SELECT_BY_ID]) {
                sret = sqlite3_prepare(m_pDB, SQL_SELECT_BY_ID, sizeof(SQL_SELECT_BY_ID) - 1, &m_stmt[STMT_SELECT_BY_ID], NULL);
                if (sret != SQLITE_OK) break;
            }
            stmt = m_stmt[STMT_SELECT_BY_ID];

            sret = sqlite3_bind_int(stmt, 1, item.GetID());
            if (sret != SQLITE_OK) break;
        }
        else if (item.GetFilePathLength()) {
            if (!m_stmt[STMT_SELECT_BY_PATH]) {
                sret = sqlite3_prepare(m_pDB, SQL_SELECT_BY_PATH, sizeof(SQL_SELECT_BY_PATH) - 1, &m_stmt[STMT_SELECT_BY_PATH], NULL);
                if (sret != SQLITE_OK) break;
            }
            stmt = m_stmt[STMT_SELECT_BY_PATH];

            size_t len = item.GetFilePathLength();
            WideCharToMultiByte(CP_UTF8, 0, item.GetFilePath(), len+1, szPathA, _countof(szPathA), NULL, NULL);
            sret = sqlite3_bind_text(stmt, 1, szPathA, len, SQLITE_STATIC);
            if (sret != SQLITE_OK) break;
        }
        else break;

        sret = sqlite3_step(stmt);
        if (sret == SQLITE_ROW) {
            item.SetID((int)sqlite3_column_int(stmt, 0));
            item.SetName(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 1), CP_UTF8));
            item.SetFilePath(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 2), CP_UTF8));
            item.SetTarget(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 3), CP_UTF8));
            item.SetArguments(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 4), CP_UTF8));
            item.SetWorkingDir(CA2TEX<MAX_PATH>((LPSTR)sqlite3_column_text(stmt, 5), CP_UTF8));
        }
    } while (0);

    if (stmt) sqlite3_reset(stmt);

    return sret == SQLITE_ROW;
}

BOOL CAppDB::UpdateItem(CShortcut & item)
{
    int sret = SQLITE_OK;
    sqlite3_stmt* stmt = m_stmt[STMT_UPDATE];

    do {
        if (!stmt) {
            sret = sqlite3_prepare(m_pDB, SQL_UPDATE, sizeof(SQL_UPDATE) - 1, &m_stmt[STMT_UPDATE], NULL);
            if (sret != SQLITE_OK) break;
            stmt = m_stmt[STMT_UPDATE];
        }

        CT2AEX<MAX_PATH> szNameA(item.GetName(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 1, szNameA, strlen(szNameA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szPathA(item.GetFilePath(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 2, szPathA, strlen(szPathA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szTargetA(item.GetTarget(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 3, szTargetA, strlen(szTargetA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szArgsA(item.GetArguments(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 4, szArgsA, strlen(szArgsA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szDirA(item.GetWorkingDir(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 5, szDirA, strlen(szDirA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        sret = sqlite3_bind_int(stmt, 6, item.GetID());
        if (sret != SQLITE_OK) break;

        sret = sqlite3_step(stmt);
    } while (0);

    if (stmt) sqlite3_reset(stmt);

    return sret == SQLITE_DONE;
}

BOOL CAppDB::InsertItem(CShortcut & item)
{
    int sret = SQLITE_OK;
    sqlite3_stmt* stmt = m_stmt[STMT_INSERT];

    do {
        if (!stmt) {
            sret = sqlite3_prepare(m_pDB, SQL_INSERT, sizeof(SQL_INSERT) - 1, &m_stmt[STMT_INSERT], NULL);
            if (sret != SQLITE_OK) break;
           stmt = m_stmt[STMT_INSERT];
        }

        CT2AEX<MAX_PATH> szNameA(item.GetName(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 1, szNameA, strlen(szNameA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szPathA(item.GetFilePath(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 2, szPathA, strlen(szPathA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szTargetA(item.GetTarget(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 3, szTargetA, strlen(szTargetA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szArgsA(item.GetArguments(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 4, szArgsA, strlen(szArgsA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        CT2AEX<MAX_PATH> szDirA(item.GetWorkingDir(), CP_UTF8);
        sret = sqlite3_bind_text(stmt, 5, szDirA, strlen(szDirA), SQLITE_STATIC);
        if (sret != SQLITE_OK) break;

        sret = sqlite3_step(stmt);
        if (sret == SQLITE_DONE) {
            item.SetID((int)sqlite3_last_insert_rowid(m_pDB));
        }
    } while (0);

    if (stmt) sqlite3_reset(stmt);

    return sret == SQLITE_DONE;
}

BOOL CAppDB::DeleteItem(CShortcut & item)
{
    int sret = SQLITE_OK;
    sqlite3_stmt* stmt = m_stmt[STMT_DELETE];

    do {
        if (!stmt) {
            sret = sqlite3_prepare(m_pDB, SQL_DELETE, sizeof(SQL_DELETE) - 1, &m_stmt[STMT_DELETE], NULL);
            if (sret != SQLITE_OK) break;
            stmt = m_stmt[STMT_DELETE];
        }

        sret = sqlite3_bind_int(stmt, 1, item.GetID());
        if (sret != SQLITE_OK) break;

        sret = sqlite3_step(stmt);
    } while (0);

    if (stmt) sqlite3_reset(stmt);

    return sret == SQLITE_DONE;
}
