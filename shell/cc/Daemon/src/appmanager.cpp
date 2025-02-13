/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 */

#include "appmanager.h"
#include "appmanageradaptor.h"

#include <QDBusInterface>
#include <QDebug>

AppManager::AppManager(QObject *parent)
    : QObject(parent)
    , m_backend(new QApt::Backend(this))
    , m_trans(nullptr)
{
    m_backend->init();

    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.registerService("com.lingmo.Daemon")) {
        qDebug() << "Cannot register D-Bus service";
    }

    if (!connection.registerObject("/AppManager", this)) {
        qDebug() << "Cannot register object";
    }

    new AppManagerAdaptor(this);
}

void AppManager::uninstall(const QString &content)
{
    QApt::Package *package = m_backend->packageForFile(content);
    QString packageName;

    if (package) {
        packageName = package->name();

        for (const QString &item : package->requiredByList()) {
            QApt::Package *p = m_backend->package(item);

            if (!p || !p->isInstalled())
                continue;

            if (p->recommendsList().contains(packageName))
                continue;

            if (p->suggestsList().contains(packageName))
                continue;

            if (m_backend->package(item)) {
                m_backend->package(item)->setPurge();
            }
        }

        m_trans = m_backend->removePackages({package});
        m_backend->commitChanges();

        connect(m_trans, &QApt::Transaction::statusChanged, this, [=] (QApt::TransactionStatus status) {
            if (status == QApt::TransactionStatus::FinishedStatus) {
                notifyUninstallSuccess(packageName);
            } else if (status == QApt::TransactionStatus::WaitingStatus) {
                notifyUninstalling(packageName);
            }
        });

        m_trans->run();
    } else {
        notifyUninstallFailure(packageName);
    }
}

void AppManager::notifyUninstalling(const QString &packageName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QList<QVariant> args;
        args << "lingmo-daemon";
        args << ((unsigned int) 0);
        args << "lingmo-installer";
        args << packageName;
        args << tr("Uninstalling");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }
}

void AppManager::notifyUninstallFailure(const QString &packageName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QList<QVariant> args;
        args << "lingmo-daemon";
        args << ((unsigned int) 0);
        args << "dialog-error";
        args << packageName;
        args << tr("Uninstallation failure");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }
}

void AppManager::notifyUninstallSuccess(const QString &packageName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QList<QVariant> args;
        args << "lingmo-daemon";
        args << ((unsigned int) 0);
        args << "process-completed-symbolic";
        args << packageName;
        args << tr("Uninstallation successful");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }

    if (m_trans) {
        m_trans->cancel();
        m_trans->deleteLater();
        m_trans = nullptr;
    }
}
