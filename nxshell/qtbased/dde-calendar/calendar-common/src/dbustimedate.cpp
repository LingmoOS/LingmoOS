// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbustimedate.h"
#include "commondef.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

#define NETWORK_DBUS_INTEERFACENAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_NAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_PATH "/com/deepin/daemon/Timedate"

DBusTimedate::DBusTimedate(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_NAME, NETWORK_DBUS_PATH, NETWORK_DBUS_INTEERFACENAME, QDBusConnection::sessionBus(), parent)
{
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(NETWORK_DBUS_NAME,
                                               NETWORK_DBUS_PATH,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(ClientLogger) << "the PropertiesChanged was fail!";
        qCWarning(ClientLogger) << this->lastError();
    }

    m_hasDateTimeFormat = getHasDateTimeFormat();
}

int DBusTimedate::shortTimeFormat()
{
    //如果存在对应的时间设置则获取，否则默认为4
    return m_hasDateTimeFormat ? getPropertyByName("ShortTimeFormat").toInt() : 4;
}

int DBusTimedate::shortDateFormat()
{
    //如果存在对应的时间设置则获取，否则默认为1
    return m_hasDateTimeFormat ? getPropertyByName("ShortDateFormat").toInt() : 1;
}

Qt::DayOfWeek DBusTimedate::weekBegins()
{
    if (m_hasDateTimeFormat) {
        // WeekBegins是从0开始的，加1才能对应DayOfWeek
        return Qt::DayOfWeek(getPropertyByName("WeekBegins").toInt() + 1);
    }
    return Qt::Monday;
}

void DBusTimedate::propertiesChanged(const QDBusMessage &msg)
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
        if (prop == "ShortTimeFormat") {
            emit ShortTimeFormatChanged(changedProps[prop].toInt());
        } else if (prop == "ShortDateFormat") {
            emit ShortDateFormatChanged(changedProps[prop].toInt());
        }
    }
}

QVariant DBusTimedate::getPropertyByName(const char *porpertyName)
{
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return dbusinterface.property(porpertyName);
}

bool DBusTimedate::getHasDateTimeFormat()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(NETWORK_DBUS_NAME,
                                                      NETWORK_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = reply.arguments().first();
        return variant.toString().contains("\"ShortDateFormat\"");
    } else {
        return false;
    }
}
