// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launcher1compat.h"

#include "pkutils.h"

#include <DDesktopEntry>
#include <DNotifySender>
#include <launcher1adaptor.h> // this is the adapter of daemon.Launcher1

// PackageKit-Qt
#include <Daemon>

// Ends with a slash, this is the install prefix for the trusted launcher app.
// For package maintianers, if your distro install binaries to weird locations,
// you can patch this to a empty string.
// TODO: better approach to check if caller is trusted?
//       pr do we really needs to check the caller?
#define BINDIR_PREFIX "/usr/bin/"

DCORE_USE_NAMESPACE

Launcher1Compat::Launcher1Compat(QObject *parent)
    : QObject(parent)
    , m_daemonLauncher1Adapter(new Launcher1Adaptor(this))
{
    PackageKit::Daemon::setHints(QStringList{"interactive=true"});
}

Launcher1Compat::~Launcher1Compat()
{
    // TODO
}

void sendNotification(const QString & displayName, bool successed)
{
    QString msg;
    if (successed) {
        msg = QString(QObject::tr("%1 removed successfully").arg(displayName));
    } else {
        msg = QString(QObject::tr("Failed to remove the app"));
    }

    DUtil::DNotifySender notifySender(msg);
    notifySender = notifySender.appName("deepin-app-store").appIcon("application-default-icon").timeOut(5000);
    notifySender.call();
}

bool uninstallLinglongBundle(const DDesktopEntry & entry)
{
    const QString appId = entry.rawValue("Exec").section(' ', 2, 2);
    const QStringList args {"uninstall", appId};
    QProcess process;

    int retCode = QProcess::execute("ll-cli", args);
    return retCode == 0;
}

void postUninstallCleanUp(const QString & desktopId)
{
    // Remove the shortcut that we created at user's desktop
    const QString &curDesktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString &appDesktopPath = curDesktop + "/" + desktopId;

    QFile file(appDesktopPath);
    if (file.exists()) {
        file.remove();
    } else {
        qDebug() << appDesktopPath << "doesn't exist, no need to remove.";
    }

    // Remove the autostart entry
    // TODO: the legacy dde-application-manager didn't do this

    // Remove the pinned dock entry
    // TODO: the legacy dde-application-manager didn't do this
}

void Launcher1Compat::uninstallPackageKitPackage(const QString & pkgDisplayName, const QString & pkPackageId)
{
    qDebug() << "Uninstall" << pkPackageId << "via PackageKit";
    PKUtils::removePackage(pkPackageId).then([=](){
        sendNotification(pkgDisplayName, true);
        QFileInfo fi(m_desktopFilePath);
        // FIXME: THIS IS NOT DESKTOP ID
        postUninstallCleanUp(fi.fileName());
    }, [=](const std::exception & e){
        sendNotification(pkgDisplayName, false);
        PKUtils::PkError::printException(e);
    });
}

void Launcher1Compat::onHandleLastoreUninstall(const QDBusMessage &message)
{
    QList<QVariant> arguments = message.arguments();

    if (3 != arguments.count())
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    const QStringList keys = changedProps.keys();

    QString status;
    if (keys.contains("Status")) {
        status = changedProps["Status"].toString();
    }

#ifdef QT_DEBUG
    qInfo() <<  "changedProps: " << changedProps << ", status: " << status;
#endif
    if (status == "succeed" || status == "end") {
        QFileInfo fi(m_desktopFilePath);
        // FIXME: THIS IS NOT DESKTOP ID
        postUninstallCleanUp(fi.fileName());
        emit UninstallSuccess(m_desktopFilePath);
    } else if (status == "failed") {
        emit UninstallFailed(m_desktopFilePath, QString());
    }
}

// the 1st argument is the full path of a desktop file.
void Launcher1Compat::RequestUninstall(const QString & desktop, bool unused)
{
    Q_UNUSED(unused)

    // TODO: If we go with packagekit, it will ask user to input the password to uninstall application.
    //       Thus this checking will be no longer necessary. We still need this check since we are
    //       still using lastore to uninstall package if it exists.
    QString servicePid = QString::number(QDBusConnection::sessionBus().interface()->servicePid(message().service()));
    QString procfs = QLatin1String("/proc/%1/cmdline").arg(servicePid);
    QFile procfile(procfs);
    if (!procfile.exists() || !procfile.open(QFile::ReadOnly)) {
        qDebug() << "Cannot verify caller";
        return;
    }
    QString cmdline(procfile.readAll());
    procfile.close();
    qDebug() << cmdline;
#ifndef QT_DEBUG
    if (!QString(cmdline).endsWith(BINDIR_PREFIX + QStringLiteral("dde-shell")) &&
        !QString(cmdline).endsWith(BINDIR_PREFIX + QStringLiteral("dde-launchpad"))) {
        qWarning() << cmdline << " has no right to uninstall " << desktop;
        return;
    }
#endif // !QT_DEBUG

    m_desktopFilePath = desktop;

    // Check if passed file is valid
    QFileInfo desktopFileInfo(desktop);
    if (!desktopFileInfo.exists()) {
        qDebug() << "File" << desktop << "doesn't exist.";
        return;
    }

    QString desktopFilePath(desktopFileInfo.isSymLink() ? desktopFileInfo.symLinkTarget() : desktop);
    DDesktopEntry desktopEntry(desktopFilePath);
    if (desktopEntry.status() != DDesktopEntry::NoError) {
        qDebug() << "Desktop file" << desktop << "is invalid.";
        return;
    }

    // Check and do uninstallation
    if (desktopFilePath.contains("/persistent/linglong") || desktopFilePath.contains("/var/lib/linglong")) {
        // Uninstall Linglong Bundle
        bool succ = uninstallLinglongBundle(desktopEntry);
        if (!succ) {
            emit UninstallFailed(desktopFilePath, QString());
            sendNotification(desktopEntry.name(), false);
        } else {
            // FIXME: the filename of the desktop file MIGHT NOT be its desktopId in freedesktop spec.
            //        here is the logic from the legacy dde-application-manager which is INCORRECT in that case.
            QFileInfo fileInfo(desktopFilePath);
            postUninstallCleanUp(fileInfo.fileName());
            emit UninstallSuccess(desktopFilePath);
            sendNotification(desktopEntry.name(), true);
        }
    } else {
        m_packageDisplayName = desktopEntry.name();
        // TODO: check if it's a flatpak or snap bundle and do the uninstallation?
        // Uninstall regular package via PackageKit or deepin-store
        PKUtils::searchFiles(desktopFilePath, PackageKit::Transaction::FilterInstalled).then([this](const PKUtils::PkPackages packages) {
            if (packages.size() == 0) {
                qDebug() << "No matching package found";
                return;
            }
            for (const PKUtils::PkPackage & pkg : packages) {
                QString pkgId;
                std::tie(std::ignore, pkgId, std::ignore) = pkg;
                uninstallPackageKitPackage(m_packageDisplayName, pkgId);
            }
        }, [](const std::exception & e){
            PKUtils::PkError::printException(e);
        });
    }
}
