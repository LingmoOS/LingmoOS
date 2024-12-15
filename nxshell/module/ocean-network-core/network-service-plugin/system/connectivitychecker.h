// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTIVITYCHECKER_H
#define CONNECTIVITYCHECKER_H

#include "constants.h"

#include <QMap>
#include <QObject>

class QTimer;
class QProcess;

namespace NetworkManager {
class Device;
class WirelessNetwork;
class ActiveConnection;
} // namespace NetworkManager

namespace network {
namespace systemservice {

class SystemIPConflict;

// 网络连通性的检测
class ConnectivityChecker : public QObject
{
    Q_OBJECT

public:
    ConnectivityChecker(QObject *parent = Q_NULLPTR);
    virtual ~ConnectivityChecker();

    virtual network::service::Connectivity connectivity() const = 0;

    virtual QString portalUrl() const { return ""; }

    virtual void checkConnectivity() {}

signals:
    void connectivityChanged(const network::service::Connectivity &);
};

class LocalConnectionvityChecker : public ConnectivityChecker
{
    Q_OBJECT

public:
    LocalConnectionvityChecker(SystemIPConflict *ipHandler, QObject *parent = Q_NULLPTR);
    ~LocalConnectionvityChecker() override;

    network::service::Connectivity connectivity() const override;
    QString portalUrl() const override;
    void checkConnectivity() override;

signals:
    void portalDetected(const QString &);

private:
    void initDeviceConnect(const QList<QSharedPointer<NetworkManager::Device>> &devices);
    void initConnectivityChecker();
    network::service::Connectivity getDefaultLimitConnectivity() const;
    void setConnectivity(const network::service::Connectivity &connectivity);
    void setPortalUrl(const QString &portalUrl);
    void initDefaultConnectivity();

private slots:
    void onUpdataActiveState(const QSharedPointer<NetworkManager::ActiveConnection> &networks);
    void onUpdateUrls(const QStringList &urls);
    void startCheck();
    void onFinished(int exitCode);
    void clearProcess();
    void onActiveConnectionChanged();

private:
    QTimer *m_checkTimer;
    QTimer *m_timer;
    QList<QMetaObject::Connection> m_checkerConnection;
    network::service::Connectivity m_connectivity;
    int m_checkCount;
    SystemIPConflict *m_ipConfilctHandler;
    QString m_portalUrl;
    QMap<QString, QProcess *> m_checkUrls;
    qint64 m_lastOpenUrlTime;
};

class NMConnectionvityChecker : public ConnectivityChecker
{
    Q_OBJECT

public:
    NMConnectionvityChecker(QObject *parent = Q_NULLPTR);
    ~NMConnectionvityChecker() override;
    network::service::Connectivity connectivity() const override;

private:
    void initMember();
    void initConnection();

private:
    network::service::Connectivity m_connectivity;
};

} // namespace systemservice

} // namespace network

#endif // CONNECTIVITYCHECKER_H
