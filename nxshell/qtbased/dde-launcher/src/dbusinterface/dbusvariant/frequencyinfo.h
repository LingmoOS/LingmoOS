// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FREQUENCYINFO_H
#define FREQUENCYINFO_H

#include <QtDBus>
#include <QDebug>

class FrequencyInfo
{
public:
    FrequencyInfo();
    ~FrequencyInfo();

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const FrequencyInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const FrequencyInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const FrequencyInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, FrequencyInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, FrequencyInfo &info);

public:
    QString m_key;
    qulonglong m_count;
};

typedef QList<FrequencyInfo> FrequencyInfoList;

Q_DECLARE_METATYPE(FrequencyInfo)
Q_DECLARE_METATYPE(FrequencyInfoList)

#endif // FREQUENCYINFO_H
