/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef APPINFODBUSARGUMENT_H
#define APPINFODBUSARGUMENT_H

#include <QDBusArgument>
#include "application-property.h"

using namespace LingmoUISearch;
QDBusArgument &operator << (QDBusArgument &argument, const LingmoUISearch::ApplicationProperty::Property &property) {
    argument.beginStructure();
    argument << static_cast<int>(property);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator >> (const QDBusArgument &argument, LingmoUISearch::ApplicationProperty::Property &property) {
    int value;
    argument.beginStructure();
    argument >> value;
    argument.endStructure();
    property = static_cast<ApplicationProperty::Property>(value);
    return argument;
}

QDBusArgument &operator << (QDBusArgument &argument, const ApplicationPropertyMap &appPropertyInfo)
{
    argument.beginMap(/*qMetaTypeId<ApplicationProperty::Property>()*/QVariant::Int, qMetaTypeId<QDBusVariant>());
    for (auto i = appPropertyInfo.constBegin(); i != appPropertyInfo.constEnd(); ++i) {
        QDBusVariant dbusVariant(i.value());
        argument.beginMapEntry();
        argument << static_cast<int>(i.key()) << dbusVariant;
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator >> (const QDBusArgument &argument, ApplicationPropertyMap &appPropertyInfo)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        int key;
        QVariant value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        appPropertyInfo.insert(static_cast<ApplicationProperty::Property>(key), value);
    }
    argument.endMap();
    return argument;
}

QDBusArgument &operator << (QDBusArgument &argument, const ApplicationInfoMap &appInfo)
{
    argument.beginMap(QVariant::String, qMetaTypeId<ApplicationPropertyMap>());
    for (auto i = appInfo.constBegin(); i != appInfo.constEnd(); ++i) {
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator >> (const QDBusArgument &argument, ApplicationInfoMap &appInfo)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        ApplicationPropertyMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        appInfo.insert(key, value);
    }
    argument.endMap();
    return argument;
}
#endif // APPINFODBUSARGUMENT_H
