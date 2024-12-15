// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "doanetworkdbus.h"
#include "commondef.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

DOANetWorkDBus::DOANetWorkDBus(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_NAME, NETWORK_DBUS_PATH, NETWORK_DBUS_INTEERFACENAME, QDBusConnection::sessionBus(), parent)
{
    if (!this->isValid()) {
        qCWarning(ClientLogger) << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
    }

    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as", this, SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(ClientLogger) << "the connection was fail!";
    }
}

/**
 * @brief getUserName           获取用户名
 * @return
 */
DOANetWorkDBus::NetWorkState DOANetWorkDBus::getNetWorkState()
{
    return getPropertyByName("State").toInt() == 70 ? DOANetWorkDBus::Active : DOANetWorkDBus::Disconnect;
}

//根据属性名称获取对应属性值
QVariant DOANetWorkDBus::getPropertyByName(const char *porpertyName)
{
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return dbusinterface.property(porpertyName);
}

//监听服务对象信号
void DOANetWorkDBus::propertiesChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    // 参数固定长度
    if (3 != arguments.count())
        return;
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != this->interface())
        return;
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "State") {
            int state = changedProps[prop].toInt();
            if(70 == state){
               emit sign_NetWorkChange(DOANetWorkDBus::Active);
            }else if(20 == state){
                emit sign_NetWorkChange(DOANetWorkDBus::Disconnect);
            }

        }
    }
}
