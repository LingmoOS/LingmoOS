// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QMap>
#include <QDBusConnection>
#include <QDBusError>
#include <QMetaType>
#include <QMetaClassInfo>
#include <QLocale>
#include <QDir>
#include <QRegularExpression>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QDBusArgument>
#include <QDBusMessage>
#include <qdbusmetatype.h>
#include <unistd.h>
#include <QUuid>
#include <QLoggingCategory>
#include <sys/stat.h>

Q_DECLARE_LOGGING_CATEGORY(DDEAMProf)

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using QStringMap = QMap<QString, QString>;
using PropMap = QMap<QString, QStringMap>;

Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(QStringMap)
Q_DECLARE_METATYPE(PropMap)

struct SystemdUnitDBusMessage
{
    QString name;
    QString subState;
    QDBusObjectPath objectPath;
};

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QStringMap &map)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }
    argument.endMap();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QList<SystemdUnitDBusMessage> &units)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        argument.beginStructure();
        QString _str;
        uint32_t _uint;
        QDBusObjectPath _path;
        SystemdUnitDBusMessage unit;
        argument >> unit.name >> _str >> _str >> _str >> unit.subState >> _str >> unit.objectPath >> _uint >> _str >> _path;
        units.push_back(unit);
        argument.endStructure();
    }
    argument.endArray();

    return argument;
}

inline void registerAmMetaType()
{
    qRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<QDBusObjectPath>();
}
