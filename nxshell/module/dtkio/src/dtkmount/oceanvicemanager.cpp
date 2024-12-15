// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DDeviceManager>
#include <DBlockDevice>
#include <DDiskDrive>
#include <DDiskJob>
#include <DBlockPartition>
#include <DBlockDeviceMonitor>
#include <DProtocolDeviceMonitor>
#include <DProtocolDevice>
#include "udisks2_interface.h"
#include "dbus/udisks2_dbus_common.h"

DMOUNT_BEGIN_NAMESPACE

static QStringList getDBusNodeNameList(const QString &service, const QString &path, const QDBusConnection &connection)
{
    QDBusInterface ud2(service, path, "org.freedesktop.DBus.Introspectable", connection);
    const QDBusReply<QString> &reply = ud2.call("Introspect");
    QXmlStreamReader xmlParser(reply.value());
    QStringList nodeList;

    while (!xmlParser.atEnd()) {
        xmlParser.readNext();

        if (xmlParser.tokenType() == QXmlStreamReader::StartElement
            && xmlParser.name().toString() == "node") {
            const QString &name = xmlParser.attributes().value("name").toString();

            if (!name.isEmpty())
                nodeList << path + "/" + name;
        }
    }

    return nodeList;
}

DBlockDeviceMonitor *DDeviceManager::globalBlockDeviceMonitor()
{
    static DBlockDeviceMonitor monitor;
    return &monitor;
}

DProtocolDeviceMonitor *DDeviceManager::globalProtocolDeviceMonitor()
{
    static DProtocolDeviceMonitor monitor;
    return &monitor;
}

DExpected<QStringList> DDeviceManager::blockDevices(const QVariantMap &options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());

    auto reply = udisksmgr.GetBlockDevices(options);
    reply.waitForFinished();
    if (reply.isError())
        return DUnexpected<> { DError { reply.error().type(), reply.error().message() } };

    const QList<QDBusObjectPath> &resultList = reply.value();
    QStringList dbusPaths;
    for (const QDBusObjectPath &singleResult : resultList)
        dbusPaths << singleResult.path();
    return std::move(dbusPaths);
}

QStringList DDeviceManager::protocolDevices()
{
    return globalProtocolDeviceMonitor()->devices();
}

QStringList DDeviceManager::diskDrives()
{
    return getDBusNodeNameList(kUDisks2Service, kUDisks2DrivePath, QDBusConnection::systemBus());
}

DExpected<DBlockDevice *> DDeviceManager::createBlockDevice(const QString &path, QObject *parent)
{
    const auto &devs = blockDevices();
    if (!devs.hasValue())
        return DUnexpected<> { devs.error() };

    if (blockDevices().value().contains(path))
        return new DBlockDevice(path, parent);
    return DUnexpected<> { DError { -1, "No such object path: " + path } };
}

DExpected<DBlockDevice *> DDeviceManager::createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent)
{
    const auto &devs = blockDevices();
    if (!devs.hasValue())
        return DUnexpected<> { devs.error() };

    for (const QString &block : devs.value()) {
        DBlockDevice *device = new DBlockDevice(block, parent);

        if (device->device() == path)
            return device;

        device->deleteLater();
    }

    return DUnexpected<> { DError { -1, "No such object path: " + path } };
}

DExpected<DBlockPartition *> DDeviceManager::createBlockPartition(const QString &path, QObject *parent)
{
    const auto &devs = blockDevices();
    if (!devs.hasValue())
        return DUnexpected<> { devs.error() };

    if (devs.value().contains(path))
        return new DBlockPartition(path, parent);
    return DUnexpected<> { DError { -1, "No such object path: " + path } };
}

DExpected<DBlockPartition *> DDeviceManager::createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent)
{
    const auto &devs = blockDevices();
    if (!devs.hasValue())
        return DUnexpected<> { devs.error() };

    for (const QString &block : devs.value()) {
        DBlockPartition *device = new DBlockPartition(block, parent);

        if (device->mountPoints().contains(path))
            return device;

        device->deleteLater();
    }

    return DUnexpected<> { DError { -1, "No such object path: " + path } };
}

DExpected<DDiskDrive *> DDeviceManager::createDiskDrive(const QString &path, QObject *parent)
{
    if (diskDrives().contains(path))
        return new DDiskDrive(path, parent);
    return DUnexpected<> { DError { -1, "No such object path: " + path } };
}

DExpected<DDiskJob *> DDeviceManager::createDiskJob(const QString &path, QObject *parent)
{
    return new DDiskJob(path, parent);
}

DExpected<DProtocolDevice *> DDeviceManager::createProtocolDevice(const QString &path, QObject *parent)
{
    return new DProtocolDevice(path, parent);
}

QStringList DDeviceManager::supportedFilesystems()
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    return udisksmgr.supportedFilesystems();
}

QStringList DDeviceManager::supportedEncryptionTypes()
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    return udisksmgr.supportedEncryptionTypes();
}

DExpected<QStringList> DDeviceManager::resolveDevice(QVariantMap devspec, QVariantMap options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    QStringList ret;
    auto r = udisksmgr.ResolveDevice(devspec, options);
    r.waitForFinished();
    if (!r.isError()) {
        const auto &devices = r.value();
        for (const auto &d : devices)
            ret.push_back(d.path());
        return ret;
    }
    return DUnexpected<> { DError { r.error().type(), r.error().message() } };
}

DExpected<QStringList> DDeviceManager::resolveDeviceNode(QString devnode, QVariantMap options)
{
    return resolveDevice({ { "path", QVariant(devnode) } }, options);
}

DExpected<bool> DDeviceManager::canCheck(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanCheck(type);
    r.waitForFinished();
    if (r.isError())
        return DUnexpected<> { DError { r.error().type(), r.error().message() } };

    if (requiredUtil)
        *requiredUtil = r.value().second;

    return r.value().first;
}

DExpected<bool> DDeviceManager::canFormat(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanFormat(type);
    r.waitForFinished();
    if (r.isError())
        return DUnexpected<> { DError { r.error().type(), r.error().message() } };

    if (requiredUtil)
        *requiredUtil = r.value().second;

    return r.value().first;
}

DExpected<bool> DDeviceManager::canRepair(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanRepair(type);
    r.waitForFinished();
    if (r.isError())
        return DUnexpected<> { DError { r.error().type(), r.error().message() } };

    if (requiredUtil)
        *requiredUtil = r.value().second;

    return r.value().first;
}

DExpected<bool> DDeviceManager::canResize(const QString &type, QString *requiredUtil)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    auto r = udisksmgr.CanResize(type);
    r.waitForFinished();
    if (r.isError())
        return DUnexpected<> { DError { r.error().type(), r.error().message() } };

    if (requiredUtil)
        *requiredUtil = r.value().second.second;

    return r.value().first;
}

DExpected<QString> DDeviceManager::loopSetup(int fd, QVariantMap options)
{
    OrgFreedesktopUDisks2ManagerInterface udisksmgr(kUDisks2Service, kUDisks2ManagerPath, QDBusConnection::systemBus());
    QDBusUnixFileDescriptor dbusfd;
    dbusfd.setFileDescriptor(fd);
    auto r = udisksmgr.LoopSetup(dbusfd, options);
    r.waitForFinished();
    if (r.isError())
        return DUnexpected<> { DError { r.error().type(), r.error().message() } };
    return r.value().path();
}

DMOUNT_END_NAMESPACE
