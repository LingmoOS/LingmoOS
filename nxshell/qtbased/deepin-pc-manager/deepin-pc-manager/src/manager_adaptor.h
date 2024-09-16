// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MANAGER_ADAPTOR_H
#define MANAGER_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.pc.manager
 */
class ManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.pc.manager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.pc.manager\">\n"
"    <method name=\"Show\"/>\n"
"    <method name=\"ShowModule\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"module\"/>\n"
"    </method>\n"
"    <method name=\"ShowPage\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"module\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"page\"/>\n"
"    </method>\n"
"    <method name=\"ExitApp\"/>\n"
"  </interface>\n"
        "")
public:
    ManagerAdaptor(QObject *parent);
    virtual ~ManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ExitApp();
    void Show();
    void ShowModule(const QString &module);
    void ShowPage(const QString &module, const QString &page);
Q_SIGNALS: // SIGNALS
};

#endif
