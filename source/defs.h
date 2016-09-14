#pragma once

#define DEF_DB_DIR              "%APPDATA%\\kvlt\\"
#define DEF_DB_FILE             DEF_DB_DIR "AdminRun.db"



// helpful delete macros

#define CLOSEHANDLE(h)          if (h) { CloseHandle(h); h = NULL; }
#define DESTRUCT(x)             if (x) { delete x; x = NULL; }
#define DESTROYMENU(m)          if (m) { DestroyMenu(m); m = NULL; }
#define CLOSEFILE(f)            if (f != INVALID_HANDLE_VALUE) { CloseHandle(f); f = INVALID_HANDLE_VALUE; }


// getter/setter macros

#define DECLARE_GETTER_VAL(Type,Fnc,Member) \
    __inline Type Fnc() const \
{ \
    return Member; \
}
#define DECLARE_GETTER_REF_LOCK(Type,Fnc,Member) \
    __inline void Fnc(Type &value) \
{ \
    Lock(); \
    value = Member; \
    Unlock(); \
}
#define DECLARE_GETTER_VAL_LOCK(Type,Fnc,Member) \
    __inline const Type Fnc() \
{ \
    Type value; \
    Lock(); \
    value = Member; \
    Unlock(); \
    return value; \
}
#define DECLARE_SETTER_VAL(Type,Fnc,Member) \
    __inline void Fnc(Type value) \
{ \
    Member = value; \
}
#define DECLARE_SETTER_REF_LOCK(Type,Fnc,Member) \
    __inline void Fnc(Type &value) \
{ \
    Lock(); \
    Member = value; \
    Unlock(); \
}
#define DECLARE_SETTER_VAL_LOCK(Type,Fnc,Member) \
    __inline void Fnc(Type value) \
{ \
    Lock(); \
    Member = value; \
    Unlock(); \
}