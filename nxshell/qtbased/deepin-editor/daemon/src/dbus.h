// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_H
#define DBUS_H

#include <QtCore/QObject>

class DBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.editor.daemon")

public:
    DBus(QObject* parent = nullptr);

public Q_SLOTS:
    bool saveFile(const QByteArray &path, const QByteArray &text, const QByteArray &encoding);
};


#endif //DBUS_H
