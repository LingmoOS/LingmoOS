// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKDEVICE_H
#define DBLOCKDEVICE_H

#include <DtkMountGlobal>

#include <QObject>
#include <QDBusError>
#include <QVariantMap>

#include <DExpected>

QT_BEGIN_NAMESPACE
class QDBusObjectPath;
QT_END_NAMESPACE

DCORE_USE_NAMESPACE
DMOUNT_BEGIN_NAMESPACE

class DBlockDevice;
namespace DDeviceManager {
DExpected<DBlockDevice *> createBlockDevice(const QString &path, QObject *parent);
DExpected<DBlockDevice *> createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent);
}   // namespace DDeviceManager

class DBlockDevicePrivate;
class DBlockDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DBlockDevice)

    Q_PROPERTY(QString path READ path CONSTANT FINAL);
    Q_PROPERTY(bool watchChanges READ watchChanges WRITE setWatchChanges);
    Q_PROPERTY(bool hasFileSystem READ hasFileSystem NOTIFY hasFileSystemChanged);
    Q_PROPERTY(bool hasPartition READ hasPartition NOTIFY hasPartitionChanged);
    Q_PROPERTY(bool isEncrypted READ isEncrypted NOTIFY isEncryptedChanged);
    Q_PROPERTY(bool isLoopDevice READ isLoopDevice CONSTANT);
    Q_PROPERTY(QList<QPair<QString, QVariantMap>> configuration READ configuration NOTIFY configurationChanged);
    Q_PROPERTY(QString cryptoBackingDevice READ cryptoBackingDevice NOTIFY cryptoBackingDeviceChanged);
    Q_PROPERTY(QByteArray device READ device CONSTANT);
    Q_PROPERTY(quint64 deviceNumber READ deviceNumber CONSTANT);
    Q_PROPERTY(QString drive READ drive CONSTANT);
    Q_PROPERTY(bool hintAuto READ hintAuto NOTIFY hintAutoChanged);
    Q_PROPERTY(QString hintIconName READ hintIconName NOTIFY hintIconNameChanged);
    Q_PROPERTY(bool hintIgnore READ hintIgnore NOTIFY hintIgnoreChanged);
    Q_PROPERTY(QString hintName READ hintName NOTIFY hintNameChanged);
    Q_PROPERTY(bool hintPartitionable READ hintPartitionable NOTIFY hintPartitionableChanged);
    Q_PROPERTY(QString hintSymbolicIconName READ hintSymbolicIconName NOTIFY hintSymbolicIconNameChanged);
    Q_PROPERTY(bool hintSystem READ hintSystem CONSTANT);
    Q_PROPERTY(QString id READ id CONSTANT);
    Q_PROPERTY(QString idLabel READ idLabel NOTIFY idLabelChanged);
    Q_PROPERTY(QString idType READ idType NOTIFY idTypeChanged);
    // TODO(zhangs); Q_PROPERTY(FSType fsType READ fsType NOTIFY fsTypeChanged);
    Q_PROPERTY(QString idUUID READ idUUID NOTIFY idUUIDChanged);
    Q_PROPERTY(QString idUsage READ idUsage NOTIFY idUsageChanged);
    Q_PROPERTY(QString idVersion READ idVersion NOTIFY idVersionChanged);
    Q_PROPERTY(QString mDRaid READ mDRaid NOTIFY mDRaidChanged);
    Q_PROPERTY(QString mDRaidMember READ mDRaidMember NOTIFY mDRaidMemberChanged);
    Q_PROPERTY(QByteArray preferredDevice READ preferredDevice NOTIFY preferredDeviceChanged);
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged);
    Q_PROPERTY(quint64 size READ size NOTIFY sizeChanged);
    Q_PROPERTY(QByteArrayList symlinks READ symlinks NOTIFY symlinksChanged);
    Q_PROPERTY(QStringList userspaceMountOptions READ userspaceMountOptions NOTIFY userspaceMountOptionsChanged);
    Q_PROPERTY(QByteArrayList mountPoints READ mountPoints NOTIFY mountPointsChanged);
    Q_PROPERTY(PartitionTableType partitionTableType READ partitionTableType NOTIFY ptTypeChanged);
    Q_PROPERTY(QList<QPair<QString, QVariantMap>> childConfiguration READ childConfiguration NOTIFY childConfigurationChanged);
    Q_PROPERTY(QString cleartextDevice READ cleartextDevice NOTIFY cleartextDeviceChanged);

public:
    enum PartitionTableType {
        InvalidPT,
        MBR,
        GPT,
        UnknownPT
    };

    Q_ENUM(PartitionTableType)

    ~DBlockDevice() override;

public:
    bool isValid() const;
    void setWatchChanges(bool watchChanges);
    bool watchChanges() const;

    QString path() const;
    QList<QPair<QString, QVariantMap>> configuration() const;
    QString cryptoBackingDevice() const;
    QByteArray device() const;
    quint64 deviceNumber() const;
    QString drive() const;
    bool hintAuto() const;
    QString hintIconName() const;
    bool hintIgnore() const;
    QString hintName() const;
    bool hintPartitionable() const;
    QString hintSymbolicIconName() const;
    bool hintSystem() const;
    QString id() const;
    QString idLabel() const;
    QString idType() const;
    // TODO(zhangs): FSType fsType() const;
    QString idUUID() const;
    QString idUsage() const;
    QString idVersion() const;
    QString mDRaid() const;
    QString mDRaidMember() const;
    QByteArray preferredDevice() const;
    bool readOnly() const;
    quint64 size() const;
    QByteArrayList symlinks() const;
    QStringList userspaceMountOptions() const;
    bool canSetLabel() const;

    bool hasFileSystem() const;
    bool hasPartition() const;
    bool isEncrypted() const;
    bool isLoopDevice() const;

    static bool hasFileSystem(const QString &path);
    static bool hasPartition(const QString &path);
    static bool isEncrypted(const QString &path);
    static bool isLoopDevice(const QString &path);

    QByteArrayList mountPoints() const;
    PartitionTableType partitionTableType() const;

    QList<QPair<QString, QVariantMap>> childConfiguration() const;

public Q_SLOTS:
    void format(const QString &type, const QVariantMap &options);
    // TODO(zhangs): void format(const FSType &type, const QVariantMap &options);
    QList<QPair<QString, QVariantMap>> getSecretConfiguration(const QVariantMap &options);
    void addConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options);
    void removeConfigurationItem(const QPair<QString, QVariantMap> &item, const QVariantMap &options);
    void updateConfigurationItem(const QPair<QString, QVariantMap> &oldItem, const QPair<QString, QVariantMap> &newItem, const QVariantMap &options);
    int openDevice(const QString &mode, const QVariantMap &options);
    int openForBackup(const QVariantMap &options);
    int openForBenchmark(const QVariantMap &options);
    int openForRestore(const QVariantMap &options);
    void rescan(const QVariantMap &options);

    QString mount(const QVariantMap &options);
    void unmount(const QVariantMap &options);
    // TODO(zhangs): async interfaces
    void setLabel(const QString &label, const QVariantMap &options);

    void changePassphrase(const QString &passphrase, const QString &newPassphrase, const QVariantMap &options);
    void lock(const QVariantMap &options);
    QString unlock(const QString &passphrase, const QVariantMap &options);
    // TODO(zhangs): async interfaces
    QString cleartextDevice();

Q_SIGNALS:
    void hasFileSystemChanged(bool hasFileSystem);
    void hasPartitionChanged(bool hasPartition);
    void isEncryptedChanged(bool isEncrypted);
    void configurationChanged(QList<QPair<QString, QVariantMap>> configuration);
    void cryptoBackingDeviceChanged(QString cryptoBackingDevice);
    void hintAutoChanged(bool hintAuto);
    void hintIconNameChanged(QString hintIconName);
    void hintIgnoreChanged(bool hintIgnore);
    void hintNameChanged(QString hintName);
    void hintPartitionableChanged(bool hintPartitionable);
    void hintSymbolicIconNameChanged(QString hintSymbolicIconName);
    void idLabelChanged(QString idLabel);
    void idTypeChanged(QString idType);
    void fsTypeChanged(QString fsType);
    void idUUIDChanged(QString idUUID);
    void idUsageChanged(QString idUsage);
    void idVersionChanged(QString idVersion);
    void mDRaidChanged(QString mDRaid);
    void mDRaidMemberChanged(QString mDRaidMember);
    void preferredDeviceChanged(QByteArray preferredDevice);
    void readOnlyChanged(bool readOnly);
    void sizeChanged(quint64 size);
    void symlinksChanged(QByteArrayList symlinks);
    void userspaceMountOptionsChanged(QStringList userspaceMountOptions);
    void ptTypeChanged();
    void mountPointsChanged(const QByteArrayList &mountPoints);
    void childConfigurationChanged(QList<QPair<QString, QVariantMap>> childConfiguration);
    void cleartextDeviceChanged(const QString &cleartextDevice);

protected:
    explicit DBlockDevice(const QString &path, QObject *parent = nullptr);
    explicit DBlockDevice(DBlockDevicePrivate &dd, const QString &path, QObject *parent = nullptr);

    QScopedPointer<DBlockDevicePrivate> d_ptr;

    friend DExpected<DBlockDevice *> DDeviceManager::createBlockDevice(const QString &path, QObject *parent);
    friend DExpected<DBlockDevice *> DDeviceManager::createBlockDeviceByDevicePath(const QByteArray &path, QObject *parent);
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKDEVICE_H
