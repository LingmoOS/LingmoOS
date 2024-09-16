// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSDRAW_ADAPTOR_H
#define DBUSDRAW_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
//#include "schedulestructs.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * @bref: dbusdraw_adaptor 提供给外部程序调用的方法
*/
class dbusdraw_adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Draw")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.Draw\">\n"

                "    <method name=\"openFiles\">\n"
                "      <arg direction=\"in\" type=\"none\" name=\"openFiles\"/>\n"
                "    </method>\n"

                "    <method name=\"openImages\">\n"
                "      <arg direction=\"in\" type=\"none\" name=\"openImages\"/>\n"
                "    </method>\n"

                "    <method name=\"openFile\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"openFile\"/>\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"

                "  </interface>\n")
public:
    explicit dbusdraw_adaptor(QObject *parent);
    virtual ~dbusdraw_adaptor();

public Q_SLOTS: // METHODS
    bool openFiles(QList<QVariant> filePaths);
//    void openImages(QList<QVariant> images);
    bool openFile(QString filePath);

Q_SIGNALS: // SIGNALS
};

#endif // DBUSDRAW_ADAPTOR_H
