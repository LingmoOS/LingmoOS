// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMON_ADAPTOR_H
#define DAEMON_ADAPTOR_H

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
 * Adaptor class for interface com.deepin.pc.manager.system.daemon
 */
class DaemonAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.pc.manager.system.daemon")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.pc.manager.system.daemon\">\n"
"    <method name=\"StartApp\"/>\n"
"    <method name=\"ExitApp\"/>\n"
"    <method name=\"SetSysSettingValue\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"key\"/>\n"
"      <arg direction=\"in\" type=\"v\" name=\"value\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"succeed\"/>\n"
"    </method>\n"
"    <method name=\"GetSysSettingValue\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"key\"/>\n"
"      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
"    </method>\n"
"    <signal name=\"SysSettingValueChanged\">\n"
"      <arg type=\"s\" name=\"key\"/>\n"
"      <arg type=\"v\" name=\"value\"/>\n"
"    </signal>\n"
"    <method name=\"getProExePath\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"sId\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"GetDiskDevicePathList\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"pathList\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"CleanSelectFile\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"pathList\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"CleanJournal\"/>\n"
"    <method name=\"QueryPackageName\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"sBinaryFile\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"sPkgName\"/>\n"
"    </method>\n"
"    <method name=\"RefreshPackageTable\"/>\n"
"    <signal name=\"NotifySqlInsertFinish\"/>\n"
"    <method name=\"InsertUninstalledAppRecord\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"sAppID\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"sAppName\"/>\n"
"    </method>\n"
"    <method name=\"DeleteUninstalledAppRecord\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"sAppID\"/>\n"
"    </method>\n"
"    <method name=\"GetUnInstalledApps\">\n"
"      <arg direction=\"out\" type=\"av\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    DaemonAdaptor(QObject *parent);
    virtual ~DaemonAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void CleanJournal();
    void CleanSelectFile(const QStringList &pathList);
    void DeleteUninstalledAppRecord(const QString &sAppID);
    void ExitApp();
    QStringList GetDiskDevicePathList();
    QDBusVariant GetSysSettingValue(const QString &key);
    QVariantList GetUnInstalledApps();
    void InsertUninstalledAppRecord(const QString &sAppID, const QString &sAppName);
    QString QueryPackageName(const QString &sBinaryFile);
    void RefreshPackageTable();
    bool SetSysSettingValue(const QString &key, const QDBusVariant &value);
    void StartApp();
    QString getProExePath(const QString &sId);
Q_SIGNALS: // SIGNALS
    void NotifySqlInsertFinish();
    void SysSettingValueChanged(const QString &key, const QDBusVariant &value);
};

#endif
