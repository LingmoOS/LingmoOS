// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSUIOPENSCHEDULE_H
#define DBUSUIOPENSCHEDULE_H
#include <QtDBus/QtDBus>

class DbusUIOpenSchedule : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.Calendar";
    }
    explicit DbusUIOpenSchedule(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);
    inline void OpenSchedule(const QString strjson)
    {
        QList<QVariant> argumentList;
        argumentList << strjson;
        //不需要返回结果，发送完直接结束
        callWithArgumentList(QDBus::NoBlock, QStringLiteral("OpenSchedule"), argumentList);
    }
};

#endif // DBUSUIOPENSCHEDULE_H
