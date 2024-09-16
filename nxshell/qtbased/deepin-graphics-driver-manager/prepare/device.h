// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICE_H
#define DEVICE_H

#include <QJsonObject>
#include <QList>

class Device
{
public:
    explicit Device(const QJsonObject &info)
    {
        m_name = info["name"].toString();
        m_info = info["info"].toString();
    }

    inline const QString name() const { return m_name; }
    inline const QString info() const { return m_info; }
    inline bool operator==(const Device &rhs) const
    {
        return m_name == rhs.name() && m_info == rhs.name();
    }
private:
    QString m_name;
    QString m_info;
};

typedef QList<Device> DeviceList;

#endif
