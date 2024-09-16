// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSCALENDAR_ADAPTOR_H
#define DBUSCALENDAR_ADAPTOR_H

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
 * Adaptor class for interface com.deepin.Calendar
 */
class CalendarAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Calendar")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.Calendar\">\n"
                "    <method name=\"ActiveWindow\">\n"
                "    </method>\n"
                "    <method name=\"RaiseWindow\">\n"
                "    </method>\n"
                "    <method name=\"OpenSchedule\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"job\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")
public:
    explicit CalendarAdaptor(QObject *parent);
    virtual ~CalendarAdaptor();
public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ActiveWindow();
    void RaiseWindow();
    void OpenSchedule(QString job);
Q_SIGNALS: // SIGNALS
};

#endif
