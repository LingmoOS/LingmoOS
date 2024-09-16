// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDISKMANAGER_H
#define DDISKMANAGER_H

#include <QObject>
#include <QMap>
#include <QDBusError>

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
class QStorageInfo;
QT_END_NAMESPACE

class DBlockDevice;
class DBlockPartition;
class DDiskDevice;
class DUDisksJob;
class DDiskManagerPrivate;
class DDiskManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DDiskManager)

    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges)

public:
    explicit DDiskManager(QObject *parent = nullptr);
    ~DDiskManager();

    Q_DECL_DEPRECATED_X("Use the static variant instead") QStringList blockDevices() const;
    QStringList diskDevices() const;

    static QStringList blockDevices(QVariantMap options);

    bool watchChanges() const;

    static QString objectPrintable(const QObject *object);
    static DBlockDevice *createBlockDevice(const QString &path, QObject *parent = nullptr);
    // device 路径以 '\0' 结尾
    DBlockDevice *createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent = nullptr) const;
    static DBlockPartition *createBlockPartition(const QString &path, QObject *parent = nullptr);
    // 挂载点以 '\0' 结尾
    DBlockPartition *createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent = nullptr) const;
    DBlockPartition *createBlockPartition(const QStorageInfo &info, QObject *parent = nullptr) const;
    static DDiskDevice *createDiskDevice(const QString &path, QObject *parent = nullptr);
    static DUDisksJob *createJob(const QString &path, QObject *parent = nullptr);

    static QStringList supportedFilesystems();
    static QStringList supportedEncryptionTypes();
    static QStringList resolveDevice(QVariantMap devspec, QVariantMap options);
    static QStringList resolveDeviceNode(QString devnode, QVariantMap options);
    static bool canCheck(const QString &type, QString *requiredUtil = nullptr);
    static bool canFormat(const QString &type, QString *requiredUtil = nullptr);
    static bool canRepair(const QString &type, QString *requiredUtil = nullptr);
    static bool canResize(const QString &type, QString *requiredUtil = nullptr);
    static QString loopSetup(int fd, QVariantMap options);

    static QDBusError lastError();

public Q_SLOTS:
    void setWatchChanges(bool watchChanges);

Q_SIGNALS:
    void blockDeviceAdded(const QString &path);
    void blockDeviceRemoved(const QString &path);
    void diskDeviceAdded(const QString &path);
    void diskDeviceRemoved(const QString &path);
    void fileSystemAdded(const QString &blockDevicePath);
    void fileSystemRemoved(const QString &blockDevicePath);
    void mountAdded(const QString &blockDevicePath, const QByteArray &mountPoint);
    void mountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint);
    void mountPointsChanged(const QString &blockDevicePath, const QByteArrayList &oldMountPoints, const QByteArrayList &newMountPoints);
    void jobAdded(const QString &jobPath);
    void opticalChanged(const QString &path);

private:
    QScopedPointer<DDiskManagerPrivate> d_ptr;

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &, const QMap<QString, QVariantMap> &);
    void onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties, const QDBusMessage &message);
};

#endif // DDISKMANAGER_H
