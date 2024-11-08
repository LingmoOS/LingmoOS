/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

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

#include "mainwindow.h"

class DbusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.usbcreator")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.lingmo.usbcreator\">\n"
"    <method name=\"showMainWindow\"/>\n"
"  </interface>\n"
        "")
public:
    DbusAdaptor(MainWindow *parent);
    virtual ~DbusAdaptor();

    inline MainWindow *parent() const
    { return static_cast<MainWindow *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void showMainWindow();
Q_SIGNALS: // SIGNALS
};

#endif
