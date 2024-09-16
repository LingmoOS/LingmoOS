// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYLIST_H
#define PROPERTYLIST_H

#include <QObject>
#include <QDBusArgument>

class Property
{
public:
    explicit Property();

    QString key;
    QString value;

    friend QDBusArgument &operator<<(QDBusArgument &arg, const Property &value);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, Property &value);
};

typedef QList<Property> PropertyList;

Q_DECLARE_METATYPE(Property)
Q_DECLARE_METATYPE(PropertyList)

void registerPropertyListMetaType();

#endif // PROPERTYLIST_H
