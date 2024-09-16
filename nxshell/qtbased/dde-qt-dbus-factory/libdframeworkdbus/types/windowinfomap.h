// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWINFOLIST_H
#define WINDOWINFOLIST_H

#include <QDebug>
#include <QList>
#include <QDBusArgument>

class WindowInfo
{
public:
    friend QDebug operator<<(QDebug argument, const WindowInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const WindowInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, WindowInfo &info);

    bool operator==(const WindowInfo &rhs) const;

public:
    bool attention;
    QString title;
};
Q_DECLARE_METATYPE(WindowInfo)

typedef QMap<quint32, WindowInfo> WindowInfoMap;
Q_DECLARE_METATYPE(WindowInfoMap)

void registerWindowInfoMetaType();
void registerWindowInfoMapMetaType();

#endif // WINDOWINFOLIST_H
