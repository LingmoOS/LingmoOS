// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYTOOLDIALOG_ADAPTOR_H
#define SECURITYTOOLDIALOG_ADAPTOR_H

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
 * Adaptor class for interface com.deepin.pc.manager.securitytooldialog
 */
class SecuritytooldialogAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.pc.manager.securitytooldialog")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.pc.manager.securitytooldialog\">\n"
"    <method name=\"SetDefenderPaletteType\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
"    </method>\n"
"    <method name=\"ShowStartup\"/>\n"
"    <method name=\"ShowNetCheck\"/>\n"
"    <method name=\"OnRequestUpdateToolsInfo\"/>\n"
"    <method name=\"OnInstallPackage\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"strPackageKey\"/>\n"
"    </method>\n"
"    <method name=\"OnUnInstallPackage\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"strPackageKey\"/>\n"
"    </method>\n"
"    <method name=\"OnUpdatePackage\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"strPackageKey\"/>\n"
"    </method>\n"
"    <method name=\"OnShowTool\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"strPackageKey\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"strParams\"/>\n"
"    </method>\n"
"    <method name=\"GetInstallingApps\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"installingapps\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetUpdatingApps\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"updatingapps\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"GetUninstallingApps\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"uninstallingapps\"/>\n"
"      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <signal name=\"notifyToolsInfoUpdate\">\n"
"      <arg direction=\"out\" type=\"a(sssssssbbisss)\" name=\"infos\"/>\n"
"      <annotation value=\"DEFSECURITYTOOLINFOLIST\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"notifyAppStatusChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"strPackageKey\"/>\n"
"      <arg direction=\"out\" type=\"i\" name=\"status\"/>\n"
"    </signal>\n"
"    <signal name=\"notifyStatusCheckFinished\"/>\n"
"  </interface>\n"
        "")
public:
    SecuritytooldialogAdaptor(QObject *parent);
    virtual ~SecuritytooldialogAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QStringList GetInstallingApps();
    QStringList GetUninstallingApps();
    QStringList GetUpdatingApps();
    void OnInstallPackage(const QString &strPackageKey);
    void OnRequestUpdateToolsInfo();
    void OnShowTool(const QString &strPackageKey, const QStringList &strParams);
    void OnUnInstallPackage(const QString &strPackageKey);
    void OnUpdatePackage(const QString &strPackageKey);
    void SetDefenderPaletteType(int type);
    void ShowNetCheck();
    void ShowStartup();
Q_SIGNALS: // SIGNALS
    void notifyAppStatusChanged(const QString &strPackageKey, int status);
    void notifyStatusCheckFinished();
    void notifyToolsInfoUpdate(DEFSECURITYTOOLINFOLIST infos);
};

#endif
