// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMON_ADAPTOR_H
#define DAEMON_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "window/modules/common/defenderprocinfolist.h"
#include "../../deepin-pc-manager/src/window/modules/securitytools/defsecuritytoolinfo.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.pc.manager.session.daemon
 */
class DaemonAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.pc.manager.session.daemon")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.pc.manager.session.daemon\">\n"
"    <method name=\"StartApp\"/>\n"
"    <method name=\"ExitApp\"/>\n"
"    <method name=\"preInitialize\"/>\n"
"    <method name=\"startLauncherManage\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"getAppsInfoEnable\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"getAppsInfoDisable\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"isAutostart\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"sPath\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"exeAutostart\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"nStatus\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"sPath\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"value\"/>\n"
"    </method>\n"
"    <signal name=\"AccessRefreshData\">\n"
"      <arg type=\"b\" name=\"bAdd\"/>\n"
"      <arg type=\"s\" name=\"sID\"/>\n"
"    </signal>\n"
"    <method name=\"GetTrashInfoList\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetHistoryInfoList\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetCacheInfoList\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetLogInfoList\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetAppTrashInfoList\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QString\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetBrowserCookiesInfoList\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"cleanerPath\"/>\n"
"      <annotation value=\"QString\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"RequestStartTrashScan\"/>\n"
"    <method name=\"DeleteSpecifiedFiles\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"path\"/>\n"
"    </method>\n"
"    <signal name=\"TrashScanFinished\">\n"
"      <arg type=\"d\" name=\"sum\"/>\n"
"    </signal>\n"
"    <method name=\"RequestCleanSelectTrash\"/>\n"
"    <signal name=\"CleanSelectTrashFinished\"/>\n"
"    <method name=\"GetSystemArchitecture\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"arch\"/>\n"
"    </method>\n"
"    <method name=\"ModulesRequestAuthorityActived\"/>\n"
"    <method name=\"RequestAuthWithID\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"moduleName\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"operationID\"/>\n"
"    </method>\n"
"    <signal name=\"NotifyAuthStarted\"/>\n"
"    <signal name=\"NotifyAuthFinished\"/>\n"
"    <signal name=\"NotifyAuthResult\">\n"
"      <arg type=\"s\" name=\"moduleName\"/>\n"
"      <arg type=\"i\" name=\"operationID\"/>\n"
"      <arg type=\"b\" name=\"isSuccess\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    DaemonAdaptor(QObject *parent);
    virtual ~DaemonAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void DeleteSpecifiedFiles(const QStringList &path);
    void ExitApp();
    QString GetAppTrashInfoList();
    QString GetBrowserCookiesInfoList();
    QStringList GetCacheInfoList();
    QStringList GetHistoryInfoList();
    QStringList GetLogInfoList();
    QString GetSystemArchitecture();
    QStringList GetTrashInfoList();
    void ModulesRequestAuthorityActived();
    void RequestAuthWithID(const QString &moduleName, int operationID);
    void RequestCleanSelectTrash();
    void RequestStartTrashScan();
    void StartApp();
    bool exeAutostart(int nStatus, const QString &sPath);
    QString getAppsInfoDisable();
    QString getAppsInfoEnable();
    bool isAutostart(const QString &sPath);
    void preInitialize();
    bool startLauncherManage();
Q_SIGNALS: // SIGNALS
    void AccessRefreshData(bool bAdd, const QString &sID);
    void CleanSelectTrashFinished();
    void NotifyAuthFinished();
    void NotifyAuthResult(const QString &moduleName, int operationID, bool isSuccess);
    void NotifyAuthStarted();
    void TrashScanFinished(double sum);
};

#endif
