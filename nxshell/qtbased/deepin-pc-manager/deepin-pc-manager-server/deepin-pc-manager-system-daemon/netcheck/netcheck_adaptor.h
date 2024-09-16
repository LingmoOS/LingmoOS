// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETCHECK_ADAPTOR_H
#define NETCHECK_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "../deepin-pc-manager-session-daemon/window/modules/common/defenderprocinfolist.h"
#include "../../deepin-pc-manager/src/window/modules/common/common.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.pc.manager.netcheck
 */
class NetcheckAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.pc.manager.netcheck")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.pc.manager.netcheck\">\n"
"    <property access=\"read\" type=\"b\" name=\"Availabled\"/>\n"
"    <method name=\"StartApp\"/>\n"
"    <method name=\"ExitApp\"/>\n"
"    <method name=\"StartCheckNetDevice\"/>\n"
"    <signal name=\"NotifyCheckNetDevice\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"StartCheckNetConnSetting\"/>\n"
"    <signal name=\"NotifyCheckNetConnSetting\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"StartCheckNetDHCP\"/>\n"
"    <signal name=\"NotifyCheckNetDHCP\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"StartCheckNetDNS\"/>\n"
"    <signal name=\"NotifyCheckNetDNS\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"StartCheckNetHost\"/>\n"
"    <signal name=\"NotifyCheckNetHost\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"StartCheckNetConn\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"url\"/>\n"
"    </method>\n"
"    <signal name=\"NotifyCheckNetConn\">\n"
"      <arg type=\"i\" name=\"result\"/>\n"
"    </signal>\n"
"    <method name=\"GetNetCheckStatus\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"status\"/>\n"
"    </method>\n"
"    <method name=\"SetNetCheckStatus\">\n"
"      <arg direction=\"in\" type=\"b\" name=\"status\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    NetcheckAdaptor(QObject *parent);
    virtual ~NetcheckAdaptor();

public: // PROPERTIES
    Q_PROPERTY(bool Availabled READ availabled)
    bool availabled() const;

public Q_SLOTS: // METHODS
    void ExitApp();
    bool GetNetCheckStatus();
    void SetNetCheckStatus(bool status);
    void StartApp();
    void StartCheckNetConn(const QString &url);
    void StartCheckNetConnSetting();
    void StartCheckNetDHCP();
    void StartCheckNetDNS();
    void StartCheckNetDevice();
    void StartCheckNetHost();
Q_SIGNALS: // SIGNALS
    void NotifyCheckNetConn(int result);
    void NotifyCheckNetConnSetting(int result);
    void NotifyCheckNetDHCP(int result);
    void NotifyCheckNetDNS(int result);
    void NotifyCheckNetDevice(int result);
    void NotifyCheckNetHost(int result);
};

#endif
