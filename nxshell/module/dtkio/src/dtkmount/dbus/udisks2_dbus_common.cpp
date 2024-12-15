// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "udisks2_dbus_common.h"

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QXmlStreamReader>

#include "objectmanager_interface.h"
#include "udisks2_interface.h"

namespace UDisks2 {

DMOUNT_USE_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(OrgFreedesktopDBusObjectManagerInterface, omGlobal, (kUDisks2Service, "/org/freedesktop/UDisks2", QDBusConnection::systemBus()))
Q_GLOBAL_STATIC_WITH_ARGS(OrgFreedesktopUDisks2ManagerInterface, umGlobal, (kUDisks2Service, "/org/freedesktop/UDisks2/Manager", QDBusConnection::systemBus()))

bool interfaceExists(const QString &path, const QString &interface)
{
    QDBusInterface ud2(kUDisks2Service, path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus());
    QDBusReply<QString> reply = ud2.call("Introspect");
    QXmlStreamReader xmlParser(reply.value());

    while (!xmlParser.atEnd()) {
        xmlParser.readNext();

        if (xmlParser.tokenType() == QXmlStreamReader::StartElement
            && xmlParser.name().toString() == "interface") {
            const QString &name = xmlParser.attributes().value("name").toString();

            if (name == interface) {
                return true;
            }
        }
    }

    return false;
}

OrgFreedesktopDBusObjectManagerInterface *objectManager()
{
    if (!omGlobal.exists()) {
        qDBusRegisterMetaType<QMap<QString, QVariantMap>>();
        qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();
        qDBusRegisterMetaType<QByteArrayList>();
        qDBusRegisterMetaType<QPair<QString, QVariantMap>>();
        qDBusRegisterMetaType<QMap<QDBusObjectPath, QMap<QString, QVariantMap>>>();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // TODO
#else
    QMetaType::registerDebugStreamOperator<QList<QPair<QString, QVariantMap>>>();
#endif
    }

    return omGlobal;
}

QString version()
{
    return umGlobal->version();
}

QStringList supportedFilesystems()
{
    return umGlobal->supportedFilesystems();
}

}   // namespace UDisks2

QDBusArgument &operator<<(QDBusArgument &argument, const UDisks2::SmartAttribute &mystruct)
{
    argument.beginStructure();
    argument << mystruct.id
             << mystruct.name
             << mystruct.flags
             << mystruct.value
             << mystruct.worst
             << mystruct.threshold
             << mystruct.pretty
             << mystruct.prettyUnit
             << mystruct.expansion;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UDisks2::SmartAttribute &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.id
            >> mystruct.name
            >> mystruct.flags
            >> mystruct.value
            >> mystruct.worst
            >> mystruct.threshold
            >> mystruct.pretty
            >> mystruct.prettyUnit
            >> mystruct.expansion;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const UDisks2::ActiveDeviceInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.block
             << mystruct.slot
             << mystruct.state
             << mystruct.numReadErrors
             << mystruct.expansion;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UDisks2::ActiveDeviceInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.block
            >> mystruct.slot
            >> mystruct.state
            >> mystruct.numReadErrors
            >> mystruct.expansion;
    argument.endStructure();

    return argument;
}
