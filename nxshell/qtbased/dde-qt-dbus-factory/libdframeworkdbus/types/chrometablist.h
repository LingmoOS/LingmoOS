// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHROMETABS_H
#define CHROMETABS_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QDBusArgument>
#include <QDBusMetaType>

struct ChromeTabInfo {
    qint32 id;
    quint64 memory;
    QString title;

    bool operator ==(const ChromeTabInfo& info);
};

typedef QList<ChromeTabInfo> ChromeTabList;

Q_DECLARE_METATYPE(ChromeTabInfo)
Q_DECLARE_METATYPE(ChromeTabList)

QDBusArgument &operator<<(QDBusArgument &arg, const ChromeTabInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &arg, ChromeTabInfo &info);

void registerChromeTabListMetaType();

#endif // CHROMETABS_H
