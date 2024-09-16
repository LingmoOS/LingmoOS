// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsgapplication.h"

#include <sys/syscall.h>
#include <unistd.h>

#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDBusUnixFileDescriptor>
#include <QDBusReply>
#include <QRegularExpression>
#include <QLoggingCategory>
#include <QDBusConnectionInterface>

#include <DDBusInterface>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg")
#else
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg", QtInfoMsg)
#endif

DCORE_BEGIN_NAMESPACE

static inline QByteArray getSelfAppId() {
    // The env is set by the application starter(eg, org.desktopspec.ApplicationManager service)
    QByteArray selfId = qgetenv("DSG_APP_ID");
    if (!selfId.isEmpty())
        return selfId;
    return DSGApplication::getId(QCoreApplication::applicationPid());
}

static bool isServiceActivatable(const QString &service)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(service))
        return false;

     const QDBusReply<QStringList> activatableNames = QDBusConnection::sessionBus().interface()->
             callWithArgumentList(QDBus::AutoDetect,
             QLatin1String("ListActivatableNames"),
             QList<QVariant>());

     return activatableNames.value().contains(service);
}

// Format appId to valid.
static QByteArray formatAppId(const QByteArray &appId)
{
    static const QRegularExpression regex("[^\\w\\-\\.]");
    QString format(appId);
    format.replace(QDir::separator(), ".");
    format = format.replace(regex, QStringLiteral("-"));
    const QString InvalidPrefix{"."};
    if (format.startsWith(InvalidPrefix))
        format = format.mid(InvalidPrefix.size());
    return format.toLocal8Bit();
}

QByteArray DSGApplication::id()
{
    static QByteArray selfId = getSelfAppId();
    if (!selfId.isEmpty())
        return selfId;
    QByteArray result = selfId;
    if (!qEnvironmentVariableIsSet("DTK_DISABLED_FALLBACK_APPID")) {
        result = QCoreApplication::applicationName().toLocal8Bit();
        if (result.isEmpty()) {
            QFile file("/proc/self/cmdline");
            if (file.open(QIODevice::ReadOnly))
                result = file.readLine();
        }
        if (result.isEmpty()) {
            const QFileInfo file(QFile::symLinkTarget("/proc/self/exe"));
            if (file.exists())
                result = file.absoluteFilePath().toLocal8Bit();
        }
        if (!result.isEmpty()) {
            result = formatAppId(result);
            qCDebug(dsgApp) << "The applicatiion ID is fallback to " << result;
        }
    }
    if (result.isEmpty())
        qCWarning(dsgApp) << "The application ID is empty.";

    return result;
}

QByteArray DSGApplication::getId(qint64 pid)
{
    if (!isServiceActivatable("org.desktopspec.ApplicationManager1")) {
        qCInfo(dsgApp) << "Can't getId from AM for the " << pid << ", because AM is unavailable.";
        return QByteArray();
    }

    int pidfd = syscall(SYS_pidfd_open, pid, 0);
    if (pidfd < 0) {
        qCWarning(dsgApp) << "pidfd open failed:" << strerror(errno);
        return QByteArray();
    }

    DDBusInterface infc("org.desktopspec.ApplicationManager1",
                        "/org/desktopspec/ApplicationManager1",
                        "org.desktopspec.ApplicationManager1");

    QDBusReply<QString> reply = infc.call("Identify", QVariant::fromValue(QDBusUnixFileDescriptor(pidfd)));

    if (!reply.isValid()) {
        qCWarning(dsgApp) << "Identify from AM failed." << reply.error().message();
        return QByteArray();
    }

    const QByteArray appId = reply.value().toLatin1();
    qCInfo(dsgApp) << "AppId is fetched from AM, and value is " << appId;
    return appId;
}

DCORE_END_NAMESPACE
