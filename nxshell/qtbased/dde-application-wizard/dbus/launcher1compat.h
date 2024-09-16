// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusContext>
#include <QDBusMessage>
#include <QObject>

class Launcher1Adaptor;
class Launcher1Compat : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    static Launcher1Compat &instance()
    {
        static Launcher1Compat _instance;
        return _instance;
    }
    ~Launcher1Compat();

// Launcher1Adapter
public:
    void RequestUninstall(const QString &desktop, bool unused);

signals:
    void UninstallFailed(const QString &appId, const QString &errMsg);
    void UninstallSuccess(const QString &appID);

private:
    explicit Launcher1Compat(QObject *parent = nullptr);

    void uninstallPackageKitPackage(const QString & pkgDisplayName, const QString & pkPackageId);
    void onHandleLastoreUninstall(const QDBusMessage &message);

    Launcher1Adaptor * m_daemonLauncher1Adapter;

    // TODO: vvv This is bad, refactor this later vvv
    QString m_packageDisplayName;
    QString m_desktopFilePath;
    // TODO: ^^^ -------------------------------- ^^^
};
