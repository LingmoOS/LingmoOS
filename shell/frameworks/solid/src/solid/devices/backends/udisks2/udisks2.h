/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UDISKS2_H
#define SOLID_BACKENDS_UDISKS2_H

#include <QCoreApplication>
#include <QDBusAbstractInterface>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QPointer>
#include <QTimer>
#include <QVariant>

typedef QMap<QString, QVariantMap> VariantMapMap;
Q_DECLARE_METATYPE(VariantMapMap)

typedef QMap<QDBusObjectPath, VariantMapMap> DBUSManagerStruct;
Q_DECLARE_METATYPE(DBUSManagerStruct)

// clang-format off

/* UDisks2 */
#define UD2_DBUS_SERVICE                 "org.freedesktop.UDisks2"
#define UD2_DBUS_PATH                    "/org/freedesktop/UDisks2"
#define UD2_UDI_DISKS_PREFIX             "/org/freedesktop/UDisks2"
#define UD2_DBUS_PATH_MANAGER            "/org/freedesktop/UDisks2/Manager"
#define UD2_DBUS_PATH_DRIVES             "/org/freedesktop/UDisks2/drives"
#define UD2_DBUS_PATH_JOBS               "/org/freedesktop/UDisks2/jobs/"
#define UD2_DBUS_PATH_BLOCKDEVICES       "/org/freedesktop/UDisks2/block_devices"
#define DBUS_INTERFACE_PROPS             "org.freedesktop.DBus.Properties"
#define DBUS_INTERFACE_INTROSPECT        "org.freedesktop.DBus.Introspectable"
#define DBUS_INTERFACE_MANAGER           "org.freedesktop.DBus.ObjectManager"
#define UD2_DBUS_INTERFACE_BLOCK         "org.freedesktop.UDisks2.Block"
#define UD2_DBUS_INTERFACE_DRIVE         "org.freedesktop.UDisks2.Drive"
#define UD2_DBUS_INTERFACE_PARTITION     "org.freedesktop.UDisks2.Partition"
#define UD2_DBUS_INTERFACE_PARTITIONTABLE   "org.freedesktop.UDisks2.PartitionTable"
#define UD2_DBUS_INTERFACE_FILESYSTEM    "org.freedesktop.UDisks2.Filesystem"
#define UD2_DBUS_INTERFACE_ENCRYPTED     "org.freedesktop.UDisks2.Encrypted"
#define UD2_DBUS_INTERFACE_SWAP          "org.freedesktop.UDisks2.Swapspace"
#define UD2_DBUS_INTERFACE_LOOP          "org.freedesktop.UDisks2.Loop"

/* errors */
#define UD2_ERROR_UNAUTHORIZED            "org.freedesktop.PolicyKit.Error.NotAuthorized"
#define UD2_ERROR_BUSY                    "org.freedesktop.UDisks2.Error.DeviceBusy"
#define UD2_ERROR_FAILED                  "org.freedesktop.UDisks2.Error.Failed"
#define UD2_ERROR_CANCELED                "org.freedesktop.UDisks2.Error.Cancelled"
#define UD2_ERROR_INVALID_OPTION          "org.freedesktop.UDisks2.Error.OptionNotPermitted"
#define UD2_ERROR_MISSING_DRIVER          "org.freedesktop.UDisks2.Error.NotSupported"

#define UD2_ERROR_ALREADY_MOUNTED         "org.freedesktop.UDisks2.Error.AlreadyMounted"
#define UD2_ERROR_NOT_MOUNTED             "org.freedesktop.UDisks2.Error.NotMounted"
#define UD2_ERROR_MOUNTED_BY_OTHER_USER   "org.freedesktop.UDisks2.Error.MountedByOtherUser"
#define UD2_ERROR_ALREADY_UNMOUNTING      "org.freedesktop.UDisks2.Error.AlreadyUnmounting"
#define UD2_ERROR_TIMED_OUT               "org.freedesktop.UDisks2.Error.Timedout"
#define UD2_ERROR_WOULD_WAKEUP            "org.freedesktop.UDisks2.Error.WouldWakeup"
#define UD2_ERROR_ALREADY_CANCELLED       "org.freedesktop.UDisks2.Error.AlreadyCancelled"

#define UD2_ERROR_NOT_AUTHORIZED          "org.freedesktop.UDisks2.Error.NotAuthorized"
#define UD2_ERROR_NOT_AUTHORIZED_CAN_OBTAIN  "org.freedesktop.UDisks2.Error.NotAuthorizedCanObtain"
#define UD2_ERROR_NOT_AUTHORIZED_DISMISSED   "org.freedesktop.UDisks2.Error.NotAuthorizedDismissed"

#endif // SOLID_BACKENDS_UDISKS2_H
