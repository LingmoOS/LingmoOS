/*
    KUser - represent a user/account (Windows)

    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kuser.h"

#include "kcoreaddons_debug.h"
#include <QDir>
#include <QStandardPaths>

#include <memory> // unique_ptr
#include <type_traits>

#include <qt_windows.h> // Must be included before lm.h

#include <lm.h> //Net*

#include <sddl.h> //ConvertSidToStringSidW
#include <shlobj.h>
#include <userenv.h> //GetProfilesDirectoryW

// this can't be a lambda due to a MSVC2012 bug
// (works fine in 2010 and 2013)
struct netApiBufferDeleter {
    void operator()(void *buffer)
    {
        if (buffer) {
            NetApiBufferFree(buffer);
        }
    }
};

template<typename T>
class ScopedNetApiBuffer : public std::unique_ptr<T, netApiBufferDeleter>
{
public:
    // explicit scope resolution operator needed in ::netApiBufferDeleter
    // because of *another* MSVC bug :(
    inline explicit ScopedNetApiBuffer(T *data)
        : std::unique_ptr<T, ::netApiBufferDeleter>(data, ::netApiBufferDeleter())
    {
    }
};

const auto handleCloser = [](HANDLE h) {
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
    }
};
typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(handleCloser)> ScopedHANDLE;

/** Make sure the NetApi functions are called with the correct level argument (for template functions)
 * This argument can be retrieved by using NetApiTypeInfo<T>::level. In order to do so the type must be
 * registered by writing e.g. NETAPI_TYPE_INFO(GROUP_INFO, 0) for GROUP_INFO_0
 */
template<typename T>
struct NetApiTypeInfo {
};
#define NETAPI_TYPE_INFO(prefix, n)                                                                                                                            \
    template<>                                                                                                                                                 \
    struct NetApiTypeInfo<prefix##_##n> {                                                                                                                      \
        enum { level = n };                                                                                                                                    \
    };
NETAPI_TYPE_INFO(GROUP_INFO, 0)
NETAPI_TYPE_INFO(GROUP_INFO, 3)
NETAPI_TYPE_INFO(USER_INFO, 0)
NETAPI_TYPE_INFO(USER_INFO, 4)
NETAPI_TYPE_INFO(USER_INFO, 11)
NETAPI_TYPE_INFO(GROUP_USERS_INFO, 0)

// T must be a USER_INFO_* structure
template<typename T>
ScopedNetApiBuffer<T> getUserInfo(LPCWSTR server, const QString &userName, NET_API_STATUS *errCode)
{
    LPBYTE userInfoTmp = nullptr;
    // if level = 11 a USER_INFO_11 structure gets filled in and allocated by NetUserGetInfo(), etc.
    NET_API_STATUS status = NetUserGetInfo(server, (LPCWSTR)userName.utf16(), NetApiTypeInfo<T>::level, &userInfoTmp);
    if (status != NERR_Success) {
        userInfoTmp = nullptr;
    }
    if (errCode) {
        *errCode = status;
    }
    return ScopedNetApiBuffer<T>((T *)userInfoTmp);
}

// enumeration functions
/** simplify calling the Net*Enum functions to prevent copy and paste for allUsers(), allUserNames(), allGroups(), allGroupNames()
 * @tparam T The type that is enumerated (e.g. USER_INFO_11) Must be registered using NETAPI_TYPE_INFO.
 * @param callback Callback for each listed object. Signature: void(const T&)
 * @param enumFunc This function enumerates the data using a Net* function.
 * It will be called in a loop as long as it returns ERROR_MORE_DATA.
 *
 */
template<class T, class Callback, class EnumFunction>
static void netApiEnumerate(uint maxCount, Callback callback, EnumFunction enumFunc)
{
    NET_API_STATUS nStatus = NERR_Success;
    DWORD_PTR resumeHandle = 0;
    uint total = 0;
    int level = NetApiTypeInfo<T>::level;
    do {
        LPBYTE buffer = nullptr;
        DWORD entriesRead = 0;
        DWORD totalEntries = 0;
        nStatus = enumFunc(level, &buffer, &entriesRead, &totalEntries, &resumeHandle);
        // qDebug("Net*Enum(level = %d) returned %d entries, total was (%d), status = %d, resume handle = %llx",
        //    level, entriesRead, totalEntries, nStatus, resumeHandle);

        // buffer must always be freed, even if Net*Enum fails
        ScopedNetApiBuffer<T> groupInfo((T *)buffer);
        if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
            for (DWORD i = 0; total < maxCount && i < entriesRead; i++, total++) {
                callback(groupInfo.get()[i]);
            }
        } else {
            qCWarning(KCOREADDONS_DEBUG, "NetApi enumerate function failed: status = %d", (int)nStatus);
        }
    } while (nStatus == ERROR_MORE_DATA);
}

template<class T, class Callback>
void enumerateAllUsers(uint maxCount, Callback callback)
{
    netApiEnumerate<T>(maxCount, callback, [](int level, LPBYTE *buffer, DWORD *count, DWORD *total, PDWORD_PTR resumeHandle) {
        // pass 0 as filter -> get all users
        // Why does this function take a DWORD* as resume handle and NetUserEnum/NetGroupGetUsers a UINT64*
        // Great API design by Microsoft...
        // casting the uint64* to uint32* is fine, it just writes to the first 32 bits
        return NetUserEnum(nullptr, level, 0, buffer, MAX_PREFERRED_LENGTH, count, total, (PDWORD)resumeHandle);
    });
}

template<typename T, class Callback>
void enumerateAllGroups(uint maxCount, Callback callback)
{
    netApiEnumerate<T>(maxCount, callback, [](int level, LPBYTE *buffer, DWORD *count, DWORD *total, PDWORD_PTR resumeHandle) {
        return NetGroupEnum(nullptr, level, buffer, MAX_PREFERRED_LENGTH, count, total, resumeHandle);
    });
}

template<typename T, class Callback>
void enumerateGroupsForUser(uint maxCount, const QString &name, Callback callback)
{
    LPCWSTR nameStr = (LPCWSTR)name.utf16();
    netApiEnumerate<T>(maxCount, callback, [&](int level, LPBYTE *buffer, DWORD *count, DWORD *total, PDWORD_PTR resumeHandle) -> NET_API_STATUS {
        Q_UNUSED(resumeHandle);
        NET_API_STATUS ret = NetUserGetGroups(nullptr, nameStr, level, buffer, MAX_PREFERRED_LENGTH, count, total);
        // if we return ERROR_MORE_DATA here it will result in an endless loop
        if (ret == ERROR_MORE_DATA) {
            qCWarning(KCOREADDONS_DEBUG) << "NetUserGetGroups for user" << name << "returned ERROR_MORE_DATA. This should not happen!";
            ret = NERR_Success;
        }
        return ret;
    });
}

template<typename T, class Callback>
void enumerateUsersForGroup(const QString &name, uint maxCount, Callback callback)
{
    LPCWSTR nameStr = (LPCWSTR)name.utf16();
    netApiEnumerate<T>(maxCount, callback, [nameStr](int level, LPBYTE *buffer, DWORD *count, DWORD *total, PDWORD_PTR resumeHandle) {
        return NetGroupGetUsers(nullptr, nameStr, level, buffer, MAX_PREFERRED_LENGTH, count, total, resumeHandle);
    });
}

class KUserPrivate : public QSharedData
{
    typedef QExplicitlySharedDataPointer<KUserPrivate> Ptr;
    KUserPrivate()
        : isAdmin(false)
    {
    }
    // takes ownership over userInfo_
    KUserPrivate(KUserId uid, KGroupId gid, const QString &loginName, const QString &fullName, const QString &domain, const QString &homeDir, bool isAdmin)
        : uid(uid)
        , gid(gid)
        , loginName(loginName)
        , fullName(fullName)
        , domain(domain)
        , homeDir(homeDir)
        , isAdmin(isAdmin)
    {
        Q_ASSERT(uid.isValid());
    }
    static QString guessHomeDir(const QString &username, KUserId uid)
    {
        // usri11_home_dir/usri4_home_dir is often empty
        // check whether it is the homedir for the current user and if not then fall back to "<user profiles dir>\<user name>"
        if (uid == KUserId::currentUserId()) {
            return QDir::homePath();
        }
        QString homeDir;
        WCHAR profileDirPath[MAX_PATH];
        DWORD bufSize = MAX_PATH;
        BOOL result = GetProfilesDirectoryW(profileDirPath, &bufSize);
        if (result) {
            // This might not be correct: e.g. with local user and domain user with same
            // In that case it could be C:\Users\Foo (local user) vs C:\Users\Foo.DOMAIN (domain user)
            // However it is still much better than the previous code which just returned the current users home dir
            homeDir = QString::fromWCharArray(profileDirPath) + QLatin1Char('\\') + username;
        }
        return homeDir;
    }

public:
    static Ptr sharedNull;
    KUserId uid;
    KGroupId gid;
    QString loginName;
    QString fullName;
    QString domain;
    QString homeDir;
    bool isAdmin;

    /** Creates a user info from a SID (never returns null) */
    static Ptr create(KUserId uid)
    {
        if (!uid.isValid()) {
            return sharedNull;
        }
        // now find the fully qualified name for the user
        DWORD nameBufferLen = UNLEN + 1;
        WCHAR nameBuffer[UNLEN + 1];
        DWORD domainBufferLen = UNLEN + 1;
        WCHAR domainBuffer[UNLEN + 1];
        SID_NAME_USE use;
        if (!LookupAccountSidW(nullptr, uid.nativeId(), nameBuffer, &nameBufferLen, domainBuffer, &domainBufferLen, &use)) {
            qCWarning(KCOREADDONS_DEBUG) << "Could not lookup user " << uid.toString() << "error =" << GetLastError();
            return sharedNull;
        }
        QString loginName = QString::fromWCharArray(nameBuffer);
        QString domainName = QString::fromWCharArray(domainBuffer);
        if (use != SidTypeUser && use != SidTypeDeletedAccount) {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "SID for " << domainName << "\\" << loginName << " (" << uid.toString() << ") is not of type user ("
                                                   << SidTypeUser << " or " << SidTypeDeletedAccount << "). Got type " << use << " instead.";
            return sharedNull;
        }
        // now get the server name to query (could be null for local machine)
        LPWSTR servernameTmp = nullptr;
        NET_API_STATUS status = NetGetAnyDCName(nullptr, nullptr, (LPBYTE *)&servernameTmp);
        if (status != NERR_Success) {
            // this always fails on my desktop system, don't spam the output
            // qDebug("NetGetAnyDCName failed with error %d", status);
        }
        ScopedNetApiBuffer<WCHAR> servername(servernameTmp);

        QString fullName;
        QString homeDir;
        KGroupId group;
        bool isAdmin = false;
        // must NOT pass the qualified name ("domain\user") here or lookup fails -> just the name
        // try USER_INFO_4 first, MSDN says it is valid only on servers (whatever that means), it works on my desktop system
        // If it fails fall back to USER_INFO11, which has all the needed information except primary group
        if (auto userInfo4 = getUserInfo<USER_INFO_4>(servername.get(), loginName, &status)) {
            Q_ASSERT(KUserId(userInfo4->usri4_user_sid) == uid); // if this is not the same we have a logic error
            fullName = QString::fromWCharArray(userInfo4->usri4_full_name);
            homeDir = QString::fromWCharArray(userInfo4->usri4_home_dir);
            isAdmin = userInfo4->usri4_priv == USER_PRIV_ADMIN;
            // now determine the primary group:
            const DWORD primaryGroup = userInfo4->usri4_primary_group_id;
            // primary group is a relative identifier, i.e. in order to get the SID for that group
            // we have to take the user SID and replace the last subauthority value with the relative identifier
            group = KGroupId(uid.nativeId()); // constructor does not check whether the sid refers to a group
            Q_ASSERT(group.isValid());
            UCHAR numSubauthorities = *GetSidSubAuthorityCount(group.nativeId());
            PDWORD lastSubAutority = GetSidSubAuthority(group.nativeId(), numSubauthorities - 1);
            *lastSubAutority = primaryGroup;
        } else if (auto userInfo11 = getUserInfo<USER_INFO_11>(servername.get(), loginName, &status)) {
            fullName = QString::fromWCharArray(userInfo11->usri11_full_name);
            homeDir = QString::fromWCharArray(userInfo11->usri11_home_dir);
            isAdmin = userInfo11->usri11_priv == USER_PRIV_ADMIN;
        } else {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "Could not get information for user " << domainName << "\\" << loginName << ": error code = " << status;
            return sharedNull;
        }
        if (homeDir.isEmpty()) {
            homeDir = guessHomeDir(loginName, uid);
        }
        // if we couldn't find a primary group just take the first group found for this user
        if (!group.isValid()) {
            enumerateGroupsForUser<GROUP_USERS_INFO_0>(1, loginName, [&](const GROUP_USERS_INFO_0 &info) {
                group = KGroupId::fromName(QString::fromWCharArray(info.grui0_name));
            });
        }
        return Ptr(new KUserPrivate(uid, group, loginName, fullName, domainName, homeDir, isAdmin));
    }
};

KUserPrivate::Ptr KUserPrivate::sharedNull(new KUserPrivate());

KUser::KUser(UIDMode mode)
{
    if (mode == UseEffectiveUID) {
        d = KUserPrivate::create(KUserId::currentEffectiveUserId());
    } else if (mode == UseRealUserID) {
        d = KUserPrivate::create(KUserId::currentUserId());
    } else {
        d = KUserPrivate::sharedNull;
    }
}

KUser::KUser(K_UID uid)
    : d(KUserPrivate::create(KUserId(uid)))
{
}

KUser::KUser(KUserId uid)
    : d(KUserPrivate::create(uid))
{
}

KUser::KUser(const QString &name)
    : d(KUserPrivate::create(KUserId::fromName(name)))
{
}

KUser::KUser(const char *name)
    : d(KUserPrivate::create(KUserId::fromName(QString::fromLocal8Bit(name))))
{
}

KUser::KUser(const KUser &user)
    : d(user.d)
{
}

KUser &KUser::operator=(const KUser &user)
{
    d = user.d;
    return *this;
}

bool KUser::operator==(const KUser &user) const
{
    return isValid() && d->uid == user.d->uid;
}

bool KUser::isValid() const
{
    return d->uid.isValid();
}

bool KUser::isSuperUser() const
{
    return d->isAdmin;
}

QString KUser::loginName() const
{
    return d->loginName;
}

QString KUser::homeDir() const
{
    return d->homeDir;
}

// Some RAII objects to help uninitializing/destroying WinAPI stuff
// used in faceIconPath.
class COMInitializer
{
public:
    COMInitializer()
        : result(CoInitialize(nullptr))
    {
    }
    ~COMInitializer()
    {
        if (SUCCEEDED(result)) {
            CoUninitialize();
        }
    }
    HRESULT result;
};
class W32Library
{
public:
    W32Library(HMODULE h)
        : h(h)
    {
    }
    ~W32Library()
    {
        if (h) {
            FreeLibrary(h);
        }
    }
    operator HMODULE()
    {
        return h;
    }
    HMODULE h;
};

// faceIconPath uses undocumented Windows API known as SHGetUserPicturePath,
// only accessible by ordinal, unofficially documented at
// http://undoc.airesoft.co.uk/shell32.dll/SHGetUserPicturePath.php

// The function has a different ordinal and parameters on Windows XP and Vista/7.
// These structs encapsulate the differences.

struct FaceIconPath_XP {
    typedef HRESULT(WINAPI *funcptr_t)(LPCWSTR, DWORD, LPWSTR);
    static const int ordinal = 233;
    static HRESULT getPicturePath(funcptr_t SHGetUserPicturePathXP, LPCWSTR username, LPWSTR buf, UINT bufsize)
    {
        Q_UNUSED(bufsize);
        // assumes the buffer is MAX_PATH in size
        return SHGetUserPicturePathXP(username, 0, buf);
    }
};
struct FaceIconPath_Vista {
    typedef HRESULT(WINAPI *funcptr_t)(LPCWSTR, DWORD, LPWSTR, UINT);
    static const int ordinal = 261;
    static HRESULT getPicturePath(funcptr_t SHGetUserPicturePathV, LPCWSTR username, LPWSTR buf, UINT bufsize)
    {
        return SHGetUserPicturePathV(username, 0, buf, bufsize);
    }
};

template<typename Platform>
static QString faceIconPathImpl(LPCWSTR username)
{
    static COMInitializer COMinit;

    static W32Library shellMod = LoadLibraryA("shell32.dll");
    if (!shellMod) {
        return QString();
    }
    static typename Platform::funcptr_t sgupp_ptr =
        reinterpret_cast<typename Platform::funcptr_t>(GetProcAddress(shellMod, MAKEINTRESOURCEA(Platform::ordinal)));
    if (!sgupp_ptr) {
        return QString();
    }

    WCHAR pathBuf[MAX_PATH];

    HRESULT res = Platform::getPicturePath(sgupp_ptr, username, pathBuf, MAX_PATH);
    if (res != S_OK) {
        return QString();
    }
    return QString::fromWCharArray(pathBuf);
}

QString KUser::faceIconPath() const
{
    if (!isValid()) {
        return QString();
    }

    LPCWSTR username = reinterpret_cast<const WCHAR *>(d->loginName.utf16());
    return faceIconPathImpl<FaceIconPath_Vista>(username);
}

QString KUser::shell() const
{
    return isValid() ? QStringLiteral("cmd.exe") : QString();
}

KUserId KUser::userId() const
{
    return d->uid;
}

KGroupId KUser::groupId() const
{
    return d->gid;
}

QVariant KUser::property(UserProperty which) const
{
    if (which == FullName) {
        return QVariant(d->fullName);
    }

    return QVariant();
}

KUser::~KUser()
{
}

class KUserGroupPrivate : public QSharedData
{
public:
    QString name;
    KGroupId gid;
    KUserGroupPrivate()
    {
    }
    KUserGroupPrivate(const QString &name, KGroupId id)
        : name(name)
        , gid(id)
    {
        if (!name.isEmpty()) {
            PBYTE groupInfoTmp = nullptr;
            NET_API_STATUS status = NetGroupGetInfo(nullptr, (LPCWSTR)name.utf16(), 0, &groupInfoTmp);
            // must always be freed, even on error
            ScopedNetApiBuffer<GROUP_INFO_0> groupInfo((GROUP_INFO_0 *)groupInfoTmp);
            if (status != NERR_Success) {
                qCWarning(KCOREADDONS_DEBUG) << "Failed to find group with name" << name << "error =" << status;
                groupInfo.reset();
            }
            if (!id.isValid()) {
                gid = KGroupId::fromName(name);
            }
        }
    }
};

KUserGroup::KUserGroup(const QString &_name)
    : d(new KUserGroupPrivate(_name, KGroupId()))
{
}

KUserGroup::KUserGroup(const char *_name)
    : d(new KUserGroupPrivate(QLatin1String(_name), KGroupId()))
{
}

static QString nameFromGroupId(KGroupId gid)
{
    if (!gid.isValid()) {
        return QString();
    }

    DWORD bufferLen = UNLEN + 1;
    WCHAR buffer[UNLEN + 1];
    DWORD domainBufferLen = UNLEN + 1;
    WCHAR domainBuffer[UNLEN + 1];
    SID_NAME_USE eUse;
    QString name;
    if (LookupAccountSidW(nullptr, gid.nativeId(), buffer, &bufferLen, domainBuffer, &domainBufferLen, &eUse)) {
        if (eUse == SidTypeGroup || eUse == SidTypeWellKnownGroup) {
            name = QString::fromWCharArray(buffer);
        } else {
            qCWarning(KCOREADDONS_DEBUG) << QString::fromWCharArray(buffer) << "is not a group, SID type is" << eUse;
        }
    }
    return name;
}

KUserGroup::KUserGroup(KGroupId gid)
    : d(new KUserGroupPrivate(nameFromGroupId(gid), gid))
{
}

KUserGroup::KUserGroup(K_GID gid)
{
    KGroupId groupId(gid);
    d = new KUserGroupPrivate(nameFromGroupId(groupId), groupId);
}

KUserGroup::KUserGroup(KUser::UIDMode mode)
{
    KGroupId gid;
    if (mode == KUser::UseEffectiveUID) {
        gid = KGroupId::currentGroupId();
    } else if (mode == KUser::UseRealUserID) {
        gid = KGroupId::currentEffectiveGroupId();
    }
    d = new KUserGroupPrivate(nameFromGroupId(gid), gid);
}

KUserGroup::KUserGroup(const KUserGroup &group)
    : d(group.d)
{
}

KUserGroup &KUserGroup::operator=(const KUserGroup &group)
{
    d = group.d;
    return *this;
}

bool KUserGroup::operator==(const KUserGroup &group) const
{
    return isValid() && d->gid == group.d->gid && d->name == group.d->name;
}

bool KUserGroup::isValid() const
{
    return d->gid.isValid() && !d->name.isEmpty();
}

QString KUserGroup::name() const
{
    return d->name;
}

KGroupId KUserGroup::groupId() const
{
    return d->gid;
}

KUserGroup::~KUserGroup()
{
}

QList<KUser> KUser::allUsers(uint maxCount)
{
    QList<KUser> result;
    // No advantage if we take a USER_INFO_11, since there is no way of copying it
    // and it is not owned by this function!
    // -> get a USER_INFO_0 instead and then use KUser(QString)
    // USER_INFO_23 or USER_INFO_23 would be ideal here since they contains a SID,
    // but that fails with error code 0x7c (bad level)
    enumerateAllUsers<USER_INFO_0>(maxCount, [&result](const USER_INFO_0 &info) {
        result.append(KUser(QString::fromWCharArray(info.usri0_name)));
    });
    return result;
}

QStringList KUser::allUserNames(uint maxCount)
{
    QStringList result;
    enumerateAllUsers<USER_INFO_0>(maxCount, [&result](const USER_INFO_0 &info) {
        result.append(QString::fromWCharArray(info.usri0_name));
    });
    return result;
}

QList<KUserGroup> KUserGroup::allGroups(uint maxCount)
{
    QList<KUserGroup> result;
    // MSDN documentation say 3 is a valid level, however the function fails with invalid level!!!
    // User GROUP_INFO_0 instead...
    enumerateAllGroups<GROUP_INFO_0>(maxCount, [&result](const GROUP_INFO_0 &groupInfo) {
        result.append(KUserGroup(QString::fromWCharArray(groupInfo.grpi0_name)));
    });
    return result;
}

QStringList KUserGroup::allGroupNames(uint maxCount)
{
    QStringList result;
    enumerateAllGroups<GROUP_INFO_0>(maxCount, [&result](const GROUP_INFO_0 &groupInfo) {
        result.append(QString::fromWCharArray(groupInfo.grpi0_name));
    });
    return result;
}

QList<KUserGroup> KUser::groups(uint maxCount) const
{
    QList<KUserGroup> result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->loginName, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(KUserGroup(QString::fromWCharArray(info.grui0_name)));
    });
    return result;
}

QStringList KUser::groupNames(uint maxCount) const
{
    QStringList result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->loginName, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(QString::fromWCharArray(info.grui0_name));
    });
    return result;
}

QList<KUser> KUserGroup::users(uint maxCount) const
{
    QList<KUser> result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->name, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(KUser(QString::fromWCharArray(info.grui0_name)));
    });
    return result;
}

QStringList KUserGroup::userNames(uint maxCount) const
{
    QStringList result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->name, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(QString::fromWCharArray(info.grui0_name));
    });
    return result;
}

static const auto invalidSidString = QStringLiteral("<invalid SID>");

static QString sidToString(void *sid)
{
    if (!sid || !IsValidSid(sid)) {
        return invalidSidString;
    }
    WCHAR *sidStr; // allocated by ConvertStringSidToSidW, must be freed using LocalFree()
    if (!ConvertSidToStringSidW(sid, &sidStr)) {
        return invalidSidString;
    }
    QString ret = QString::fromWCharArray(sidStr);
    LocalFree(sidStr);
    return ret;
}

struct WindowsSIDWrapper : public QSharedData {
    char sidBuffer[SECURITY_MAX_SID_SIZE];
    /** @return a copy of @p sid or null if sid is not valid or an error occurs */
    static WindowsSIDWrapper *copySid(PSID sid)
    {
        if (!sid || !IsValidSid(sid)) {
            return nullptr;
        }
        // create a copy of sid
        WindowsSIDWrapper *copy = new WindowsSIDWrapper();
        bool success = CopySid(SECURITY_MAX_SID_SIZE, copy->sidBuffer, sid);
        if (!success) {
            QString sidString = sidToString(sid);
            qCWarning(KCOREADDONS_DEBUG, "Failed to copy SID %s, error = %d", qPrintable(sidString), (int)GetLastError());
            delete copy;
            return nullptr;
        }
        return copy;
    }
};

template<>
KUserOrGroupId<void *>::KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void *>::~KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void *>::KUserOrGroupId(const KUserOrGroupId<void *> &other)
    : data(other.data)
{
}

template<>
inline KUserOrGroupId<void *> &KUserOrGroupId<void *>::operator=(const KUserOrGroupId<void *> &other)
{
    data = other.data;
    return *this;
}

template<>
KUserOrGroupId<void *>::KUserOrGroupId(void *nativeId)
    : data(WindowsSIDWrapper::copySid(nativeId))
{
}

template<>
bool KUserOrGroupId<void *>::isValid() const
{
    return data;
}

template<>
void *KUserOrGroupId<void *>::nativeId() const
{
    if (!data) {
        return nullptr;
    }
    return data->sidBuffer;
}

template<>
bool KUserOrGroupId<void *>::operator==(const KUserOrGroupId<void *> &other) const
{
    if (data) {
        if (!other.data) {
            return false;
        }
        return EqualSid(data->sidBuffer, other.data->sidBuffer);
    }
    return !other.data; // only equal if other data is also invalid
}

template<>
bool KUserOrGroupId<void *>::operator!=(const KUserOrGroupId<void *> &other) const
{
    return !(*this == other);
}

template<>
QString KUserOrGroupId<void *>::toString() const
{
    return sidToString(data ? data->sidBuffer : nullptr);
}

/** T must be either KUserId or KGroupId, Callback has signature T(PSID, SID_NAME_USE) */
template<class T, class Callback>
static T sidFromName(const QString &name, Callback callback)
{
    if (name.isEmpty()) {
        // for some reason empty string will always return S-1-5-32 which is of type domain
        // we only want users or groups -> return invalid
        return T();
    }
    char buffer[SECURITY_MAX_SID_SIZE];
    DWORD sidLength = SECURITY_MAX_SID_SIZE;
    // ReferencedDomainName must be passed or LookupAccountNameW fails
    // Documentation says it is optional, however if not passed the function fails and returns the required size
    WCHAR domainBuffer[1024];
    DWORD domainBufferSize = 1024;
    SID_NAME_USE sidType;
    bool ok = LookupAccountNameW(nullptr, (LPCWSTR)name.utf16(), buffer, &sidLength, domainBuffer, &domainBufferSize, &sidType);
    if (!ok) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to lookup account" << name << "error code =" << GetLastError();
        return T();
    }
    return callback(buffer, sidType);
}

KUserId KUserId::fromName(const QString &name)
{
    return sidFromName<KUserId>(name, [&](PSID sid, SID_NAME_USE sidType) -> KUserId {
        if (sidType != SidTypeUser && sidType != SidTypeDeletedAccount) {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "Failed to lookup user name " << name << ": resulting SID " << sidToString(sid)
                                                   << " is not a user."
                                                      " Got SID type "
                                                   << sidType << " instead.";
            return KUserId();
        }
        return KUserId(sid);
    });
}

KGroupId KGroupId::fromName(const QString &name)
{
    return sidFromName<KGroupId>(name, [&](PSID sid, SID_NAME_USE sidType) -> KGroupId {
        if (sidType != SidTypeGroup && sidType != SidTypeWellKnownGroup) {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "Failed to lookup user name " << name << ": resulting SID " << sidToString(sid)
                                                   << " is not a group."
                                                      " Got SID type "
                                                   << sidType << " instead.";
            return KGroupId();
        }
        return KGroupId(sid);
    });
}

static std::unique_ptr<char[]> queryProcessInformation(TOKEN_INFORMATION_CLASS type)
{
    HANDLE _token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &_token)) {
        qCWarning(KCOREADDONS_DEBUG, "Failed to get the token for the current process: %d", (int)GetLastError());
        return nullptr;
    }
    ScopedHANDLE token(_token, handleCloser);
    // query required size
    DWORD requiredSize;
    if (!GetTokenInformation(token.get(), type, nullptr, 0, &requiredSize)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            qCWarning(KCOREADDONS_DEBUG, "Failed to get the required size for the token information %d: %d", type, (int)GetLastError());
            return nullptr;
        }
    }
    std::unique_ptr<char[]> buffer(new char[requiredSize]);
    if (!GetTokenInformation(token.get(), type, buffer.get(), requiredSize, &requiredSize)) {
        qCWarning(KCOREADDONS_DEBUG, "Failed to get token information %d from current process: %d", type, (int)GetLastError());
        return nullptr;
    }
    return buffer;
}

KUserId KUserId::currentUserId()
{
    std::unique_ptr<char[]> userTokenBuffer = queryProcessInformation(TokenUser);
    TOKEN_USER *userToken = (TOKEN_USER *)userTokenBuffer.get();
    return KUserId(userToken->User.Sid);
}

KGroupId KGroupId::currentGroupId()
{
    std::unique_ptr<char[]> primaryGroupBuffer = queryProcessInformation(TokenPrimaryGroup);
    TOKEN_PRIMARY_GROUP *primaryGroup = (TOKEN_PRIMARY_GROUP *)primaryGroupBuffer.get();
    return KGroupId(primaryGroup->PrimaryGroup);
}

KUserId KUserId::currentEffectiveUserId()
{
    return currentUserId();
}

KGroupId KGroupId::currentEffectiveGroupId()
{
    return currentGroupId();
}

KCOREADDONS_EXPORT size_t qHash(const KUserId &id, size_t seed)
{
    if (!id.isValid()) {
        return seed;
    }
    // we can't just hash the pointer since equal object must have the same hash -> hash contents
    char *sid = (char *)id.nativeId();
    return qHash(QByteArray::fromRawData(sid, GetLengthSid(sid)), seed);
}

KCOREADDONS_EXPORT size_t qHash(const KGroupId &id, size_t seed)
{
    if (!id.isValid()) {
        return seed;
    }
    // we can't just hash the pointer since equal object must have the same hash -> hash contents
    char *sid = (char *)id.nativeId();
    return qHash(QByteArray::fromRawData(sid, GetLengthSid(sid)), seed);
}
