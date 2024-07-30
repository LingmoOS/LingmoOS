/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "disks.h"

#ifdef Q_OS_FREEBSD
#include <devstat.h>
#include <libgeom.h>
#endif

#include <QUrl>

#include <KIO/FileSystemFreeSpaceJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/Predicate>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

#include <systemstats/AggregateSensor.h>
#include <systemstats/SensorContainer.h>
#include <systemstats/SensorObject.h>

class VolumeObject : public KSysGuard::SensorObject
{
    Q_OBJECT
public:
    VolumeObject(const Solid::Device &device, KSysGuard::SensorContainer *parent);
    bool isRootDevice() const;
    void update();
    void setBytes(quint64 read, quint64 written, qint64 elapsedTime);

    const QString udi;
    const QString mountPoint;
private:
    static QString idHelper(const Solid::Device &device);

    KSysGuard::SensorProperty *m_name = nullptr;
    KSysGuard::SensorProperty *m_total = nullptr;
    KSysGuard::SensorProperty *m_used = nullptr;
    KSysGuard::SensorProperty *m_free = nullptr;
    KSysGuard::SensorProperty *m_readRate = nullptr;
    KSysGuard::SensorProperty *m_writeRate = nullptr;
    quint64 m_bytesRead = 0;
    quint64 m_bytesWritten = 0;
    bool m_rootDevice = false;
};

QString VolumeObject::idHelper(const Solid::Device &device)
{
    auto volume = device.as<Solid::StorageVolume>();
    auto block = device.as<Solid::Block>();
    if (!volume->uuid().isEmpty()) {
        return volume->uuid();
    } else if (!volume->label().isEmpty()) {
        return volume->label();
    } else {
        return QUrl(block->device()).fileName();
    }
}

VolumeObject::VolumeObject(const Solid::Device &device, KSysGuard::SensorContainer* parent)
    : SensorObject(idHelper(device), device.displayName(),  parent)
    , udi(device.udi())
    , mountPoint(device.is<Solid::StorageAccess>() ? device.as<Solid::StorageAccess>()->filePath() : QString())
{
    auto volume = device.as<Solid::StorageVolume>();

    m_name = new KSysGuard::SensorProperty("name", i18nc("@title", "Name"), device.displayName(), this);
    m_name->setShortName(i18nc("@title", "Name"));
    m_name->setVariantType(QVariant::String);

    m_total = new KSysGuard::SensorProperty("total", i18nc("@title", "Total Space"), volume->size(), this);
    m_total->setPrefix(name());
    m_total->setShortName(i18nc("@title Short for 'Total Space'", "Total"));
    m_total->setUnit(KSysGuard::UnitByte);
    m_total->setVariantType(QVariant::ULongLong);

    m_readRate = new KSysGuard::SensorProperty("read", i18nc("@title", "Read Rate"), 0, this);
    m_readRate->setPrefix(name());
    m_readRate->setShortName(i18nc("@title Short for 'Read Rate'", "Read"));
    m_readRate->setUnit(KSysGuard::UnitByteRate);
    m_readRate->setVariantType(QVariant::Double);

    m_writeRate = new KSysGuard::SensorProperty("write", i18nc("@title", "Write Rate"), 0, this);
    m_writeRate->setPrefix(name());
    m_writeRate->setShortName(i18nc("@title Short for 'Write Rate'", "Write"));
    m_writeRate->setUnit(KSysGuard::UnitByteRate);
    m_writeRate->setVariantType(QVariant::Double);

    if (volume->usage() != Solid::StorageVolume::PartitionTable) {
        m_used = new KSysGuard::SensorProperty("used", i18nc("@title", "Used Space"), this);
        m_used->setPrefix(name());
        m_used->setShortName(i18nc("@title Short for 'Used Space'", "Used"));
        m_used->setUnit(KSysGuard::UnitByte);
        m_used->setVariantType(QVariant::ULongLong);
        m_used->setMax(volume->size());

        m_free = new KSysGuard::SensorProperty("free", i18nc("@title", "Free Space"), this);
        m_free->setPrefix(name());
        m_free->setShortName(i18nc("@title Short for 'Free Space'", "Free"));
        m_free->setUnit(KSysGuard::UnitByte);
        m_free->setVariantType(QVariant::ULongLong);
        m_free->setMax(volume->size());

        auto usedPercent = new KSysGuard::PercentageSensor(this, "usedPercent", i18nc("@title", "Percentage Used"));
        usedPercent->setPrefix(name());
        usedPercent->setBaseSensor(m_used);

        auto freePercent = new KSysGuard::PercentageSensor(this, "freePercent", i18nc("@title", "Percentage Free"));
        freePercent->setPrefix(name());
        freePercent->setBaseSensor(m_free);
    } else {
        m_rootDevice = true;
    }
}

bool VolumeObject::isRootDevice() const
{
    return m_rootDevice;
}

void VolumeObject::update()
{
    if (mountPoint.isEmpty()) {
        // skip non-mounted partitions
        return;
    }
    auto job = KIO::fileSystemFreeSpace(QUrl::fromLocalFile(mountPoint));
    connect(job, &KJob::result, this, [this, job]() {
        if (!job->error()) {
            KIO::filesize_t size = job->size();
            KIO::filesize_t available = job->availableSize();
            m_total->setValue(size);
            m_free->setValue(available);
            m_free->setMax(size);
            m_used->setValue(size - available);
            m_used->setMax(size);
        }
    });
}

void VolumeObject::setBytes(quint64 read, quint64 written, qint64 elapsed)
{
    if (elapsed != 0) {
        double seconds = elapsed / 1000.0;
        m_readRate->setValue((read - m_bytesRead) / seconds);
        m_writeRate->setValue((written - m_bytesWritten) / seconds);
    }
    m_bytesRead = read;
    m_bytesWritten = written;
}

DisksPlugin::DisksPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto container = new KSysGuard::SensorContainer("disk", i18n("Disks"), this);
    auto storageVolumes = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (const auto &storageVolume : storageVolumes) {
       addDevice(storageVolume);
    }
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, [this] (const QString &udi) {
            addDevice(Solid::Device(udi));
    });
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, [this, container] (const QString &udi) {
        Solid::Device device(udi);
        if (device.isDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
            auto it = std::find_if(m_volumesByDevice.begin(), m_volumesByDevice.end(), [&udi] (VolumeObject *volume) {
                return volume->udi == udi;
            });
            if (it != m_volumesByDevice.end()) {
                container->removeObject(*it);
                m_volumesByDevice.erase(it);
            }
        }
    });
    addAggregateSensors();
#ifdef Q_OS_FREEBSD
    geom_stats_open();
#endif
}

DisksPlugin::~DisksPlugin()
{
#ifdef Q_OS_FREEBSD
    geom_stats_close();
#endif
}

void DisksPlugin::addDevice(const Solid::Device& device)
{
    auto container = containers()[0];
    const auto volume = device.as<Solid::StorageVolume>();
    if (!volume || (volume->isIgnored() && volume->usage() != Solid::StorageVolume::PartitionTable)) {
        return;
    }
    Solid::Device drive = device;
    // Only exlude volumes if we know that they are for sure not on a hard disk
    while (drive.isValid()) {
        if (drive.is<Solid::StorageDrive>()) {
            auto type = drive.as<Solid::StorageDrive>()->driveType();
            if (type == Solid::StorageDrive::HardDisk) {
                break;
            } else {
                return;
            }
        }
        drive = drive.parent();
    }
    if (volume->usage() == Solid::StorageVolume::PartitionTable) {
        auto block = device.as<Solid::Block>();
        m_volumesByDevice.insert(block->device(), new VolumeObject(device, container));
        return;
    }
    auto access = device.as<Solid::StorageAccess>();
    if (!access) {
        return;
    }
    if (access->filePath() != QString()) {
        createAccessibleVolumeObject(device);
    }
    connect(access, &Solid::StorageAccess::accessibilityChanged, this, [this, container] (bool accessible, const QString &udi) {
        if (accessible) {
            Solid::Device device(udi);
            createAccessibleVolumeObject(device);
        } else {
            auto it = std::find_if(m_volumesByDevice.begin(), m_volumesByDevice.end(), [&udi] (VolumeObject *disk) {
                return disk->udi == udi;
            });
            if (it != m_volumesByDevice.end()) {
                container->removeObject(*it);
                m_volumesByDevice.erase(it);
            }
        }
    });
}

void DisksPlugin::createAccessibleVolumeObject(const Solid::Device &device)
{
    auto block = device.as<Solid::Block>();
    auto access = device.as<Solid::StorageAccess>();
    Q_ASSERT(access->isAccessible());
    const QString  mountPoint = access->filePath();
    const bool hasMountPoint = std::any_of(m_volumesByDevice.cbegin(), m_volumesByDevice.cend(), [mountPoint] (const VolumeObject* volume) {
        return volume->mountPoint == mountPoint;
    });
    if (hasMountPoint) {
        return;
    }
    m_volumesByDevice.insert(block->device(), new VolumeObject(device,  containers()[0]));
}

void DisksPlugin::addAggregateSensors()
{
    auto container = containers()[0];
    auto allDisks = new KSysGuard::SensorObject("all", i18nc("@title", "All Disks"), container);

    auto filterFunction = [](const KSysGuard::SensorProperty *sensor) {
        if (sensor->parentObject()->id() == u"all"_qs) {
            return false;
        }

        auto volumeObject = qobject_cast<VolumeObject *>(sensor->parentObject());
        if (volumeObject && volumeObject->isRootDevice()) {
            return false;
        }

        return true;
    };

    auto total = new KSysGuard::AggregateSensor(allDisks, "total", i18nc("@title", "Total Space"));
    total->setShortName(i18nc("@title Short for 'Total Space'", "Total"));
    total->setUnit(KSysGuard::UnitByte);
    total->setVariantType(QVariant::ULongLong);
    total->setMatchSensors(QRegularExpression("^.*$"), "total");
    total->setFilterFunction(filterFunction);

    auto free = new KSysGuard::AggregateSensor(allDisks, "free", i18nc("@title", "Free Space"));
    free->setShortName(i18nc("@title Short for 'Free Space'", "Free"));
    free->setUnit(KSysGuard::UnitByte);
    free->setVariantType(QVariant::ULongLong);
    free->setMax(total->value().toULongLong());
    free->setMatchSensors(QRegularExpression("^.*$"), "free");
    free->setFilterFunction(filterFunction);

    auto used = new KSysGuard::AggregateSensor(allDisks, "used", i18nc("@title", "Used Space"));
    used->setShortName(i18nc("@title Short for 'Used Space'", "Used"));
    used->setUnit(KSysGuard::UnitByte);
    used->setVariantType(QVariant::ULongLong);
    used->setMax(total->value().toULongLong());
    used->setMatchSensors(QRegularExpression("^.*$"), "used");
    used->setFilterFunction(filterFunction);

    auto readRate = new KSysGuard::AggregateSensor(allDisks, "read", i18nc("@title", "Read Rate"), 0);
    readRate->setShortName(i18nc("@title Short for 'Read Rate'", "Read"));
    readRate->setUnit(KSysGuard::UnitByteRate);
    readRate->setVariantType(QVariant::Double);
    readRate->setMatchSensors(QRegularExpression("^(?!all).*$"), "read");
    readRate->setFilterFunction(filterFunction);

    auto writeRate = new KSysGuard::AggregateSensor(allDisks, "write", i18nc("@title", "Write Rate"), 0);
    writeRate->setShortName(i18nc("@title Short for 'Write Rate'", "Write"));
    writeRate->setUnit(KSysGuard::UnitByteRate);
    writeRate->setVariantType(QVariant::Double);
    writeRate->setMatchSensors(QRegularExpression("^(?!all).*$"), "write");
    writeRate->setFilterFunction(filterFunction);

    auto freePercent = new KSysGuard::PercentageSensor(allDisks, "freePercent", i18nc("@title", "Percentage Free"));
    freePercent->setShortName(i18nc("@title, Short for `Percentage Free", "Free"));
    freePercent->setBaseSensor(free);

    auto usedPercent = new KSysGuard::PercentageSensor(allDisks, "usedPercent", i18nc("@title", "Percentage Used"));
    usedPercent->setShortName(i18nc("@title, Short for `Percentage Used", "Used"));
    usedPercent->setBaseSensor(used);

    connect(total, &KSysGuard::SensorProperty::valueChanged, this, [total, free, used] () {
        free->setMax(total->value().toULongLong());
        used->setMax(total->value().toULongLong());
    });
}

void DisksPlugin::update()
{
    bool anySubscribed = false;
    for (auto volume : m_volumesByDevice) {
        if (volume->isSubscribed()) {
            anySubscribed = true;
            volume->update();
        }
    }

    if (!anySubscribed) {
        return;
    }

    qint64 elapsed = 0;
    if (m_elapsedTimer.isValid()) {
        elapsed = m_elapsedTimer.restart();
    } else {
        m_elapsedTimer.start();
    }
#if defined Q_OS_LINUX
    QFile diskstats("/proc/diskstats");
    if (!diskstats.exists()) {
        return;
    }
    diskstats.open(QIODevice::ReadOnly | QIODevice::Text);
    /* procfs-diskstats (See https://www.kernel.org/doc/Documentation/ABI/testing/procfs-diskstats)
    The /proc/diskstats file displays the I/O statistics
    of block devices. Each line contains the following 14
    fields:
    - major number
    - minor mumber
    - device name
    - reads completed successfully
    - reads merged
    - sectors read
    - time spent reading (ms)
    - writes completed
    - writes merged
    - sectors written
    [...]
    */
    for (QByteArray line = diskstats.readLine(); !line.isNull(); line = diskstats.readLine()) {
        QList<QByteArray> fields = line.simplified().split(' ');
        const QString device = QStringLiteral("/dev/%1").arg(QString::fromLatin1(fields[2]));
        if (m_volumesByDevice.contains(device)) {
            // A sector as reported in diskstats is 512 Bytes, see https://stackoverflow.com/a/38136179
            m_volumesByDevice[device]->setBytes(fields[5].toULongLong() * 512, fields[9].toULongLong() * 512, elapsed);
        }
    }
#elif defined Q_OS_FREEBSD
    std::unique_ptr<void, decltype(&geom_stats_snapshot_free)> stats(geom_stats_snapshot_get(), geom_stats_snapshot_free);
    gmesh mesh;
    geom_gettree(&mesh);
    while (devstat *dstat = geom_stats_snapshot_next(stats.get())) {
        gident *id = geom_lookupid(&mesh, dstat->id);
        if (id && id->lg_what == gident::ISPROVIDER) {
            auto provider = static_cast<gprovider*>(id->lg_ptr);
            const QString device = QStringLiteral("/dev/%1").arg(QString::fromLatin1(provider->lg_name));
            if (m_volumesByDevice.contains(device)) {
                uint64_t bytesRead, bytesWritten;
                devstat_compute_statistics(dstat, nullptr, 0, DSM_TOTAL_BYTES_READ, &bytesRead, DSM_TOTAL_BYTES_WRITE, &bytesWritten, DSM_NONE);
                m_volumesByDevice[device]->setBytes(bytesRead, bytesWritten, elapsed);
            }
        }
    }
    geom_deletetree(&mesh);
#endif
}

K_PLUGIN_CLASS_WITH_JSON(DisksPlugin, "metadata.json")
#include "disks.moc"

#include "moc_disks.cpp"
