// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

#include <QDebug>

Service::Service(QObject *parent)
    : QDBusService(parent)
{
    initPolicy(QDBusConnection::SystemBus, QString(SERVICE_CONFIG_DIR) + "other/sdk-demo.json");
}

QString Service::Hello()
{
    qInfo() << "Service::Hello called.";

    Q_EMIT dbusLog("Call:Hello  Reply:World");

    return "World";
}

bool Service::Register(const QString &id, QString &result2)
{
    qInfo() << "Service::Register called. idd:" << id;

    if (id == "error") {
        sendErrorReply("com.deepin.dbusdemo.RegisterNotSupported",
                       "The method call 'Register()' is not supported");
    }
    result2 = "reply2";
    return true;
}

QString Service::Msg()
{
    qInfo() << "Service::Msg";
    return m_msg;
}

void Service::SetMsg(QString value)
{
    qInfo() << "Service::SetMsg value:" << value;
    m_msg = value;
}
