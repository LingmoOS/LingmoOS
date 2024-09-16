// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONPATH_H
#define SESSIONPATH_H

#include <QDBusMetaType>
#include <QDBusObjectPath>

struct SessionPath {
    QString out0;
    QDBusObjectPath path;

    bool operator ==(const SessionPath &other);
    bool operator !=(const SessionPath &other);
};

Q_DECLARE_METATYPE(SessionPath)

QDBusArgument &operator<<(QDBusArgument &arg, const SessionPath &rect);
const QDBusArgument &operator>>(const QDBusArgument &arg, SessionPath &rect);

void registerSessionPathMetaType();

#endif // SESSIONPATH_H
