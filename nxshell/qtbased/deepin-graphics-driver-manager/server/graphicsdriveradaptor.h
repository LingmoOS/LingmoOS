// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRAPHICSDRIVERADAPTOR_H
#define GRAPHICSDRIVERADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "graphicsdriverinterface.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.daemon.GraphicsDriver
 */
class GraphicsDriverAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.GraphicsDriver")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.daemon.GraphicsDriver\">\n"
"    <signal name=\"ReportProgress\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"ratio\"/>\n"
"    </signal>\n"
"    <signal name=\"Cancel\"/>\n"
"    <method name=\"GetDevice\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetResolutionTitle\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"PrepareInstall\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"language\"/>\n"
"    </method>\n"
"    <method name=\"CancelInstall\"/>\n"
"    <method name=\"TestSuccess\"/>\n"
"    <method name=\"IsTestSuccess\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"RealInstall\"/>\n"
"    <method name=\"TestInstall\"/>\n"
"    <method name=\"GetOldDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetNewDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetCurrDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    GraphicsDriverAdaptor(QObject *parent);
    virtual ~GraphicsDriverAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void CancelInstall();
    QString GetCurrDriverName();
    QString GetDevice();
    QString GetNewDriverName();
    QString GetOldDriverName();
    QString GetResolutionTitle();
    bool IsTestSuccess();
    void PrepareInstall(const QString &name, const QString &language);
    void RealInstall();
    void TestInstall();
    void TestSuccess();
Q_SIGNALS: // SIGNALS
    void Cancel();
    void ReportProgress(const QString &ratio);
};

#endif
