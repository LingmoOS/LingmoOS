// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddiskmanager.h"
#include "udisks2_dbus_common.h"
#include "udisks2_interface.h"
#include "objectmanager_interface.h"
#include "dblockdevice.h"
#include "dblockpartition.h"
#include "ddiskdevice.h"
#include "dudisksjob.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QXmlStreamReader>
#include <QDBusMetaType>
#include <QScopedPointer>
#include <QDebug>

const QString ManagerPath = "/org/freedesktop/UDisks2/Manager";

static int udisks2VersionCompare(const QString &version)
{
    const QStringList &version_list = UDisks2::version().split(".");
    const QStringList &v_v_list = version.split(".");

    for (int i = 0; i < version_list.count(); ++i) {
        if (v_v_list.count() <= i)
            return -1;

        int number_v = version_list[i].toInt();
        int number_v_v = v_v_list[i].toInt();

        if (number_v == number_v_v)
            continue;

        return number_v_v > number_v ? 1 : -1;
    }

    return v_v_list.count() > version_list.count() ? 1 : 0;
}

// 2.1.7版本的UDisks2在U盘插入时没有drive device added的信号
// 当收到block device added信号后，通过diskDeviceAddSignalFlag
// 判断此块设备对应的磁盘设备信号是否已发送，未发送时补发信号
// 风险：如果 diskDeviceAddSignalFlag 的值删除的不及时
//      会导致设备再插入时不会再有信号发出
static bool fixUDisks2DiskAddSignal()
{
    static bool fix = udisks2VersionCompare("2.1.7.1") > 0;

    return fix;
}

class DDiskManagerPrivate
{
public:
    DDiskManagerPrivate(DDiskManager *qq);

    void updateBlockDeviceMountPointsMap();

    bool watchChanges = false;
    QMap<QString, QByteArrayList> blockDeviceMountPointsMap;
    QSet<QString> diskDeviceAddSignalFlag;

    DDiskManager *q_ptr;
};

DDiskManagerPrivate::DDiskManagerPrivate(DDiskManager *qq)
    : q_ptr(qq)
{

}

void DDiskManagerPrivate::updateBlockDeviceMountPointsMap()
{
    blockDeviceMountPointsMap.clear();

    auto om = UDisks2::objectManager();
    const QMap<QDBusObjectPath, QMap<QString, QVariantMap>> &objects = om->GetManagedObjects().value();
    auto begin = objects.constBegin();

    while (begin != objects.constEnd()) {
        const QString path = begin.key().path();
        const QMap<QString, QVariantMap> object = begin.value();

        ++begin;

        if (!path.startsWith(QStringLiteral("/org/freedesktop/UDisks2/block_devices/"))) {
            continue;
        }

        const QVariantMap &filesystem = object.value(QStringLiteral(UDISKS2_SERVICE ".Filesystem"));

        if (filesystem.isEmpty()) {
            continue;
        }

        blockDeviceMountPointsMap[path] = qdbus_cast<QByteArrayList>(filesystem.value("MountPoints"));
    }
}

void DDiskManager::onInterfacesAdded(const QDBusObjectPath &object_path, const QMap<QString, QVariantMap> &interfaces_and_properties)
{
    const QString &path = object_path.path();
    const QString &path_drive = QStringLiteral("/org/freedesktop/UDisks2/drives/");
    const QString &path_device = QStringLiteral("/org/freedesktop/UDisks2/block_devices/");
    const QString &path_job = QStringLiteral("/org/freedesktop/UDisks2/jobs/");

    Q_D(DDiskManager);

    if (path.startsWith(path_drive)) {
        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Drive"))) {
            if (fixUDisks2DiskAddSignal()) {
                if (!d->diskDeviceAddSignalFlag.contains(path)) {
                    d->diskDeviceAddSignalFlag.insert(path);
                    // 防止flag未清除导致再也收不到此设备的信号
                    QTimer::singleShot(1000, this, [d, path] {
                        d->diskDeviceAddSignalFlag.remove(path);
                    });

                    Q_EMIT diskDeviceAdded(path);
                }
            } else {
                Q_EMIT diskDeviceAdded(path);
            }
        }
    } else if (path.startsWith(path_device)) {
        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Block"))) {
            if (fixUDisks2DiskAddSignal()) {
                QScopedPointer<DBlockDevice> bd(createBlockDevice(path));
                const QString &drive = bd->drive();

                if (!d->diskDeviceAddSignalFlag.contains(drive)) {
                    d->diskDeviceAddSignalFlag.insert(drive);
                    // 防止flag未清除导致再也收不到此设备的信号
                    QTimer::singleShot(1000, this, [d, drive] {
                        d->diskDeviceAddSignalFlag.remove(drive);
                    });

                    Q_EMIT diskDeviceAdded(drive);
                }
            }

            Q_EMIT blockDeviceAdded(path);
        }

        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Filesystem"))) {
            Q_D(DDiskManager);

            d->blockDeviceMountPointsMap.remove(object_path.path());

            Q_EMIT fileSystemAdded(path);
        }
    } else if (path.startsWith(path_job)) {
        if (interfaces_and_properties.contains(QStringLiteral(UDISKS2_SERVICE ".Job"))) {
            Q_EMIT jobAdded(path);
        }
    }
}

void DDiskManager::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    const QString &path = object_path.path();

    Q_D(DDiskManager);

    for (const QString &i : interfaces) {
        if (i == QStringLiteral(UDISKS2_SERVICE ".Drive")) {
            d->diskDeviceAddSignalFlag.remove(path);

            Q_EMIT diskDeviceRemoved(path);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Filesystem")) {
            d->blockDeviceMountPointsMap.remove(object_path.path());

            Q_EMIT fileSystemRemoved(path);
        } else if (i == QStringLiteral(UDISKS2_SERVICE ".Block")) {
            Q_EMIT blockDeviceRemoved(path);
        }
    }
}

void DDiskManager::onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties, const QDBusMessage &message)
{
    Q_D(DDiskManager);

    const QString &path = message.path();

    if (changed_properties.contains("Optical")) {
        Q_EMIT opticalChanged(path);
    }

    if (interface != UDISKS2_SERVICE ".Filesystem") {
        return;
    }

    if (!changed_properties.contains("MountPoints")) {
        return;
    }

    const QByteArrayList old_mount_points = d->blockDeviceMountPointsMap.value(path);
    const QByteArrayList &new_mount_points = qdbus_cast<QByteArrayList>(changed_properties.value("MountPoints"));

    d->blockDeviceMountPointsMap[path] = new_mount_points;

    Q_EMIT mountPointsChanged(path, old_mount_points, new_mount_points);

    if (old_mount_points.isEmpty()) {
        if (!new_mount_points.isEmpty()) {
            Q_EMIT mountAdded(path, new_mount_points.first());
        }
    } else if (new_mount_points.isEmpty()) {
        Q_EMIT mountRemoved(path, old_mount_points.first());
    }
}

/*!
 * \class DDiskManager
 * \inmodule dde-file-manager-lib
 *
 * \brief DDiskManager provide severial ways to manage devices and partitions.
 *
 * \sa DBlockPartition, DBlockDevice, UDiskDeviceInfo
 */

DDiskManager::DDiskManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DDiskManagerPrivate(this))
{

}

DDiskManager::~DDiskManager()
{

}

static QStringList getDBusNodeNameList(const QString &service, const QString &path, const QDBusConnection &connection)
{
    QDBusInterface ud2(service, path, "org.freedesktop.DBus.Introspectable", connection);
    QDBusReply<QString> reply = ud2.call("Introspect");
    QXmlStreamReader xml_parser(reply.value());
    QStringList nodeList;

    while (!xml_parser.atEnd()) {
        xml_parser.readNext();

        if (xml_parser.tokenType() == QXmlStreamReader::StartElement
                && xml_parser.name().toString() == "node") {
            const QString &name = xml_parser.attributes().value("name").toString();

            if (!name.isEmpty()) {
                nodeList << path + "/" + name;
            }
        }
    }

    return nodeList;
}

QStringList DDiskManager::blockDevices() const
{
    return getDBusNodeNameList(UDISKS2_SERVICE, "/org/freedesktop/UDisks2/block_devices", QDBusConnection::systemBus());
}

QStringList DDiskManager::diskDevices() const
{
    return getDBusNodeNameList(UDISKS2_SERVICE, "/org/freedesktop/UDisks2/drives", QDBusConnection::systemBus());
}

QStringList DDiskManager::blockDevices(QVariantMap options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());

    auto reply = udisksmgr.GetBlockDevices(options);
    reply.waitForFinished();
    QList<QDBusObjectPath> resultList = reply.value();
    QStringList dbusPaths;
    for (const QDBusObjectPath &singleResult : resultList) {
        dbusPaths << singleResult.path();
    }
    return dbusPaths;
}

bool DDiskManager::watchChanges() const
{
    Q_D(const DDiskManager);

    return d->watchChanges;
}

QString DDiskManager::objectPrintable(const QObject *object)
{
    QString string;
    QDebug debug(&string);
    const QMetaObject *mo = object->metaObject();

    debug << object;

    int property_count = mo->propertyCount();
    int base_property_count = QObject::staticMetaObject.propertyCount();

    debug << "\n";

    for (int i = base_property_count; i < property_count; ++i) {
        const QMetaProperty &mp = mo->property(i);

        debug.nospace() << mp.name() << ": " << mp.read(object);
        debug << "\n";
    }

    return string;
}

DBlockDevice *DDiskManager::createBlockDevice(const QString &path, QObject *parent)
{
    return new DBlockDevice(path, parent);
}

DBlockDevice *DDiskManager::createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent) const
{
    for (const QString &block : blockDevices()) {
        DBlockDevice *device = new DBlockDevice(block, parent);

        if (device->device() == path) {
            return device;
        }

        device->deleteLater();
    }

    return nullptr;
}

DBlockPartition *DDiskManager::createBlockPartition(const QString &path, QObject *parent)
{
    return new DBlockPartition(path, parent);
}

DBlockPartition *DDiskManager::createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent) const
{
    for (const QString &block : blockDevices()) {
        DBlockPartition *device = new DBlockPartition(block, parent);

        if (device->mountPoints().contains(path)) {
            return device;
        }

        device->deleteLater();
    }

    return nullptr;
}

DBlockPartition *DDiskManager::createBlockPartition(const QStorageInfo &info, QObject *parent) const
{
    return createBlockPartitionByMountPoint(info.rootPath().toLocal8Bit() + '\0', parent);
}

DDiskDevice *DDiskManager::createDiskDevice(const QString &path, QObject *parent)
{
    return new DDiskDevice(path, parent);
}

DUDisksJob *DDiskManager::createJob(const QString &path, QObject *parent)
{
    return new DUDisksJob(path, parent);
}

QStringList DDiskManager::supportedFilesystems()
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    return udisksmgr.supportedFilesystems();
}

QStringList DDiskManager::supportedEncryptionTypes()
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    return udisksmgr.supportedEncryptionTypes();
}

QStringList DDiskManager::resolveDevice(QVariantMap devspec, QVariantMap options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    QStringList ret;
    auto devices = udisksmgr.ResolveDevice(devspec, options);
    devices.waitForFinished();
    if (!devices.isError()) {
        for (auto &d : devices.value()) {
            ret.push_back(d.path());
        }
    }
    return ret;
}

QStringList DDiskManager::resolveDeviceNode(QString devnode, QVariantMap options)
{
    return resolveDevice({{"path", QVariant(devnode)}}, options);
}

bool DDiskManager::canCheck(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanCheck(type);
    r.waitForFinished();
    if (r.isError()) {
        return false;
    }
    if (requiredUtil) {
        *requiredUtil = r.value().second;
    }
    return r.value().first;
}

bool DDiskManager::canFormat(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanFormat(type);
    r.waitForFinished();
    if (r.isError()) {
        return false;
    }
    if (requiredUtil) {
        *requiredUtil = r.value().second;
    }
    return r.value().first;
}

bool DDiskManager::canRepair(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanRepair(type);
    r.waitForFinished();
    if (r.isError()) {
        return false;
    }
    if (requiredUtil) {
        *requiredUtil = r.value().second;
    }
    return r.value().first;
}

bool DDiskManager::canResize(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanRepair(type);
    r.waitForFinished();
    if (r.isError()) {
        return false;
    }
    if (requiredUtil) {
        *requiredUtil = r.value().second;
    }
    return r.value().first;
}

QString DDiskManager::loopSetup(int fd, QVariantMap options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(UDISKS2_SERVICE, ManagerPath, QDBusConnection::systemBus());
    QDBusUnixFileDescriptor dbusfd;
    dbusfd.setFileDescriptor(fd);
    auto r = udisksmgr.LoopSetup(dbusfd, options);
    r.waitForFinished();
    return r.value().path();
}

QDBusError DDiskManager::lastError()
{
    return QDBusConnection::systemBus().lastError();
}

void DDiskManager::setWatchChanges(bool watchChanges)
{
    Q_D(DDiskManager);

    if (d->watchChanges == watchChanges)
        return;

    OrgFreedesktopDBusObjectManagerInterface *object_manager = UDisks2::objectManager();
    auto sc = QDBusConnection::systemBus();

    if (watchChanges) {
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                this, &DDiskManager::onInterfacesAdded);
        connect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                this, &DDiskManager::onInterfacesRemoved);

        d->updateBlockDeviceMountPointsMap();

        sc.connect(UDISKS2_SERVICE, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage&)));
    } else {
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                   this, &DDiskManager::onInterfacesAdded);
        disconnect(object_manager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                   this, &DDiskManager::onInterfacesRemoved);

        d->blockDeviceMountPointsMap.clear();

        sc.disconnect(UDISKS2_SERVICE, QString(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                      this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &, const QDBusMessage&)));
    }
}
