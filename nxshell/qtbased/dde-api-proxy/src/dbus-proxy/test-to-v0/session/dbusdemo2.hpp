// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "com_deepin_dbusdemo.h"

#include "session/dbusdemo2sub.hpp"

class DBusProxy2 : public DBusProxyBase {
public:
    DBusProxy2(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        registerAreaListMetaType();
        InitFilterProperies(QStringList());
        InitFilterMethods(QStringList({"Hello", "SetArea"}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface * initConnect() {
        m_dbusProxy = new com::deepin::dbusdemo(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        connect(m_dbusProxy, &com::deepin::dbusdemo::Tick2, this, [this](QString ret){
            qInfo() << "Tick2:" << ret;
            sub = new DBusProxy2Sub(m_dbusName, m_dbusPath, m_dbusInterface, m_proxyDbusName, "/com/test/dbusdemo/sub1", "com.test.dbusdemo.sub1", m_dbusType);
            QDBusMessage msg = QDBusMessage::createSignal(m_proxyDbusPath, m_proxyDbusInterface, "Tick2");
            QList<QVariant> arguments;
            arguments.push_back(ret);
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).send(msg);
        });
        connect(m_dbusProxy, &com::deepin::dbusdemo::AreaTestSig, this, [this](const QString &id, AreaList areaList){
            qInfo() << "AreaTestSig:" << id;
            sub->ServiceStop();
            delete sub;
            QDBusMessage msg = QDBusMessage::createSignal(m_proxyDbusPath, m_proxyDbusInterface, "AreaTestSig");
            QList<QVariant> arguments;
            arguments.push_back(id);
            QVariant areas;
            areas.setValue(areaList);
            arguments.push_back(areas);
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).send(msg);
        });
    }
private:
    com::deepin::dbusdemo *m_dbusProxy;
    DBusProxyBase *sub;
};


