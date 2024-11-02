#include "full_partition_delegate.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QBitArray>
#include <sys/sysinfo.h>
#include <math.h>
#include <QDebug>
#include "partman/partition_unit.h"
#include "partman/filesystem.h"
#include "partman/operationdisk.h"
#include "partition_delegate.h"

namespace KInstaller {
#define DISKMINSIZE 20
#define DISKLIMITESIZE 32

FullPartitionDelegate::FullPartitionDelegate(QObject* parent) :PartitionDelegate(parent),
    m_quickInstall(false)
{
    this->setObjectName("Quick_disk_delegate");

}

Device::Ptr FullPartitionDelegate::fullDiskScheme(Device::Ptr device)
{
    Device::Ptr fake(new Device());
    for(Device::Ptr dev : m_selDevices) {
        if(dev->m_path != device->m_path)
            continue;
        fake = Device::Ptr(new Device(*dev));
        fake->partitions.clear();

        for(auto partition : dev->partitions) {
            if(partition->m_sectorEnd < 0 || partition->m_sectorStart < 0 || partition->m_type == PartitionType::Extended) {
                continue;
            }
            Partition::Ptr p(new Partition(*partition));
            p->m_type = PartitionType::Unallocated;
            p->m_total = p->m_sectorEnd - p->m_sectorStart +1;
            fake->partitions.append(p);
        }

        std::sort(fake->partitions.begin(), fake->partitions.end(), [=](Partition::Ptr partition1, Partition::Ptr partition2) {
            return partition1->m_sectorStart < partition2->m_sectorStart;
        });
        return fake;
    }
    return fake;

}

bool FullPartitionDelegate::formatWholeDevcie(const QString &devPath, PartTableType type)
{
    qDebug() << "formatWholeDevcie()" << devPath;

    const int devIndex = deviceIndex(m_virtualDevices, devPath);
    if(devIndex == -1) {
        qCritical() << "formatWholeDevcie() device index out of range:" << devPath;
        return false;
    }
    Device::Ptr device = m_virtualDevices[devIndex];
    qDebug() << "device selected to format:" << device;

    Device::Ptr newDev(new Device(*device));
    newDev->partitions.clear();
    newDev->m_partTableType = type;
    OperationDisk operation(newDev);
    m_operations.append(operation);
    operation.applyToShow(device);

    if(device->partitions.length() == 0) {
        qCritical() << "partition is empty" << device;
        return false;
    }
    const QByteArray& policyStr{getQuickDiskInstallPolicy()};
    if(policyStr.isEmpty()) {
        qWarning() << "Quick Disk Install policy is empty!" ;
        return false;
    }

    const QJsonArray& policyArray = QJsonDocument::fromJson(policyStr).array();

    if(policyArray.isEmpty()) {
        qWarning() << "Quick Disk Install policy is empty!";
        return false;
    }

    const int swapSize = getSwapSize();
    qint64 lastDeviceLen{device->m_length};

    Partition::Ptr unallocated = device->partitions.last();

    if(device->m_partTableType == PartTableType::GPT) {
        const qint64 uefiSize = parsePartitionSize("300Mib", lastDeviceLen * device->m_sectorSize);

        bool ok;
        QString str = "/boot/efi";
        ok = createPrimaryPartition(unallocated, PartitionType::Normal, true, FSType::Ext4,
                                    str, uefiSize / device->m_sectorSize);

        if(!ok) {
            qCritical() << "failed to create partition on" << unallocated;
            return false;
        }
        const qint64 sectors = uefiSize / device->m_sectorSize;
        lastDeviceLen -= sectors;

        unallocated = device->partitions.last();
    }
    for(const QJsonValue& jsonValue : policyArray) {
        const QJsonObject& jsonObject = jsonValue.toObject();
        QString mountpoint = jsonObject["mountPoint"].toString();

        const QJsonArray& platform = jsonObject["platform"].toArray();
        if(!platform.contains("x86_64")) {
            continue;
        }
        const FSType fsType = findFSTypeByName(jsonObject["filesystem"].toString());
        qint64 partSize = 0;

        const QString& useRange = jsonObject["usage"].toString().toLower();
        if(useRange == "swap-size") {
            partSize = parsePartitionSize(QString("%1gib").arg(swapSize), lastDeviceLen * device->m_sectorSize);
        }

        if(partSize < 1) {
            partSize = parsePartitionSize(useRange, lastDeviceLen * device->m_sectorSize);

        }
        if(mountpoint == "linux-swap") {
            mountpoint = "";
        }
        const qint64 sectors = partSize / device->m_sectorSize;
        lastDeviceLen -= sectors;

        bool ok = false;
        if(device->m_partTableType == PartTableType::GPT || m_primaryPartitionLen < (device->m_maxPrims -1)) {
            ok = createPrimaryPartition(unallocated, PartitionType::Normal, true/*jsonObject["alignStart"].toBool*/,
                    fsType, mountpoint, sectors);

        } else {
            ok = createLogicalPartition(unallocated, true/*jsonObject["alignStart"].toBool*/,
                    fsType, mountpoint, sectors);
        }

        if(!ok) {
            qCritical() << "failed to create partition on ";// << unallocated;
            return false;
        }

        for(Partition::Ptr p : device->partitions) {
            if(p->m_type == PartitionType::Unallocated) {
                unallocated = p;
                break;
            }
        }
    }
    qDebug() << "operations for quick disk mode:" ;//<< m_operations;

    setBootLoaderPath(newDev->m_path);
    return true;
}

void FullPartitionDelegate::onCustomPartDone(const DeviceList &devices)
{
    qDebug() << "QuickPartitionDelegate::onManualPartDone()";// << devices;

    QString rootDisk = "";
    QString rootPath = "";
    QStringList mountPoints = {};
    bool foundSwap = false;
    QString espPath = "";
    Device::Ptr rootDevice;

    for(const Device::Ptr device : devices) {
        for(const Partition::Ptr part : device->partitions) {
            if(!part->m_mountPoint.isEmpty()) {
                const QString record(QString("%1=%2").arg(part->m_partitionPath).arg(part->m_mountPoint));
                mountPoints.append(record);
                if(part->m_mountPoint == "//") {
                    rootDisk = part->m_devPath;
                    rootPath = part->m_partitionPath;
                    rootDevice = device;
                }
            }
        }
    }

    for(Partition::Ptr part : rootDevice->partitions) {
        if(part->fs == FSType::LinuxSwap) {
            foundSwap = true;
            const QString record(QString("%1=swap").arg(part->m_partitionPath));
            mountPoints.append(record);
        }
    }
    if(!isMBRPreferred(m_realDevices)) {
        m_setting.uefiRequired = true;

        if(espPath.isEmpty()) {
            qCritical() << "esp path is empty!";
        }
        m_setting.rootDisk = rootDisk;
        m_setting.rootPartition = rootPath;
        m_setting.bootPartition = espPath;
        m_setting.mountPoints = mountPoints.join(';');

    } else {
        m_setting.uefiRequired = false;
        m_setting.rootDisk = rootDisk;
        m_setting.rootPartition = rootPath;
        m_setting.bootPartition = m_bootLoaderPath;
        m_setting.mountPoints = mountPoints.join(';');
    }

    bool useSwapFile = false;
    if(foundSwap) {
        useSwapFile = false;
    } else {
        useSwapFile = isSwapAreaNeeded();

    }
    m_setting.swapFileRequire = useSwapFile;
     writeDiskPartitionSetting(m_setting);

}

void FullPartitionDelegate::saveSwapSize()
{

}
bool FullPartitionDelegate::DiskSizeValidate(QString devpath)
{

    Device::Ptr dev = findDevice(devpath);
    if(dev != nullptr) {
        qint64 len = DISKMINSIZE * kGibiByte;
        if(dev->m_diskTotal >= len) {
            return true;
        } else
            return false;
    } else
        return false;


}

bool FullPartitionDelegate::DiskSizeLimited(QString devpath)
{
    Device::Ptr dev = findDevice(devpath);
    if(dev != nullptr) {
        qint64 len = DISKLIMITESIZE * kGibiByte;
        if(dev->m_diskTotal <= len) {
            return true;
        } else
            return false;
    } else
        return false;
}

uint FullPartitionDelegate::getSwapSize()
{
    struct sysinfo myinfo;
    unsigned long totalBytes;
    sysinfo(&myinfo);

    totalBytes = myinfo.mem_unit * myinfo.totalram;

    const double by = static_cast<double>(totalBytes) /
            static_cast<double>(kKibiByte) /
            static_cast<double>(kKibiByte) /
            static_cast<double>(kKibiByte);

    qDebug() << "system memory is:" << totalBytes << by;
    uint size = qRound(sqrt(by) + qRound(by));

    return std::min(static_cast<uint>(16), size);

}

bool FullPartitionDelegate::formatWholeDeviceMultipleDisk()
{/*
    resetOperations();
    m_selDevices.clear();
    if(selectedDisks().isEmpty()) {
        qWarning() << Q_FUNC_INFO << "select Disk is Empty";
        return false;
    }

    const QStringList& devPathList = m_selDisks;
    for(QString devPath : devPathList) {
        int devIndex = deviceIndex(m_virtualDevices, devPath);
        if(devIndex == -1) {
            qWarning() << Q_FUNC_INFO << "not find device:" << devPath;
            return false;
        }
    }
    PartTableType table;
    table = isEFIEnabled() ? PartTableType::GPT : PartTableType::MsDos;

    QuickDiskOption diskOption;
    QuickDiskPolicyList& policyList = diskOption.policyList;

    {
        const QByteArray& policyStr{getQuickDiskInstallPolicy()};
        if(policyStr.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "policy is empty";
            return false;
        }
        const QJsonArray policyArray = QJsonDocument::fromJson(policyStr).array();
        if(policyArray.isEmpty()) {
            return false;
        }
        for(const QJsonValue& jsonValue : policyArray) {
            const QJsonObject& jsonObject = jsonValue.toObject();
            const QJsonArray& platform = jsonObject["platform"].toArray();

            if(!platform.contains("x86")) {
                continue;
            }
            QuickDiskPolicy policy;

            policy.filesystem = findFSTypeByName(jsonObject["filesystem"].toString());
            policy.mountPoint = jsonObject["mountPoint"].toString();
            policy.label      = jsonObject["label"].toString();
            policy.usage      = jsonObject["usage"].toString();
            policy.alignStart = jsonObject["alignStart"].toBool();
            policy.device     = jsonObject["device"].toString();
            if (policy.mountPoint == "Linuxswap") {
                policy.mountPoint = "";
            }
            policyList.push_back(policy);
        }
    }
    for (int i = 0; i < policyList.length(); i++) {
        QuickDiskPolicy & policy = policyList[i];
        if (devPathList.length() > 1
            && policy.mountPoint == QString("/data")) {
            policy.device = devPathList.at(1);
        }
        else {
            policy.device = devPathList.at(0);
        }
    }
    for (const QString& devPath : devPathList) {
        int device_index = DeviceIndex(virtual_devices_, device_path);

        Device::Ptr device = virtual_devices_[device_index];
        device->partitions.clear();
        device->table = table;

        disk_option.is_system_disk = device->path == device_path_list.at(0);
        if (!formatWholeDeviceV2(device, disk_option)) {
            m_selDevices.clear();
            return false;
        }
        m_selDevices.append(device);
    }*/
    return true;

}




}
