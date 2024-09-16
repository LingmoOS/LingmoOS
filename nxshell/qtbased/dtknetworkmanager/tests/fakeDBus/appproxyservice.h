// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPPROXYSERVICE_H
#define APPPROXYSERVICE_H

#include <QDBusObjectPath>

class FakeAppProxyService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeNetwork.ProxyChains")
public:
    explicit FakeAppProxyService(QObject *parent = nullptr);
    ~FakeAppProxyService() override;

    Q_PROPERTY(QString IP READ IP)
    Q_PROPERTY(QString Password READ password)
    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(QString User READ user)
    Q_PROPERTY(quint32 Port READ port)

    QString m_IP{"127.0.0.1"};
    QString m_password{"123456789"};
    QString m_type{"http"};
    QString m_user{"deepin"};
    quint32 m_port{1080};

    QString IP() const { return m_IP; };
    QString password() const { return m_password; };
    QString type() const { return m_type; };
    QString user() const { return m_user; };
    quint32 port() const { return m_port; };

    bool m_SetTrigger{false};
public Q_SLOTS:
    Q_SCRIPTABLE void Set(QString, QString, quint32, QString, QString) { m_SetTrigger = true; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.daemon.FakeNetwork"};
    const QString Path{"/com/deepin/daemon/FakeNetwork/ProxyChains"};
};

#endif
