// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QFile>
#include <QQueue>
#include <QString>
#include <functional>
#include <QRegularExpression>
#include <DStandardPaths>
#include <QLoggingCategory>
#include "helper.hpp"

Q_DECLARE_LOGGING_CATEGORY(cfLog);

// /appid/filename/subpath/userid
using ConnKey = QString;
// /appid/filename/subpath
using ResourceKey = QString;
// /filename/subpath
using GenericResourceKey = QString;
using ConnServiceName = QString;
using ConnRefCount = int;
// user: u-${ConnKey}, global: g-${ResourceKey}
using ConfigCacheKey = QString;
static constexpr int TestUid = 0U;
static const QString VirtualInterAppId = "_";

inline QString formatDBusObjectPath(QString path)
{
    return path.replace(QRegularExpression(QStringLiteral("[\\. -]")), QStringLiteral("_"));
}
inline QString outerAppidToInner(const QString &appid)
{
    return appid == NoAppId ? VirtualInterAppId : appid;
}
inline QString innerAppidToOuter(const QString &appid)
{
    return appid == VirtualInterAppId ? NoAppId : appid;
}
inline bool isGenericResourceConn(const ConnKey &connKey)
{
    return connKey.startsWith("/" + VirtualInterAppId);
}
inline ResourceKey getResourceKey(const QString &appid, const GenericResourceKey &key)
{
    return QString("/%1%2").arg(appid).arg(key);
}
inline ResourceKey getResourceKey(const ConnKey &connKey)
{
    return connKey.left(connKey.lastIndexOf('/'));
}
inline GenericResourceKey getGenericResourceKeyByResourceKey(const ResourceKey &resourceKey)
{
    return resourceKey.mid(resourceKey.indexOf('/', 1));
}
inline GenericResourceKey getGenericResourceKey(const QString &name, const QString &subpath)
{
    return QString("/%1%2").arg(name, subpath);
}
inline GenericResourceKey getGenericResourceKey(const ConnKey &connKey)
{
    return getGenericResourceKeyByResourceKey(getResourceKey(connKey));
}
inline uint getConnectionKey(const ConnKey &connKey)
{
    return connKey.mid(connKey.lastIndexOf('/') + 1).toUInt();
}
inline ConnKey getConnectionKey(const ResourceKey &key, const uint uid)
{
    return QString("%1/%2").arg(key).arg(uid);
}

struct ConfigureId {
    QString appid;
    QString subpath;
    QString resource;
    bool isInValid() const {
        return resource.isEmpty();
    }
};

// remove slash if back is slash.
static QString removeBackSlash(const QString &target)
{
    if (target.isEmpty() || target.back() != '/')
        return target;

    return target.left(target.count() - 1);
}

inline ConfigureId getMetaConfigureId(const QString &path)
{
    ConfigureId info;
    // /usr/share/dsg/configs/[$appid]/[$subpath]/$resource.json
    static QRegularExpression usrReg(R"(/configs/(?<appid>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)?)(?<subpath>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)*)(?<resource>[a-z0-9\s\-_\@\-\^!#$%&.]+).json$)");

    QRegularExpressionMatch match;
    match = usrReg.match(path);
    if (!match.hasMatch()) {
        return info;
    }
    info.appid = removeBackSlash(match.captured("appid"));
    info.subpath = removeBackSlash(match.captured("subpath"));
    info.resource = match.captured("resource");

    return info;
}

inline ConfigureId getOverrideConfigureId(const QString &path)
{
    ConfigureId info;
    // /usr/share/dsg/configs/overrides/[$appid]/$resource/[$subpath]/$override_id.json
    static QRegularExpression usrReg(R"(/configs/overrides/(?<appid>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)?)(?<resource>[a-z0-9\s\-_\@\-\^!#$%&.]+)/(?<subpath>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)*)(?<configurationid>[a-z0-9\s\-_\@\-\^!#$%&.]+).json$)");

    // /etc/dsg/configs/overrides/[$appid]/$resource/[$subpath]/$override_id.json
    static QRegularExpression etcReg(R"(^/etc/dsg/configs/overrides/(?<appid>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)?)(?<resource>[a-z0-9\s\-_\@\-\^!#$%&.]+)/(?<subpath>([a-z0-9\s\-_\@\-\^!#$%&.]+\/)*)(?<configurationid>[a-z0-9\s\-_\@\-\^!#$%&.]+).json$)");

    QRegularExpressionMatch match;
    match = usrReg.match(path);
    if (!match.hasMatch()) {
        match = etcReg.match(path);
        if (!match.hasMatch())
            return info;
    }
    info.appid = removeBackSlash(match.captured("appid"));
    info.subpath = removeBackSlash(match.captured("subpath"));
    info.resource = match.captured("resource");

    return info;
}

template<class T>
class ObjectPool
{
public:
    typedef T* DataType;
    ~ObjectPool()
    {
        clear();
    }
    using InitFunc = std::function<void(DataType)>;
    void setInitFunc(InitFunc func) { m_initFunc = func;}

    DataType pull()
    {
        if (m_pool.isEmpty()) {
            auto item = new T();
            if (m_initFunc)
                m_initFunc(item);

            return item;
        }
        return m_pool.dequeue();
    }
    void push(DataType item) { m_pool.enqueue(item);}

    void clear()
    {
        qDeleteAll(m_pool);
        m_pool.clear();
    }

private:
    QQueue<DataType> m_pool;
    InitFunc m_initFunc;
};

inline QString getProcessNameByPid(const uint pid)
{
#ifdef Q_OS_LINUX
    const QString desc = QString("/proc/%1/cmdline").arg(pid);

    QFile file(desc);
    if(file.open(QIODevice::ReadOnly)) {
        const QByteArray &name = file.readLine();
        return name.split('\0').join(" ").trimmed();
    }
#endif // Q_OS_LINUX
    return QString::number(pid);
}

#ifdef Q_OS_LINUX
#include <pwd.h>
#endif // Q_OS_LINUX
inline QString getUserNameByUid(const uint uid)
{
#ifdef Q_OS_LINUX
    passwd *passwd = getpwuid(uid);
    return QString::fromLocal8Bit(passwd->pw_name);
#else // Q_OS_LINUX
    return QString::number(uid);
#endif
}
