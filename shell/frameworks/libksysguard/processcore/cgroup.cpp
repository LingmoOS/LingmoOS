/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "cgroup.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QStringView>
#include <QThreadPool>

#include "process.h"

using namespace KSysGuard;

class KSysGuard::CGroupPrivate
{
public:
    CGroupPrivate(const QString &_processGroupId)
        : processGroupId(_processGroupId)
        , service(serviceFromAppId(_processGroupId))
    {
    }
    const QString processGroupId;
    const KService::Ptr service;
    QList<pid_t> pids;

    static KService::Ptr serviceFromAppId(const QString &appId);

    static QRegularExpression s_appIdFromProcessGroupPattern;
    static QString unescapeName(const QString &cgroupId);

    class ReadPidsRunnable;
};

class CGroupPrivate::ReadPidsRunnable : public QRunnable
{
public:
    ReadPidsRunnable(QObject *context, const QString &path, std::function<void(QList<pid_t>)> callback)
        : m_context(context)
        , m_path(path)
        , m_callback(callback)
    {
    }

    void run() override
    {
        QFile pidFile(m_path);
        pidFile.open(QFile::ReadOnly | QIODevice::Text);
        QTextStream stream(&pidFile);

        QList<pid_t> pids;
        QString line = stream.readLine();
        while (!line.isNull()) {
            pids.append(line.toLong());
            line = stream.readLine();
        }
        // Ensure we call the callback on the thread the context object lives on.
        if (m_context) {
            QMetaObject::invokeMethod(m_context, std::bind(m_callback, pids));
        }
    }

private:
    QPointer<QObject> m_context;
    QString m_path;
    std::function<void(QList<pid_t>)> m_callback;
};

class CGroupSystemInformation
{
public:
    CGroupSystemInformation();
    QString sysGgroupRoot;
};

Q_GLOBAL_STATIC(CGroupSystemInformation, s_cGroupSystemInformation)

// The spec says that the two following schemes are allowed
// - app[-<launcher>]-<ApplicationID>-<RANDOM>.scope
// - app[-<launcher>]-<ApplicationID>[@<RANDOM>].service
// Flatpak's are currently in a cgroup, but they don't follow the specification
// this has been fixed, but this provides some compatibility till that lands
// app vs apps exists because the spec changed.
QRegularExpression
    CGroupPrivate::s_appIdFromProcessGroupPattern(QStringLiteral("(apps|app|flatpak)-(?:[^-]*-)?([^-]+(?=-.*\\.scope)|[^@]+(?=(?:@.*)?\\.service))"));

CGroup::CGroup(const QString &id)
    : d(new CGroupPrivate(id))
{
}

CGroup::~CGroup()
{
}

QString KSysGuard::CGroup::id() const
{
    return d->processGroupId;
}

KService::Ptr KSysGuard::CGroup::service() const
{
    return d->service;
}

QList<pid_t> CGroup::pids() const
{
    return d->pids;
}

void CGroup::setPids(const QList<pid_t> &pids)
{
    d->pids = pids;
}

void CGroup::requestPids(QObject *context, std::function<void(QList<pid_t>)> callback)
{
    QString path = cgroupSysBasePath() + d->processGroupId + QLatin1String("/cgroup.procs");
    auto readPidsRunnable = new CGroupPrivate::ReadPidsRunnable(context, path, callback);
    QThreadPool::globalInstance()->start(readPidsRunnable);
}

QString CGroupPrivate::unescapeName(const QString &name)
{
    // strings are escaped in the form of \xZZ where ZZ is a two digits in hex representing an ascii code
    QString rc = name;
    while (true) {
        int escapeCharIndex = rc.indexOf(QLatin1Char('\\'));
        if (escapeCharIndex < 0) {
            break;
        }
        const QStringView sequence = QStringView(rc).mid(escapeCharIndex, 4);
        if (sequence.length() != 4 || sequence.at(1) != QLatin1Char('x')) {
            qWarning() << "Badly formed cgroup name" << name;
            return name;
        }
        bool ok;
        int character = sequence.mid(2).toInt(&ok, 16);
        if (ok) {
            rc.replace(escapeCharIndex, 4, QLatin1Char(character));
        }
    }
    return rc;
}

KService::Ptr CGroupPrivate::serviceFromAppId(const QString &processGroup)
{
    const int lastSlash = processGroup.lastIndexOf(QLatin1Char('/'));

    QString serviceName = processGroup;
    if (lastSlash != -1) {
        serviceName = processGroup.mid(lastSlash + 1);
    }

    const QRegularExpressionMatch &appIdMatch = s_appIdFromProcessGroupPattern.match(serviceName);

    if (!appIdMatch.isValid() || !appIdMatch.hasMatch()) {
        // create a transient service object just to have a sensible name
        return KService::Ptr(new KService(serviceName, QString(), QString()));
    }

    const QString appId = unescapeName(appIdMatch.captured(2));

    KService::Ptr service = KService::serviceByMenuId(appId + QStringLiteral(".desktop"));
    if (!service && processGroup.endsWith(QLatin1String("@autostart.service"))) {
        auto file = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String("autostart/%1.desktop").arg(appId));
        if (!file.isEmpty()) {
            service = new KService(file);
        }
    }
    if (!service) {
        service = new KService(appId, QString(), QString());
    }

    return service;
}

QString CGroup::cgroupSysBasePath()
{
    return s_cGroupSystemInformation->sysGgroupRoot;
}

CGroupSystemInformation::CGroupSystemInformation()
{
    QDir base(QStringLiteral("/sys/fs/cgroup"));
    if (base.exists(QLatin1String("unified"))) {
        sysGgroupRoot = base.absoluteFilePath(QStringLiteral("unified"));
        return;
    }
    if (base.exists()) {
        sysGgroupRoot = base.absolutePath();
    }
}
