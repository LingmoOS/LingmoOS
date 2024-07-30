/*
    KUser - represent a user/account

    SPDX-FileCopyrightText: 2002 Tim Jansen <tim@tjansen.de>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-util.h"
#include "kcoreaddons_debug.h"
#include "kuser.h"

#include <QFileInfo>

#include <cerrno>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm> // std::find
#include <functional> // std::function

#if defined(__BIONIC__) && __ANDROID_API__ < 26
static inline struct passwd *getpwent()
{
    return nullptr;
}
inline void setpwent()
{
}
static inline void setgrent()
{
}
static inline struct group *getgrent()
{
    return nullptr;
}
inline void endpwent()
{
}
static inline void endgrent()
{
}
#endif

// Only define os_pw_size() if it's going to be used
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
static int os_pw_size() // hint for size of passwd struct
{
    const int size_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (size_max != -1) {
        return size_max;
    }
    return 1024;
}
#endif

// Only define os_gr_size() if it's going to be used
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD) && (!defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID) && (__ANDROID_API__ >= 24))
static int os_gr_size() // hint for size of group struct
{
    const int size_max = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (size_max != -1) {
        return size_max;
    }
    return 1024;
}
#endif

class KUserPrivate : public QSharedData
{
public:
    uid_t uid = uid_t(-1);
    gid_t gid = gid_t(-1);
    QString loginName;
    QString homeDir, shell;
    QMap<KUser::UserProperty, QVariant> properties;

    KUserPrivate()
    {
    }
    KUserPrivate(const char *name)
    {
        if (!name) {
            fillPasswd(nullptr);
        } else {
            struct passwd *pw = nullptr;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
            static const int bufsize = os_pw_size();
            QVarLengthArray<char, 1024> buf(bufsize);
            struct passwd entry;
            getpwnam_r(name, &entry, buf.data(), buf.size(), &pw);
#else
            pw = getpwnam(name); // not thread-safe!
#endif
            fillPasswd(pw);
        }
    }
    KUserPrivate(K_UID uid)
    {
        struct passwd *pw = nullptr;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
        static const int bufsize = os_pw_size();
        QVarLengthArray<char, 1024> buf(bufsize);
        struct passwd entry;
        getpwuid_r(uid, &entry, buf.data(), buf.size(), &pw);
#else
        pw = getpwuid(uid); // not thread-safe!
#endif
        fillPasswd(pw);
    }
    KUserPrivate(const passwd *p)
    {
        fillPasswd(p);
    }

    void fillPasswd(const passwd *p)
    {
        if (p) {
#ifndef __BIONIC__
            QString gecos = QString::fromLocal8Bit(p->pw_gecos);
#else
            QString gecos = QString();
#endif
            QStringList gecosList = gecos.split(QLatin1Char(','));
            // fill up the list, should be at least 4 entries
            while (gecosList.size() < 4) {
                gecosList << QString();
            }

            uid = p->pw_uid;
            gid = p->pw_gid;
            loginName = QString::fromLocal8Bit(p->pw_name);
            properties[KUser::FullName] = QVariant(gecosList[0]);
            properties[KUser::RoomNumber] = QVariant(gecosList[1]);
            properties[KUser::WorkPhone] = QVariant(gecosList[2]);
            properties[KUser::HomePhone] = QVariant(gecosList[3]);
            if (uid == ::getuid() && uid == ::geteuid()) {
                homeDir = QFile::decodeName(qgetenv("HOME"));
            }
            if (homeDir.isEmpty()) {
                homeDir = QFile::decodeName(p->pw_dir);
            }
            shell = QString::fromLocal8Bit(p->pw_shell);
        }
    }
};

KUser::KUser(UIDMode mode)
{
    uid_t _uid = ::getuid();
    uid_t _euid;
    if (mode == UseEffectiveUID && (_euid = ::geteuid()) != _uid) {
        d = new KUserPrivate(_euid);
    } else {
        d = new KUserPrivate(qgetenv("LOGNAME").constData());
        if (d->uid != _uid) {
            d = new KUserPrivate(qgetenv("USER").constData());
            if (d->uid != _uid) {
                d = new KUserPrivate(_uid);
            }
        }
    }
}

KUser::KUser(K_UID _uid)
    : d(new KUserPrivate(_uid))
{
}

KUser::KUser(KUserId _uid)
    : d(new KUserPrivate(_uid.nativeId()))
{
}

KUser::KUser(const QString &name)
    : d(new KUserPrivate(name.toLocal8Bit().data()))
{
}

KUser::KUser(const char *name)
    : d(new KUserPrivate(name))
{
}

KUser::KUser(const passwd *p)
    : d(new KUserPrivate(p))
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
    return isValid() && (d->uid == user.d->uid);
}

bool KUser::isValid() const
{
    return d->uid != uid_t(-1);
}

KUserId KUser::userId() const
{
    return KUserId(d->uid);
}

KGroupId KUser::groupId() const
{
    return KGroupId(d->gid);
}

bool KUser::isSuperUser() const
{
    return d->uid == 0;
}

QString KUser::loginName() const
{
    return d->loginName;
}

QString KUser::homeDir() const
{
    return d->homeDir;
}

QString KUser::faceIconPath() const
{
    QString pathToFaceIcon;
    if (!d->loginName.isEmpty()) {
        pathToFaceIcon = QStringLiteral(ACCOUNTS_SERVICE_ICON_DIR) + QLatin1Char('/') + d->loginName;
    }

    if (QFile::exists(pathToFaceIcon)) {
        return pathToFaceIcon;
    }

    pathToFaceIcon = homeDir() + QLatin1Char('/') + QLatin1String(".face.icon");

    if (QFileInfo(pathToFaceIcon).isReadable()) {
        return pathToFaceIcon;
    }

    return QString();
}

QString KUser::shell() const
{
    return d->shell;
}

template<class Func>
static void listGroupsForUser(const char *name, gid_t gid, uint maxCount, Func handleNextGroup)
{
    if (Q_UNLIKELY(maxCount == 0)) {
        return;
    }
    uint found = 0;
#if HAVE_GETGROUPLIST
    QVarLengthArray<gid_t, 100> gid_buffer;
    gid_buffer.resize(100);
    int numGroups = gid_buffer.size();
    int result = getgrouplist(name, gid, gid_buffer.data(), &numGroups);
    if (result < 0 && uint(numGroups) < maxCount) {
        // getgrouplist returns -1 if the buffer was too small to store all entries, the required size is in numGroups
        qCDebug(KCOREADDONS_DEBUG) << "Buffer was too small: " << gid_buffer.size() << ", need" << numGroups;
        gid_buffer.resize(numGroups);
        numGroups = gid_buffer.size();
        getgrouplist(name, gid, gid_buffer.data(), &numGroups);
    }
    for (int i = 0; i < numGroups && found < maxCount; ++i) {
        struct group *g = getgrgid(gid_buffer[i]); // ### not threadsafe
        // should never be null, but better be safe than crash
        if (g) {
            found++;
            handleNextGroup(g);
        }
    }
#else
    // fall back to getgrent() and reading gr->gr_mem
    // This is slower than getgrouplist, but works as well
    // add the current gid, this is often not part of g->gr_mem (e.g. build.kde.org or my openSuSE 13.1 system)
    struct group *g = getgrgid(gid); // ### not threadsafe
    if (g) {
        handleNextGroup(g);
        found++;
        if (found >= maxCount) {
            return;
        }
    }

    static const auto groupContainsUser = [](struct group *g, const char *name) -> bool {
        for (char **user = g->gr_mem; *user; user++) {
            if (strcmp(name, *user) == 0) {
                return true;
            }
        }
        return false;
    };

    setgrent();
    while ((g = getgrent())) {
        // don't add the current gid again
        if (g->gr_gid != gid && groupContainsUser(g, name)) {
            handleNextGroup(g);
            found++;
            if (found >= maxCount) {
                break;
            }
        }
    }
    endgrent();
#endif
}

QList<KUserGroup> KUser::groups(uint maxCount) const
{
    QList<KUserGroup> result;
    listGroupsForUser(d->loginName.toLocal8Bit().constData(), d->gid, maxCount, [&](const group *g) {
        result.append(KUserGroup(g));
    });
    return result;
}

QStringList KUser::groupNames(uint maxCount) const
{
    QStringList result;
    listGroupsForUser(d->loginName.toLocal8Bit().constData(), d->gid, maxCount, [&](const group *g) {
        result.append(QString::fromLocal8Bit(g->gr_name));
    });
    return result;
}

QVariant KUser::property(UserProperty which) const
{
    return d->properties.value(which);
}

QList<KUser> KUser::allUsers(uint maxCount)
{
    QList<KUser> result;

    passwd *p;
    setpwent();

    for (uint i = 0; i < maxCount && (p = getpwent()); ++i) {
        result.append(KUser(p));
    }

    endpwent();

    return result;
}

QStringList KUser::allUserNames(uint maxCount)
{
    QStringList result;

    passwd *p;
    setpwent();

    for (uint i = 0; i < maxCount && (p = getpwent()); ++i) {
        result.append(QString::fromLocal8Bit(p->pw_name));
    }

    endpwent();
    return result;
}

KUser::~KUser()
{
}

class KUserGroupPrivate : public QSharedData
{
public:
    gid_t gid = gid_t(-1);
    QString name;

    KUserGroupPrivate()
    {
    }
    KUserGroupPrivate(const char *_name)
    {
        fillGroup(_name ? ::getgrnam(_name) : nullptr);
    }
    KUserGroupPrivate(K_GID gid)
    {
        struct group *gr = nullptr;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD) && (!defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID) && (__ANDROID_API__ >= 24))
        static const int bufsize = os_gr_size();
        QVarLengthArray<char, 1024> buf(bufsize);
        struct group entry;
        // Some large systems have more members than the POSIX max size
        // Loop over by doubling the buffer size (upper limit 250k)
        for (int size = bufsize; size < 256000; size += size) {
            buf.resize(size);
            // ERANGE indicates that the buffer was too small
            if (!getgrgid_r(gid, &entry, buf.data(), buf.size(), &gr) || errno != ERANGE) {
                break;
            }
        }
#else
        gr = getgrgid(gid); // not thread-safe!
#endif
        fillGroup(gr);
    }
    KUserGroupPrivate(const ::group *p)
    {
        fillGroup(p);
    }

    void fillGroup(const ::group *p)
    {
        if (p) {
            gid = p->gr_gid;
            name = QString::fromLocal8Bit(p->gr_name);
        }
    }
};

KUserGroup::KUserGroup(KUser::UIDMode mode)
    : d(new KUserGroupPrivate(KUser(mode).groupId().nativeId()))
{
}

KUserGroup::KUserGroup(K_GID _gid)
    : d(new KUserGroupPrivate(_gid))
{
}

KUserGroup::KUserGroup(KGroupId _gid)
    : d(new KUserGroupPrivate(_gid.nativeId()))
{
}

KUserGroup::KUserGroup(const QString &_name)
    : d(new KUserGroupPrivate(_name.toLocal8Bit().data()))
{
}

KUserGroup::KUserGroup(const char *_name)
    : d(new KUserGroupPrivate(_name))
{
}

KUserGroup::KUserGroup(const ::group *g)
    : d(new KUserGroupPrivate(g))
{
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
    return isValid() && (d->gid == group.d->gid);
}

bool KUserGroup::isValid() const
{
    return d->gid != gid_t(-1);
}

KGroupId KUserGroup::groupId() const
{
    return KGroupId(d->gid);
}

QString KUserGroup::name() const
{
    return d->name;
}

static void listGroupMembers(gid_t gid, uint maxCount, std::function<void(passwd *)> handleNextGroupUser)
{
    if (maxCount == 0) {
        return;
    }
    struct group *g = getgrgid(gid); // ### not threadsafe
    if (!g) {
        return;
    }
    uint found = 0;
    QVarLengthArray<uid_t> addedUsers;
    struct passwd *p = nullptr;
    for (char **user = g->gr_mem; *user; user++) {
        if ((p = getpwnam(*user))) { // ### not threadsafe
            addedUsers.append(p->pw_uid);
            handleNextGroupUser(p);
            found++;
            if (found >= maxCount) {
                break;
            }
        }
    }

    // gr_mem doesn't contain users where the primary group == gid -> we have to iterate over all users
    setpwent();
    while ((p = getpwent()) && found < maxCount) {
        if (p->pw_gid != gid) {
            continue; // only need primary gid since otherwise gr_mem already contains this user
        }
        // make sure we don't list a user twice
        if (std::find(addedUsers.cbegin(), addedUsers.cend(), p->pw_uid) == addedUsers.cend()) {
            handleNextGroupUser(p);
            found++;
        }
    }
    endpwent();
}

QList<KUser> KUserGroup::users(uint maxCount) const
{
    QList<KUser> result;
    listGroupMembers(d->gid, maxCount, [&](const passwd *p) {
        result.append(KUser(p));
    });
    return result;
}

QStringList KUserGroup::userNames(uint maxCount) const
{
    QStringList result;
    listGroupMembers(d->gid, maxCount, [&](const passwd *p) {
        result.append(QString::fromLocal8Bit(p->pw_name));
    });
    return result;
}

QList<KUserGroup> KUserGroup::allGroups(uint maxCount)
{
    QList<KUserGroup> result;

    ::group *g;
    setgrent();

    for (uint i = 0; i < maxCount && (g = getgrent()); ++i) {
        result.append(KUserGroup(g));
    }

    endgrent();

    return result;
}

QStringList KUserGroup::allGroupNames(uint maxCount)
{
    QStringList result;

    ::group *g;
    setgrent();

    for (uint i = 0; i < maxCount && (g = getgrent()); ++i) {
        result.append(QString::fromLocal8Bit(g->gr_name));
    }

    endgrent();

    return result;
}

KUserGroup::~KUserGroup()
{
}

KUserId KUserId::fromName(const QString &name)
{
    if (name.isEmpty()) {
        return KUserId();
    }
    QByteArray name8Bit = name.toLocal8Bit();
    struct passwd *p = ::getpwnam(name8Bit.constData());
    if (!p) {
        qCWarning(KCOREADDONS_DEBUG, "Failed to lookup user %s: %s", name8Bit.constData(), strerror(errno));
        return KUserId();
    }
    return KUserId(p->pw_uid);
}

KGroupId KGroupId::fromName(const QString &name)
{
    if (name.isEmpty()) {
        return KGroupId();
    }
    QByteArray name8Bit = name.toLocal8Bit();
    struct group *g = ::getgrnam(name8Bit.constData());
    if (!g) {
        qCWarning(KCOREADDONS_DEBUG, "Failed to lookup group %s: %s", name8Bit.constData(), strerror(errno));
        return KGroupId();
    }
    return KGroupId(g->gr_gid);
}

KUserId KUserId::currentUserId()
{
    return KUserId(getuid());
}

KUserId KUserId::currentEffectiveUserId()
{
    return KUserId(geteuid());
}

KGroupId KGroupId::currentGroupId()
{
    return KGroupId(getgid());
}

KGroupId KGroupId::currentEffectiveGroupId()
{
    return KGroupId(getegid());
}
