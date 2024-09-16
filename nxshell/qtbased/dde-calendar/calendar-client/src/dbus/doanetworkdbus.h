// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOANETWORKDBUS_H
#define DOANETWORKDBUS_H

#include <QDBusAbstractInterface>

#define NETWORK_DBUS_INTEERFACENAME "org.deepin.dde.Network1"
#define NETWORK_DBUS_NAME "org.deepin.dde.Network1"
#define NETWORK_DBUS_PATH "/org/deepin/dde/Network1"

class DOANetWorkDBus : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit DOANetWorkDBus(QObject *parent = nullptr);



    enum NetWorkState {
        Active = 1, //已连接
        Disconnect, //已断开
        Connecting, //连接中
        unknow      //未知
    };
    Q_ENUM(NetWorkState)

    /**
     * @brief getNetWorkState           获取网络状态
     * @return
     */
    DOANetWorkDBus::NetWorkState getNetWorkState();

signals:
    void sign_NetWorkChange(DOANetWorkDBus::NetWorkState);

public slots:
    void propertiesChanged(const QDBusMessage &msg);
private:
    QVariant getPropertyByName(const char *porpertyName);
};

#endif // DOANETWORKDBUS_H
