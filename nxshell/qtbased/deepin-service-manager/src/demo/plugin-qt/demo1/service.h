// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>

class Service : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.service.demo1")
    // 1, Qtdbus会自动导出改类的dbus接口
    // 2，添加了"D-Bus Introspection"后，可以指定需要导出到dbus的接口
    // 3，"D-Bus Introspection"可不添加，默认导出所有
    //    Q_CLASSINFO("D-Bus Introspection", ""
    //"  <interface name=\"com.deepin.dbusdemo\">\n"
    //"    <method name=\"Hello\">\n"
    //"      <arg direction=\"out\" type=\"s\"/>\n"
    //"    </method>\n"
    //"  </interface>\n"
    //        "")
public:
    explicit Service(QObject *parent = 0);

signals:

public slots:
    QString Hello();
};

#endif // SERVICE_H
