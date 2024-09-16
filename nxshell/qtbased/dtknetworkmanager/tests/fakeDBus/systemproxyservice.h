// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMPROXYSERVICE_H
#define SYSTEMPROXYSERVICE_H

#include <QDBusObjectPath>

class FakeSystemProxyService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeNetwork.Network")
public:
    explicit FakeSystemProxyService(QObject *parent = nullptr);
    ~FakeSystemProxyService() override;

    bool m_getAutoProxyTrigger{false};
    bool m_getProxyTrigger{false};
    bool m_getProxyIgnoreHostsTrigger{false};
    bool m_getProxyMethodTrigger{false};
    bool m_setAutoProxyTrigger{false};
    bool m_setProxyTrigger{false};
    bool m_setProxyIgnoreHostsTrigger{false};
    bool m_setProxyMethodTrigger{false};
public Q_SLOTS:
    Q_SCRIPTABLE QString GetAutoProxy()
    {
        m_getAutoProxyTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QString GetProxy(const QString &, QString &)
    {
        m_getProxyTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QString GetProxyIgnoreHosts()
    {
        m_getProxyIgnoreHostsTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QString GetProxyMethod()
    {
        m_getProxyMethodTrigger = true;
        return "auto";
    };

    Q_SCRIPTABLE void SetAutoProxy(const QString &) { m_setAutoProxyTrigger = true; }

    Q_SCRIPTABLE void SetProxy(const QString &, const QString &, const QString &) { m_setProxyTrigger = true; }

    Q_SCRIPTABLE void SetProxyIgnoreHosts(const QString &) { m_setProxyIgnoreHostsTrigger = true; }

    Q_SCRIPTABLE void SetProxyMethod(const QString &) { m_setProxyMethodTrigger = true; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.daemon.FakeNetwork"};
    const QString Path{"/com/deepin/daemon/FakeNetwork/Network"};
};

#endif
