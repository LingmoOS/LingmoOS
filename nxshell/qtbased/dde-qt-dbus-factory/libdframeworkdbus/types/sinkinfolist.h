// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PEERINFOLIST_H
#define PEERINFOLIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QDBusArgument>
#include <QDBusMetaType>

class SinkInfo {

public:
    friend QDebug operator<<(QDebug arg, const SinkInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const SinkInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, SinkInfo &info);

    static SinkInfo fromJson(const QJsonObject &infoObject);

    bool operator==(const SinkInfo &what);

public:
    QString m_name;
    QString m_p2pMac;
    QString m_interface;
    bool m_connected;
    QDBusObjectPath m_sinkPath;
    QDBusObjectPath m_linkPath;
};

typedef QList<SinkInfo> SinkInfoList;

Q_DECLARE_METATYPE(SinkInfo)
Q_DECLARE_METATYPE(SinkInfoList)

void registerSinkInfoMetaType();
void registerSinkInfoListMetaType();

#endif // PEERINFOLIST_H
