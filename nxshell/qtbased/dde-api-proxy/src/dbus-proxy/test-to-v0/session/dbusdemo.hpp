// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDBusVirtualObject>
#include <QDBusAbstractInterface>

#include "com_deepin_dbusdemo.h"

class DBusProxy : public QDBusVirtualObject {
public:
    DBusProxy(QObject *parent = nullptr) :QDBusVirtualObject(parent)
    {
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.dbusdemo").registerService("com.test.dbusdemo");
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.dbusdemo").registerVirtualObject("/com/test/dbusdemo", this);
        m_dbusProxy = new com::deepin::dbusdemo("com.deepin.dbusdemo", "/com/deepin/dbusdemo", QDBusConnection::sessionBus(), this);
        signalMonitor();
    }
    virtual	~DBusProxy(){}
    virtual bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
    {
        qInfo() << "asdsad";
        //     inline QDBusPendingReply<QString> GetSoundFile(const QString &name)
        if (message.interface() != "org.freedesktop.DBus.Properties" && message.member() == "Hello") {
            //  qInfo() << "GetSoundFile:" << message.arguments().at(0).toString();
//            QDBusReply<QString> reply = m_dbusProxy->Hello();
//            qInfo() << "reply:" << reply.value();
//            connection.send(message.createReply(reply.value()));
//            return true;

            QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList(QStringLiteral("Hello"), message.arguments());
            call.waitForFinished();
            connection.send(message.createReply(call.reply().arguments()));
            return true;
        } else if (message.interface() != "org.freedesktop.DBus.Properties" && message.member() == "SetArea") {
            //  SetArea(const QString &id, AreaList area)
//            qInfo() << "SetArea id:" << message.arguments().at(0).toString();
//            AreaList areas;
//            message.arguments().at(1).value<QDBusArgument>() >> areas;
//            qInfo() << "SetArea area:" <<  areas.size();
//            QDBusReply<bool> reply = m_dbusProxy->SetArea(message.arguments().at(0).toString(), areas);
//            qInfo() << "reply:" << reply.value();
//            connection.send(message.createReply(reply.value()));
//            return true;

            QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList(QStringLiteral("SetArea"), message.arguments());
            call.waitForFinished();
            connection.send(message.createReply(call.reply().arguments()));
            return true;
        } else if (message.interface() == "org.freedesktop.DBus.Properties"  && message.member() == "Get") {
            qInfo() << "Get:" <<message.arguments().at(1).toString();
            QVariant var = m_dbusProxy->property(message.arguments().at(1).toString().toStdString().c_str());
            connection.send(message.createReply(var));
            return true;
        } else if (message.interface() == "org.freedesktop.DBus.Properties"  && message.member() == "Set") {
            m_dbusProxy->setProperty(message.arguments().at(1).toString().toStdString().c_str(), message.arguments().at(2));
            connection.send(message.createReply());
            return true;
        }
        // connection.send(message.createErrorReply("com.error", "error"));
        return false;
    }
    virtual QString	introspect(const QString &path) const
    {
        return "";
    }
    void signalMonitor()
    {
        connect(m_dbusProxy, &com::deepin::dbusdemo::Tick2, this, [](QString ret){
            qInfo() << "Tick2:" << ret;
            QDBusMessage msg = QDBusMessage::createSignal("/com/test/dbusdemo", "com.test.dbusdemo", "Tick2");
            QList<QVariant> arguments;
            arguments.push_back(ret);
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.dbusdemo").send(msg);
        });
        connect(m_dbusProxy, &com::deepin::dbusdemo::AreaTestSig, this, [](const QString &id, AreaList areaList){
            qInfo() << "AreaTestSig:" << id;
            QDBusMessage msg = QDBusMessage::createSignal("/com/test/dbusdemo", "com.test.dbusdemo", "AreaTestSig");
            QList<QVariant> arguments;
            arguments.push_back(id);
            QVariant areas;
            areas.setValue(areaList);
            arguments.push_back(areas);
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.dbusdemo").send(msg);

        });
        connect(m_dbusProxy, &com::deepin::dbusdemo::propertyChanged, this, [](const QString &propName, const QVariant &value){
            qInfo() << "propertyChanged:" << propName << value;
            QDBusMessage msg = QDBusMessage::createSignal("/com/test/dbusdemo", "org.freedesktop.DBus.Properties", "PropertiesChanged");
            QList<QVariant> arguments;
            arguments.push_back(QStringLiteral("com.test.dbusdemo"));
            QVariantMap changedProps;
            changedProps.insert(propName, value);
            arguments.push_back(changedProps);
            arguments.push_back(QStringList());
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(QDBusConnection::SessionBus, "com.test.dbusdemo").send(msg);
        });
    }

private:
    com::deepin::dbusdemo *m_dbusProxy;
};


