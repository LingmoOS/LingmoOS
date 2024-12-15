// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINKINFOLIST_H
#define LINKINFOLIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QDBusArgument>
#include <QDBusMetaType>

class LinkInfo {

public:
    friend QDebug operator<<(QDebug arg, const LinkInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const LinkInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, LinkInfo &info);

    static LinkInfo fromJson(const QJsonObject &infoObject);

    bool operator==(const LinkInfo &what);

public:
    QString m_name;
    QString m_friendlyName;
    QString m_macAddr;
    bool m_managed;
    bool m_p2pScanning;
    QDBusObjectPath m_dbusPath;
};

typedef QList<LinkInfo> LinkInfoList;

Q_DECLARE_METATYPE(LinkInfo)
Q_DECLARE_METATYPE(LinkInfoList)

void registerLinkInfoMetaType();
void registerLinkInfoListMetaType();

#endif // LINKINFOLIST_H
