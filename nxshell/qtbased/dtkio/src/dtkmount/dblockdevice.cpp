// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dblockdevice_p.h"
#include "objectmanager_interface.h"
#include "udisks2_interface.h"
#include "dbus/udisks2_dbus_common.h"

#include <QDBusUnixFileDescriptor>


DMOUNT_BEGIN_NAMESPACE

DBlockDevicePrivate::DBlockDevicePrivate(DBlockDevice *qq)
    : q_ptr(qq)
{
}

void DBlockDevicePrivate::onInterfacesAdded(const QDBusObjectPath &objectPath, const QMap<QString, QVariantMap> &interfacesAndProperties)
{
    Q_Q(DBlockDevice);

    const QString &path = objectPath.path();

    if (path != dbus->path())
        return;

    if (interfacesAndProperties.contains(QString { kUDisks2Service } + ".Filesystem"))
        Q_EMIT q->hasFileSystemChanged(true);

    if (interfacesAndProperties.contains(QString { kUDisks2Service } + ".Partition"))
        Q_EMIT q->hasPartitionChanged(true);

    if (interfacesAndProperties.contains(QString { kUDisks2Service } + ".Encrypted"))
        Q_EMIT q->isEncryptedChanged(true);
}

void DBlockDevicePrivate::onInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    Q_Q(DBlockDevice);

    const QString &path = objectPath.path();

    if (path != dbus->path())
        return;

    for (const QString &i : interfaces) {
        if (i == QString { kUDisks2Service } + ".Filesystem") {
            Q_EMIT q->hasFileSystemChanged(false);
        } else if (i == QString { kUDisks2Service } + ".Partition") {
            Q_EMIT q->hasPartitionChanged(false);
        } else if (i == QString { kUDisks2Service } + ".Encrypted") {
            Q_EMIT q->isEncryptedChanged(false);
        }
    }
}

void DBlockDevicePrivate::onPropertiesChanged(const QString &interface, const QVariantMap &changedProperties)
{
    Q_Q(DBlockDevice);

    if (interface.endsWith(".PartitionTable")) {
        auto begin = changedProperties.begin();

        while (begin != changedProperties.constEnd()) {
            if (begin.key() == "Type") {
                Q_EMIT q->ptTypeChanged();
                break;
            }

            ++begin;
        }
    } else {
        auto begin = changedProperties.begin();
        for (; begin != changedProperties.constEnd(); ++begin) {
            QString propertyName = begin.key();
            int pindex = this->metaObject()->indexOfProperty(propertyName.toLatin1().constData());

            if (pindex < 0) {
                propertyName[0] = propertyName.at(0).toLower();
                pindex = this->metaObject()->indexOfProperty(propertyName.toLatin1().constData());
            }

            if (pindex < 0)
                continue;

            const QMetaProperty &mp = this->metaObject()->property(pindex);
            if (!mp.hasNotifySignal())
                continue;

            mp.notifySignal().invoke(this, QGenericArgument(begin.value().typeName(), begin.value().constData()));
        }
    }
}

DBlockDevice::DBlockDevice(const QString &path, QObject *parent)
    : DBlockDevice(*new DBlockDevicePrivate(this), path, parent)
{
}

DBlockDevice::DBlockDevice(DBlockDevicePrivate &dd, const QString &path, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{

    dd.dbus = new OrgFreedesktopUDisks2BlockInterface(kUDisks2Service, path, QDBusConnection::systemBus(), this);

    connect(this, &DBlockDevice::idTypeChanged, this, &DBlockDevice::fsTypeChanged);
}

DBlockDevice::~DBlockDevice()
{
}

bool DBlockDevice::isValid() const
{
    Q_D(const DBlockDevice);

    return d->dbus->isValid();
}

void DBlockDevice::setWatchChanges(bool watchChanges)
{
    Q_D(DBlockDevice);

    if (d->watchChanges == watchChanges)
        return;

    d->watchChanges = watchChanges;

    OrgFreedesktopDBusObjectManagerInterface *objectManager = UDisks2::objectManager();
    auto sb = QDBusConnection::systemBus();

    if (watchChanges) {
        connect(objectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                d, &DBlockDevicePrivate::onInterfacesAdded);
        connect(objectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                d, &DBlockDevicePrivate::onInterfacesRemoved);

        sb.connect(kUDisks2Service, d->dbus->path(), "org.freedesktop.DBus.Properties",
                   "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
    } else {
        disconnect(objectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
                   d, &DBlockDevicePrivate::onInterfacesAdded);
        disconnect(objectManager, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
                   d, &DBlockDevicePrivate::onInterfacesRemoved);

        sb.disconnect(kUDisks2Service, d->dbus->path(), "org.freedesktop.DBus.Properties",
                      "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
    }
}

bool DBlockDevice::watchChanges() const
{
    Q_D(const DBlockDevice);

    return d->watchChanges;
}

/*!
 * @~english \brief Get the block device (patition) DBus path.
 *
 * @~english \return the DBus path
 */
QString DBlockDevice::path() const
{
    Q_D(const DBlockDevice);

    return d->dbus->path();
}

QList<QPair<QString, QVariantMap>> DBlockDevice::configuration() const
{
    Q_D(const DBlockDevice);

    return d->dbus->configuration();
}

QString DBlockDevice::cryptoBackingDevice() const
{
    Q_D(const DBlockDevice);

    return d->dbus->cryptoBackingDevice().path();
}

/*!
 * @~english \brief Get the device path, like `/dev/sda1`
 *
 * @~english \return Device path in QByteArray.
 */
QByteArray DBlockDevice::device() const
{
    Q_D(const DBlockDevice);

    return d->dbus->device();
}

quint64 DBlockDevice::deviceNumber() const
{
    Q_D(const DBlockDevice);

    return d->dbus->deviceNumber();
}

/*!
 * @~english \brief Get the disk device dbus path
 *
 * @~english \return a dbus path in QString
 */
QString DBlockDevice::drive() const
{
    Q_D(const DBlockDevice);

    return d->dbus->drive().path();
}

bool DBlockDevice::hintAuto() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintAuto();
}

QString DBlockDevice::hintIconName() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintIconName();
}

bool DBlockDevice::hintIgnore() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintIgnore();
}

QString DBlockDevice::hintName() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintName();
}

bool DBlockDevice::hintPartitionable() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintPartitionable();
}

QString DBlockDevice::hintSymbolicIconName() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintSymbolicIconName();
}

bool DBlockDevice::hintSystem() const
{
    Q_D(const DBlockDevice);

    return d->dbus->hintSystem();
}

QString DBlockDevice::id() const
{
    Q_D(const DBlockDevice);

    return d->dbus->id();
}

QString DBlockDevice::idLabel() const
{
    Q_D(const DBlockDevice);

    return d->dbus->idLabel();
}

QString DBlockDevice::idType() const
{
    Q_D(const DBlockDevice);

    return d->dbus->idType();
}

QString DBlockDevice::idUUID() const
{
    Q_D(const DBlockDevice);

    return d->dbus->idUUID();
}

QString DBlockDevice::idUsage() const
{
    Q_D(const DBlockDevice);

    return d->dbus->idUsage();
}

QString DBlockDevice::idVersion() const
{
    Q_D(const DBlockDevice);

    return d->dbus->idVersion();
}

QString DBlockDevice::mDRaid() const
{
    Q_D(const DBlockDevice);

    return d->dbus->mDRaid().path();
}

QString DBlockDevice::mDRaidMember() const
{
    Q_D(const DBlockDevice);

    return d->dbus->mDRaidMember().path();
}

QByteArray DBlockDevice::preferredDevice() const
{
    Q_D(const DBlockDevice);

    return d->dbus->preferredDevice();
}

bool DBlockDevice::readOnly() const
{
    Q_D(const DBlockDevice);

    return d->dbus->readOnly();
}

quint64 DBlockDevice::size() const
{
    Q_D(const DBlockDevice);

    return d->dbus->size();
}

QByteArrayList DBlockDevice::symlinks() const
{
    Q_D(const DBlockDevice);

    return d->dbus->symlinks();
}

QStringList DBlockDevice::userspaceMountOptions() const
{
    Q_D(const DBlockDevice);

    return d->dbus->userspaceMountOptions();
}

bool DBlockDevice::hasFileSystem() const
{
    Q_D(const DBlockDevice);

    return hasFileSystem(d->dbus->path());
}

bool DBlockDevice::hasPartition() const
{
    Q_D(const DBlockDevice);

    return hasPartition(d->dbus->path());
}

bool DBlockDevice::isEncrypted() const
{
    Q_D(const DBlockDevice);

    return isEncrypted(d->dbus->path());
}

bool DBlockDevice::isLoopDevice() const
{
    Q_D(const DBlockDevice);

    return isLoopDevice(d->dbus->path());
}

bool DBlockDevice::hasFileSystem(const QString &path)
{
    return UDisks2::interfaceExists(path, QString { kUDisks2Service } + ".Filesystem");
}

bool DBlockDevice::hasPartition(const QString &path)
{
    return UDisks2::interfaceExists(path, QString { kUDisks2Service } + ".Partition");
}

bool DBlockDevice::isEncrypted(const QString &path)
{
    return UDisks2::interfaceExists(path, QString { kUDisks2Service } + ".Encrypted");
}

bool DBlockDevice::isLoopDevice(const QString &path)
{
    return UDisks2::interfaceExists(path, QString { kUDisks2Service } + ".Loop");
}

QByteArrayList DBlockDevice::mountPoints() const
{
    if (!hasFileSystem())
        return QByteArrayList();

    Q_D(const DBlockDevice);

    OrgFreedesktopUDisks2FilesystemInterface fsif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    return fsif.mountPoints();
}

DBlockDevice::PartitionTableType DBlockDevice::partitionTableType() const
{
    Q_D(const DBlockDevice);

    if (!UDisks2::interfaceExists(d->dbus->path(), QString { kUDisks2Service } + ".PartitionTable"))
        return InvalidPT;

    OrgFreedesktopUDisks2PartitionTableInterface ptif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());

    const QString &type = ptif.type();

    if (type.isEmpty())
        return InvalidPT;

    if (type == "dos")
        return MBR;

    if (type == "gpt")
        return GPT;

    return UnknownPT;
}

QList<QPair<QString, QVariantMap>> DBlockDevice::childConfiguration() const
{
    Q_D(const DBlockDevice);

    if (!isEncrypted())
        return QList<QPair<QString, QVariantMap>>();

    OrgFreedesktopUDisks2EncryptedInterface eif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    return eif.childConfiguration();
}
void DBlockDevice::format(const QString &type, const QVariantMap &options)
{
    Q_D(DBlockDevice);

    d->dbus->setTimeout(INT_MAX);
    auto r = d->dbus->Format(type, options);
    r.waitForFinished();
    d->err = r.error();
    d->dbus->setTimeout(-1);
}

QList<QPair<QString, QVariantMap>> DBlockDevice::getSecretConfiguration(const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->GetSecretConfiguration(options);
    r.waitForFinished();
    d->err = r.error();
    return r.value();
}

void DBlockDevice::addConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->AddConfigurationItem(item, options);
    r.waitForFinished();
    d->err = r.error();
}

void DBlockDevice::removeConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->RemoveConfigurationItem(item, options);
    r.waitForFinished();
    d->err = r.error();
}

void DBlockDevice::updateConfigurationItem(const QPair<QString, QVariantMap> &oldItem, const QPair<QString, QVariantMap> &newItem, const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->UpdateConfigurationItem(oldItem, newItem, options);
    r.waitForFinished();
    d->err = r.error();
}

int DBlockDevice::openDevice(const QString &mode, const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->OpenDevice(mode, options);
    r.waitForFinished();
    d->err = r.error();
    return r.value().fileDescriptor();
}

int DBlockDevice::openForBackup(const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->OpenForBackup(options);
    r.waitForFinished();
    d->err = r.error();
    return r.value().fileDescriptor();
}

int DBlockDevice::openForBenchmark(const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->OpenForBenchmark(options);
    r.waitForFinished();
    d->err = r.error();
    return r.value().fileDescriptor();
}

int DBlockDevice::openForRestore(const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->OpenForRestore(options);
    r.waitForFinished();
    d->err = r.error();
    return r.value().fileDescriptor();
}

void DBlockDevice::rescan(const QVariantMap &options)
{
    Q_D(DBlockDevice);

    auto r = d->dbus->Rescan(options);
    r.waitForFinished();
    d->err = r.error();
}

/*!
 * @~english \brief Mount the block device.
 *
 * @~english \param options please refer to \l {http://storaged.org/doc/udisks2-api/latest/gdbus-org.freedesktop.UDisks2.Filesystem.html#gdbus-method-org-freedesktop-UDisks2-Filesystem.Mount} {UDisks2.Filesystem Mount()}
 *
 * @~english \return the mount point path.
 *
 * @~english \sa unmount()
 */
QString DBlockDevice::mount(const QVariantMap &options)
{
    if (!hasFileSystem())
        return QString();

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2FilesystemInterface fsif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());

    auto r = fsif.Mount(options);
    r.waitForFinished();
    d->err = r.error();
    return r.value();
}

void DBlockDevice::unmount(const QVariantMap &options)
{
    if (!hasFileSystem())
        return;

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2FilesystemInterface fsif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    auto r = fsif.Unmount(options);
    r.waitForFinished();
    d->err = r.error();
}

/*!
 * @~english \brief Check if we can set the filesystem label.
 *
 * @~english \return
 */
bool DBlockDevice::canSetLabel() const
{
    if (!hasFileSystem())
        return false;

    if (!mountPoints().isEmpty())
        return false;

    return true;
}

/*!
 * @~english \brief Sets the filesystem label.
 *
 * @~english \param options Options (currently unused except for standard options).
 */
void DBlockDevice::setLabel(const QString &label, const QVariantMap &options)
{
    if (!hasFileSystem())
        return;

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2FilesystemInterface fsif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    auto r = fsif.SetLabel(label, options);
    r.waitForFinished();
    d->err = r.error();
}

void DBlockDevice::changePassphrase(const QString &passphrase, const QString &new_passphrase, const QVariantMap &options)
{
    if (!isEncrypted())
        return;

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2EncryptedInterface eif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    auto r = eif.ChangePassphrase(passphrase, new_passphrase, options);
    r.waitForFinished();
    d->err = r.error();
}

void DBlockDevice::lock(const QVariantMap &options)
{
    if (!isEncrypted())
        return;

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2EncryptedInterface eif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    QDBusPendingReply<void> r = eif.Lock(options);
    r.waitForFinished();
    d->err = r.error();
}

QString DBlockDevice::unlock(const QString &passphrase, const QVariantMap &options)
{
    if (!isEncrypted())
        return QString();

    Q_D(DBlockDevice);

    OrgFreedesktopUDisks2EncryptedInterface eif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());

    auto r = eif.Unlock(passphrase, options);
    r.waitForFinished();
    d->err = r.error();
    return r.value().path();
}

QString DBlockDevice::cleartextDevice()
{
    if (!isEncrypted())
        return QString();

    Q_D(const DBlockDevice);

    OrgFreedesktopUDisks2EncryptedInterface eif(kUDisks2Service, d->dbus->path(), QDBusConnection::systemBus());
    return eif.cleartextDevice().path();
}

DMOUNT_END_NAMESPACE
