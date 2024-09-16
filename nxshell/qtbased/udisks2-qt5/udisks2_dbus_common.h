// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UDISK2_DBUS_COMMON_H
#define UDISK2_DBUS_COMMON_H

#include <QDBusObjectPath>
#include <QString>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
class QDBusArgument;
QT_END_NAMESPACE

class OrgFreedesktopDBusObjectManagerInterface;

#define UDISKS2_SERVICE "org.freedesktop.UDisks2"

namespace UDisks2 {
/// by: http://storaged.org/doc/udisks2-api/2.7.2/gdbus-org.freedesktop.UDisks2.Drive.Ata.html#gdbus-method-org-freedesktop-UDisks2-Drive-Ata.SmartGetAttributes
struct SmartAttribute
{
    uchar id; // Attribute Identifier
    QString name; // The identifier as a string.
    quint16 flags; // 16-bit attribute flags (bit 0 is prefail/oldage, bit 1 is online/offline).
    qint32 value; // The current value or -1 if unknown.
    qint32 worst; // The worst value of -1 if unknown.
    qint32 threshold; // The threshold or -1 if unknown.
    qint64 pretty; // An interpretation of the value - must be ignored if pretty_unit is 0.
    qint32 pretty_unit; // The unit of the pretty value - the following units are known: 0 (unknown), 1 (dimensionless), 2 (milliseconds), 3 (sectors), 4 (millikelvin).
    QVariantMap expansion; // Currently unused. Intended for future expansion.
};

/// by: http://storaged.org/doc/udisks2-api/2.7.2/gdbus-org.freedesktop.UDisks2.MDRaid.html#gdbus-property-org-freedesktop-UDisks2-MDRaid.ActiveDevices
struct ActiveDeviceInfo
{
    QDBusObjectPath block; // The object path for the underlying block device (guaranteed to implement the org.freedesktop.UDisks2.Block interface)
    qint32 slot; // -1 if the device is not currently part of the array (ie. spare or faulty), otherwise the slot number the device currently fills (between 0 and "NumDevices")
    QStringList state; // The state of the device - known elements include faulty, in_sync, write_mostly, blocked and spare
    quint64 num_read_errors; // An ongoing count of read errors that have been detected on this device but have not caused the device to be evicted from the array
    QVariantMap expansion; // Currently unused. Intended for future expansion.
};

/// by: http://storaged.org/doc/udisks2-api/2.7.2/udisks-std-options.html
// default options
// Many method calls take a parameter of type 'a{sv}' that is normally called options. The following table lists well-known options:
// "auth.no_user_interaction" 	bool 	// If set to TRUE, then no user interaction will happen when checking if the method call is authorized.

bool interfaceExists(const QString &path, const QString &interface);
OrgFreedesktopDBusObjectManagerInterface *objectManager();
QStringList supportedFilesystems();
QString version();
}

Q_DECLARE_METATYPE(UDisks2::SmartAttribute)

QDBusArgument &operator<<(QDBusArgument &argument, const UDisks2::SmartAttribute &mystruct);
const QDBusArgument &operator>>(const QDBusArgument &argument, UDisks2::SmartAttribute &mystruct);

Q_DECLARE_METATYPE(UDisks2::ActiveDeviceInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const UDisks2::ActiveDeviceInfo &mystruct);
const QDBusArgument &operator>>(const QDBusArgument &argument, UDisks2::ActiveDeviceInfo &mystruct);

#endif // UDISK2_DBUS_COMMON_H
