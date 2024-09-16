// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertylist.h"

#include <QtDBus>

Property::Property()
{

}

QDBusArgument &operator<<(QDBusArgument &arg, const Property &value)
{
    arg.beginStructure();
    arg << value.key << value.value;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Property &value)
{
    arg.beginStructure();
    arg >> value.key >> value.value;
    arg.endStructure();

    return arg;
}

void registerPropertyListMetaType()
{
    qRegisterMetaType<Property>();
    qDBusRegisterMetaType<Property>();

    qRegisterMetaType<PropertyList>();
    qDBusRegisterMetaType<PropertyList>();
}
