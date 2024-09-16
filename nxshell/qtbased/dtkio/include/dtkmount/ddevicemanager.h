// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDEVICEMANAGER_H
#define DDEVICEMANAGER_H

#include <DtkMountGlobal>

#include <QObject>
#include <QVariantMap>

#include <DExpected>

DCORE_USE_NAMESPACE
DMOUNT_BEGIN_NAMESPACE

class DBlockDeviceMonitor;
class DProtocolDeviceMonitor;
class DBlockDevice;
class DBlockPartition;
class DDiskDrive;
class DDiskJob;
class DProtocolDevice;

namespace DDeviceManager {
DBlockDeviceMonitor *globalBlockDeviceMonitor();
DProtocolDeviceMonitor *globalProtocolDeviceMonitor();
DExpected<QStringList> blockDevices(const QVariantMap &options = {});
QStringList protocolDevices();
QStringList diskDrives();

[[nodiscard]] DExpected<DBlockDevice *> createBlockDevice(const QString &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DBlockDevice *> createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DBlockPartition *> createBlockPartition(const QString &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DBlockPartition *> createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DDiskDrive *> createDiskDrive(const QString &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DDiskJob *> createDiskJob(const QString &path, QObject *parent = nullptr);
[[nodiscard]] DExpected<DProtocolDevice *> createProtocolDevice(const QString &path, QObject *parent = nullptr);

QStringList supportedFilesystems();
QStringList supportedEncryptionTypes();
DExpected<QStringList> resolveDevice(QVariantMap devspec, QVariantMap options);
DExpected<QStringList> resolveDeviceNode(QString devnode, QVariantMap options);
DExpected<bool> canCheck(const QString &type, QString *requiredUtil = nullptr);
DExpected<bool> canFormat(const QString &type, QString *requiredUtil = nullptr);
DExpected<bool> canRepair(const QString &type, QString *requiredUtil = nullptr);
DExpected<bool> canResize(const QString &type, QString *requiredUtil = nullptr);
DExpected<QString> loopSetup(int fd, QVariantMap options);

// TODO(zhangs): error handle

}   // namespace DDeviceManager

DMOUNT_END_NAMESPACE

#endif   // DDEVICEMANAGER_H
