// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "session_org_deepin_dde_ControlCenter1.h"
#include "session_org_deepin_dde_ControlCenter1_GrandSearch.h"

class SessionControlCenter1Proxy : public DBusProxyBase {
public:
    SessionControlCenter1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr) 
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        // InitFilterProperies(QStringList({}));
        // InitFilterMethods(QStringList({"Show"})); // 另外代理了"ShowModule", "ShowPage"，在handleMessageCustom处理
        ServiceStart();
    }
    virtual	bool handleMessageCustom(const QDBusMessage &message, const QDBusConnection &connection)
    {
        // 对比V20，V23不仅服务名变了，方法也有变动，所以不能统一流程转发，需要自定义处理
        if (message.interface() == m_proxyDbusInterface) {
            if (message.member() == "ShowModule") {
                // V23去掉了ShowModule，改用ShowPage
                QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList("ShowPage", message.arguments());
                call.waitForFinished();
                connection.send(message.createReply(call.reply().arguments()));
                return true;
            } else if (message.member() == "ShowPage") {
                // V23的ShowPage参数从两个变成一个, 兼容v20是两个参数的情况
                if (message.arguments().size() == 1) {
                    QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList("ShowPage", message.arguments());
                    call.waitForFinished();
                    connection.send(message.createReply(call.reply().arguments()));
                } else if (message.arguments().size() == 2) {
                    QString param = message.arguments().at(0).toString();
                    if (message.arguments().at(1).toString() != "") {
                        param = param + "/" + message.arguments().at(1).toString();
                    }
                    QList<QVariant> arguments;
                    arguments << param;
                    QDBusPendingCall call = m_dbusProxy->asyncCallWithArgumentList("ShowPage", arguments);
                    call.waitForFinished();
                    connection.send(message.createReply(call.reply().arguments()));
                } else {
                    connection.send(message.createErrorReply("com.deepin.dde.error.ParamError", "param error"));
                }
                return true;
            }
        }
        // 目前的Proxy封装，对于同PATH下不同Interface没有统一处理，所以在这里处理com.deepin.dde.ControlCenter.GrandSearch代理
        if (message.interface() == "com.deepin.dde.ControlCenter.GrandSearch") {
            QStringList updaterFilterMethods{"Action", "Search", "Stop"};
            // if (!updaterFilterMethods.contains(message.member())) {
            //     qInfo() << "com.deepin.dde.ControlCenter.GrandSearch" << "method-filter:" << message.member() << "is not allowed.";
            //     connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "is not allowed"));
            //     return true;
            // }
            QDBusPendingCall call = m_dbusGrandSearchProxy->asyncCallWithArgumentList(message.member(), message.arguments());
            call.waitForFinished();
            connection.send(message.createReply(call.reply().arguments()));
            return true;
        }
        return false;
    }

    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::ControlCenter1(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        m_dbusGrandSearchProxy = new org::deepin::dde::controlcenter1::GrandSearch(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }
private:
    org::deepin::dde::ControlCenter1 *m_dbusProxy;
    org::deepin::dde::controlcenter1::GrandSearch *m_dbusGrandSearchProxy;
};


