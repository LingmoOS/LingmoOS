// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DBlockDeviceMonitor>
#include "dblockdevicemonitor_p.h"
#include "dbus/udisks2_dbus_common.h"
#include "objectmanager_interface.h"

DMOUNT_USE_NAMESPACE

static constexpr char kIfaceDrive[] { "org.freedesktop.UDisks2.Drive" };
static constexpr char kIfaceBlock[] { "org.freedesktop.UDisks2.Block" };
static constexpr char kIfaceFileSystem[] { "org.freedesktop.UDisks2.Filesystem" };
static constexpr char kIfaceJob[] { "org.freedesktop.UDisks2.Job" };

DBlockDeviceMonitor::DBlockDeviceMonitor(QObject *parent)
    : QObject(parent), d_ptr(new DBlockDeviceMonitorPrivate(this))
{
}

DBlockDeviceMonitor::~DBlockDeviceMonitor()
{
}

void DBlockDeviceMonitor::setWatchChanges(bool watchChanges)
{
    Q_D(DBlockDeviceMonitor);
    if (d->isWatching == watchChanges)
        return;
    d->isWatching = watchChanges;

    OrgFreedesktopDBusObjectManagerInterface *objMng = UDisks2::objectManager();
    auto sysBus = QDBusConnection::systemBus();

    if (watchChanges) {
        connect(objMng, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                d, &DBlockDeviceMonitorPrivate::onInterfacesAdded);
        connect(objMng, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                d, &DBlockDeviceMonitorPrivate::onInterfacesRemoved);

        sysBus.connect(kUDisks2Service, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                       d, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage &)));
    } else {

        disconnect(objMng, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                   d, &DBlockDeviceMonitorPrivate::onInterfacesAdded);
        disconnect(objMng, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                   d, &DBlockDeviceMonitorPrivate::onInterfacesRemoved);

        sysBus.disconnect(kUDisks2Service, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                          d, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage &)));
    }
}

bool DBlockDeviceMonitor::watchChanges() const
{
    Q_D(const DBlockDeviceMonitor);
    return d->isWatching;
}

void DBlockDeviceMonitorPrivate::onInterfacesAdded(const QDBusObjectPath &objPath, const QMap<QString, QVariantMap> &ifaces)
{
    static const QString &kPrefixDrive = QStringLiteral("/org/freedesktop/UDisks2/drives/");
    static const QString &kPrefixDevice = QStringLiteral("/org/freedesktop/UDisks2/block_devices/");
    static const QString &kPrefixJob = QStringLiteral("/org/freedesktop/UDisks2/jobs/");

    Q_Q(DBlockDeviceMonitor);
    const QString &path = objPath.path();
    if (path.startsWith(kPrefixDrive)) {
        if (ifaces.contains(kIfaceDrive)) {
            // TODO(xust): should concern about udisks v2.1.7- (the missing DriveAdded signal) ?
            Q_EMIT q->driveAdded(path);
        }
    } else if (path.startsWith(kPrefixDevice)) {
        if (ifaces.contains(kIfaceBlock)) {
            // TODO(xust): should concern about udisks v2.1.7- (the missing DriveAdded signal) ?
            Q_EMIT q->deviceAdded(path);
        } else if (ifaces.contains(kIfaceFileSystem)) {
            Q_EMIT q->fileSystemAdded(path);
        }
    } else if (path.startsWith(kPrefixJob)) {
        if (ifaces.contains(kIfaceJob)) {
        }   // TODO(xust): job notify
    }
}

void DBlockDeviceMonitorPrivate::onInterfacesRemoved(const QDBusObjectPath &objPath, const QStringList &ifaces)
{
    const QString &path = objPath.path();

    Q_Q(DBlockDeviceMonitor);
    for (const QString &iface : qAsConst(ifaces)) {
        if (iface == kIfaceDrive)
            Q_EMIT q->driveRemoved(path);
        else if (iface == kIfaceFileSystem)
            Q_EMIT q->fileSystemRemoved(path);
        else if (iface == kIfaceBlock)
            Q_EMIT q->deviceRemoved(path);
    }
}

void DBlockDeviceMonitorPrivate::onPropertiesChanged(const QString &iface, const QVariantMap &changedProperties, const QDBusMessage &message)
{
    Q_Q(DBlockDeviceMonitor);

    const QString &objPath = message.path();

    if (iface == kIfaceFileSystem) {
        if (changedProperties.contains("MountPoints")) {
            const auto &mpts = qdbus_cast<QByteArrayList>(changedProperties.value("MountPoints"));
            if (mpts.isEmpty())
                Q_EMIT q->mountRemoved(objPath);
            else
                Q_EMIT q->mountAdded(objPath, mpts.first());
        }
    }

    if (iface == kIfaceBlock) {
        if (changedProperties.contains("HintIgnore")) {
            // TODO(xust): report propertychanged.
        }
    }
}
