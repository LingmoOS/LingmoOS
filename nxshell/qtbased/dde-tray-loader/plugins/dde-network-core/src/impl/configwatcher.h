// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGWATCHER_H
#define CONFIGWATCHER_H

#include "networkconst.h"

#include <QObject>

class ConfigWatcher : public QObject
{
    Q_OBJECT

public:
    enum class ModuleState {
        Hidden = 0,
        Enabled,
        Disabled
    };

public:
    static ConfigWatcher *instance();
    bool enableAirplaneMode() const;
    bool wpa3EnterpriseVisible() const;
    dde::network::ProxyMethod proxyMethod() const;
    void setProxyMethod(const dde::network::ProxyMethod &method);
    ModuleState wirelessState() const;
    bool airplaneModeEnabled() const;
    void setAirplaneModeEnabled(bool enabled);
    int wirelessScanInterval() const;
    bool enableAccountNetwork() const;

signals:
    void enableAirplaneModeChanged(bool);
    void wpa3EnterpriseVisibleChanged(bool);
    void lastProxyMethodChanged(dde::network::ProxyMethod);
    void wirelessStateChanged(ModuleState);
    void airplaneModeEnabledChanged(bool);
    void wirelessScanIntervalChanged(int);

private:
    explicit ConfigWatcher(QObject *parent = nullptr);
    ~ConfigWatcher();

private slots:
    void onValueChanged(const QString &key);
    void onDccConfigChanged(const QString &key);
    void onAirplaneModeChanged(const QString &key);

private:
    QString m_lastProxyMethod;
    bool m_networkAirplaneMode;
    bool m_wpa3EnterpriseVisible;
    int m_wirelessScanInterval;
    QString m_wirelessState;
    bool m_airplaneModeEnabled;
    bool m_enableAccountNetwork;
};

#endif // CONFIGSETTING_H
