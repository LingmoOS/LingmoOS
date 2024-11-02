#include "partition_delegate.h"

#include <QDebug>
#include <QString>
#include "customize_device_model.h"
#include "partman/partition_unit.h"
#include <QSet>


namespace KInstaller {
PartitionDelegate::PartitionDelegate(QObject* parent):QObject(parent)
{

}
PartitionDelegate::~PartitionDelegate()
{

}

bool PartitionDelegate::canAddLogical(const Partition::Ptr partition)
{
    const int index = deviceIndex(m_virtualDevices, partition->m_devPath);
    if(index == -1) {
        qCritical() << "getSupportedPartitionType() no device found at:" << partition->m_devPath;
        return false;
    }
    const Device::Ptr device = m_virtualDevices.at(index);

    if(device->m_partTableType == PartTableType::Empty) {
        return false;
    }

    if(device->m_partTableType == PartTableType::GPT) {
        return false;
    }

    bool logical_Ok = true;
    const int extendIndex = extendedPartitionIndex(device->partitions);

    if(extendIndex == -1) {
        if(getPrimaryPartitions(device->partitions).length() >= device->m_maxPrims) {
            logical_Ok = false;
        }
    } else {
        const Partition::Ptr extendPartition = device->partitions.at(extendIndex);
        const PartitionList primPartitions = getPrimaryPartitions(device->partitions);
        if(partition->m_sectorEnd < extendPartition->m_sectorStart) {
            for(Partition::Ptr primPartition : primPartitions) {
                if(primPartition->m_sectorEnd > partition->m_sectorStart &&
                        primPartition->m_sectorStart < extendPartition->m_sectorStart) {
                    logical_Ok = false;
                }
            }
        } else if(partition->m_sectorStart > extendPartition->m_sectorEnd) {
            for(Partition::Ptr primPartition : primPartitions) {
                if(primPartition->m_sectorEnd < partition->m_sectorStart &&
                        primPartition->m_sectorStart > extendPartition->m_sectorEnd) {
                    logical_Ok = false;
                }
            }
        }
    }
    return logical_Ok;
}

bool PartitionDelegate::canAddPrimary(const Partition::Ptr partition)
{
    const int index = deviceIndex(m_virtualDevices, partition->m_devPath);
    if(index == -1) {
        qCritical() << "getSupportedPartitionType() no device found at:" << partition->m_devPath;
        return false;
    }

    const Device::Ptr device = m_virtualDevices.at(index);

    if(device->m_partTableType == PartTableType::Empty) {
        return true;
    }
    const PartitionList primPartitions = getPrimaryPartitions(device->partitions);
    const PartitionList logicalPartitions = getLogicalPartitions(device->partitions);

    bool primary_OK = true;
    if(primPartitions.length() >= device->m_maxPrims) {
        primary_OK = false;
    } else {
        bool hasLogicalBefore = false;
        bool hasLogicalAfter = false;
        for(Partition::Ptr logicalPartition : logicalPartitions) {
            if(logicalPartition->m_sectorStart < partition->m_sectorStart) {
                hasLogicalBefore = true;
            }
            if(logicalPartition->m_sectorEnd > partition->m_sectorEnd) {
                hasLogicalAfter = true;
            }
        }
        if(hasLogicalAfter && hasLogicalBefore) {
            primary_OK = false;
        }
    }
    return primary_OK;
}

bool PartitionDelegate::isMBRPreferredDe() const
{
    return isMBRPreferred(m_realDevices);
}

bool PartitionDelegate::isPartitionTableMatchDe(const QString &devPath) const
{
    return isPartitionTableMatch(m_realDevices, devPath);
}

void PartitionDelegate::selectPartition(const Partition::Ptr partition)
{
    m_selPartition = partition;
}

bool PartitionDelegate::setBootFlag()
{
    bool foundBoot = false;

    for(OperationDisk& operation : m_operations) {
        if(operation.m_type == OperatorTYPE::NewPartitionTable)
            continue;
        if(operation.m_newPartition->fs == FSType::EFI) {
            operation.m_newPartition->flags.append(PartitionFlag::Boot);
            operation.m_newPartition->flags.append(PartitionFlag::ESP);
            foundBoot = true;
        }
    }

    for(Device::Ptr device : m_virtualDevices) {
        for(Partition::Ptr partition : device->partitions) {
            if(partition->fs == FSType::EFI) {
                return true;
            }
        }
    }

    if(!foundBoot) {
        for(OperationDisk& operation : m_operations) {
            if(operation.m_type == OperatorTYPE::NewPartitionTable)
                continue;
            if(operation.m_newPartition->m_mountPoint == "/") {
                operation.m_newPartition->flags.append(PartitionFlag::Boot);
                foundBoot = true;
            }
        }
    }

    return foundBoot;
}

void PartitionDelegate::addSelectDevice(Device::Ptr device)
{
    m_selDevices.append(device);
}

void PartitionDelegate::createDeviceTable(Device::Ptr& dev)
{
    Device::Ptr newDev(new Device(*dev));
    newDev->partitions.clear();
    newDev->m_partTableType = isEFIEnabled() ? PartTableType::GPT : PartTableType::MsDos;

    OperationDisk operation(newDev);

    m_operations.append(operation);
    operation.applyToShow(dev);
    emit deviceRefreshed(m_virtualDevices);
}

void PartitionDelegate::freshVirtualDeviceList()
{
    for(int i = 0; i < m_virtualDevices.length(); i++) {
        Device::Ptr dev = m_virtualDevices.at(i);
        int lognum = 5;
        for(int j = 0; j < dev->partitions.length(); j++) {
            if(dev->partitions.at(j)->m_type == PartitionType::Logical && dev->partitions.at(j)->m_partitionPath != "") {
                (m_virtualDevices.at(i))->partitions.at(j)->m_num = lognum;
                (m_virtualDevices.at(i))->partitions.at(j)->m_partitionPath = (m_virtualDevices.at(i))->partitions.at(j)->m_devPath + QString::number(lognum);
                lognum ++;
            }
        }
    }
    emit deviceRefreshed(m_virtualDevices);
}

void PartitionDelegate::resetOperations()
{
    m_operations.clear();
    m_virtualDevices = filterInstallerDevice(m_realDevices);
    for(Device::Ptr device : m_virtualDevices) {
        device->partitions = filterFragmentationPartition(device->partitions);
    }
    m_primaryPartitionLen = 0;

}


bool PartitionDelegate::createPartition(Partition::Ptr partition,
                                        PartitionType partitionType,
                                        bool alignStart, FSType fsType,
                                        QString& mountPoint,
                                        qint64 totalSectors,
                                        QString label,
                                        bool blformat)
 {
    Device::Ptr device = findDevice(partition->m_devPath);

    if(device == nullptr) {
        return false;
    }
//    if(device->m_partTableType == PartTableType::Empty) {
//        qWarning() << "partTableTypr is Empty!" ;
//        return false;
//    }
    if(device->m_partTableType == PartTableType::Empty) {
        createDeviceTable(device);
        if(m_operations.last().m_dev->m_partTableType == PartTableType::GPT) {
            partition->m_total -= 33;
            partition->m_sectorEnd -= 33;
        }
    }

    if(device->m_partTableType == PartTableType::MsDos) {
        if(getPrimaryPartitions(device->partitions).length() == 3) {
//            createPrimaryPartition(partition, PartitionType::Extended, alignStart, FSType::Extended, partition->m_mountPoint, partition->m_total,label);
            return createLogicalPartition(partition, alignStart, fsType, mountPoint, totalSectors, label, blformat);
        }
    }

    if(partitionType == PartitionType::Normal) {
        return createPrimaryPartition(partition, partitionType, alignStart, fsType, mountPoint, totalSectors,label, blformat);
    } else if(partitionType == PartitionType::Logical) {
        return createLogicalPartition(partition, alignStart, fsType, mountPoint, totalSectors, label, blformat);
    } else {
       // qCritical() << QString("not supported partition type:%1").arg(partitionType);
        return false;
    }
}

bool PartitionDelegate::createLogicalPartition(Partition::Ptr partition,
                                               bool alignStart,
                                               FSType fsType,
                                               QString &mountPoint,
                                               qint64 totalSectors,
                                               QString label,
                                               bool blformat)
{
   Device::Ptr device = findDevice(partition->m_devPath);

    if(device == nullptr) {
        return false;
    }
    const qint64 oneMebiByteSector = 1 * kMebiByte / partition->m_sectorSize;

    int extendIndex = extendedPartitionIndex(device->partitions);
    Partition::Ptr extendPartition(new Partition);

    if(extendIndex == -1) {
        qCritical() << "cannot create extended partition in simple mode";
        QString mountP = QString("");
        if(!createPrimaryPartition(partition, PartitionType::Extended, alignStart, FSType::Extended, mountP, totalSectors, label, false)) {
            qCritical() << "failed to create extended partition";
            return false;
        }

        extendPartition = m_operations.last().m_newPartition;
    } else {
        extendPartition = device->partitions.at(extendIndex);
        if(extendPartition->m_sectorStart > partition->m_sectorStart ||
                extendPartition->m_sectorEnd < partition->m_sectorEnd) {
            Partition::Ptr newExtendPartition(new Partition(*extendPartition));
            newExtendPartition->m_sectorStart = qMin(extendPartition->m_sectorStart, partition->m_sectorStart);
            newExtendPartition->m_sectorEnd = qMin(extendPartition->m_sectorEnd, partition->m_sectorEnd);

            alignPartition(newExtendPartition);

            OperationDisk resizeExtendOpt(OperatorTYPE::Resize, extendPartition, newExtendPartition);
            resizeExtendOpt.m_dev = device;
            extendPartition = newExtendPartition;
            m_operations.append(resizeExtendOpt);
            resizeExtendOpt.applyToShow(device);
        }
    }

    Partition::Ptr newPartition(new Partition);
    newPartition->m_devPath = partition->m_devPath;
    newPartition->m_partitionPath = partition->m_partitionPath;
    newPartition->m_sectorSize = partition->m_sectorSize;
    newPartition->m_status = PartitionStatus::New;
    newPartition->m_type = PartitionType::Logical;
    newPartition->fs = fsType;
    newPartition->m_mountPoint = mountPoint;
    newPartition->m_FSlabel = label;
    newPartition->blFormatPartition = blformat;

    const int partitionNum = allocLogicalPartitionNumber(device);
    if(partitionNum < 0) {
        qCritical() << "failed to allocate logical part number1";
        return false;
    }
    newPartition->changeNumber(partitionNum);

    if(fsType == FSType::Recovery) {
        newPartition->flags << PartitionFlag::Hidden;
        newPartition->m_FSlabel = "backup";
        newPartition->m_mountPoint = "/backup";

    }

    if(alignStart) {
        const qint64 startSector = qMax(partition->m_sectorStart, extendPartition->m_sectorStart);
        newPartition->m_sectorStart = startSector ;

        const qint64 endSector = qMin(partition->m_sectorEnd, extendPartition->m_sectorEnd);
        newPartition->m_sectorEnd = qMin(endSector, totalSectors + newPartition->m_sectorStart + oneMebiByteSector);

    } else {
        newPartition->m_sectorEnd = qMin(partition->m_sectorEnd, extendPartition->m_sectorEnd) - oneMebiByteSector;
        const qint64 startSector = qMax(partition->m_sectorStart, extendPartition->m_sectorStart);
        newPartition->m_sectorStart = qMax(startSector, partition->m_sectorEnd - totalSectors) ;
    }
    newPartition->m_sectorStart += 1;

    alignPartition(newPartition);

    if(newPartition->m_sectorStart < partition->m_sectorStart ||
            newPartition->m_sectorStart >= partition->m_sectorEnd ||
            newPartition->getByteLength() < kMebiByte ||
            newPartition->m_sectorEnd > partition->m_sectorEnd) {
        qCritical() << "Invalid partition sector rangr";
        return false;
    }
    newPartition->getByteLength();
    newPartition->m_freespace = newPartition->getByteLength();


    resetOperationMountPoint(mountPoint);

    OperationDisk operation(OperatorTYPE::Create, partition, newPartition);

    operation.m_dev = device;
    m_operations.append(operation);
    operation.applyToShow(device);


    return true;

}

bool PartitionDelegate::createPrimaryPartition(Partition::Ptr partition,
                                               PartitionType partitionType,
                                               bool alignStart,
                                               FSType fsType,
                                               QString &mountPoint,
                                               qint64 totalSectors,
                                               QString label,
                                               bool blformat)
{
    if(partitionType != PartitionType::Normal &&
            partitionType != PartitionType::Extended) {
        //qCritical() << "CreatePrimaryPartition() invalid part type:" << partitionType;
        return false;
    }
    Device::Ptr device = findDevice(partition->m_devPath);

    if(device == nullptr) {
        return false;
    }

    const qint64 oneMebiByteSector = 1 * kMebiByte / partition->m_sectorSize;

    const int extendIndex = extendedPartitionIndex(device->partitions);
    if(partitionType == PartitionType::Normal && extendIndex > -1) {
        const Partition::Ptr extendPartition = device->partitions.at(extendIndex);
        const PartitionList logicalPartitions = getLogicalPartitions(device->partitions);

        if(logicalPartitions.isEmpty()) {
            Partition::Ptr unallocatedPartition(new Partition);
            unallocatedPartition->m_devPath = extendPartition->m_devPath;
            unallocatedPartition->m_partitionPath = extendPartition->m_partitionPath;
            unallocatedPartition->m_sectorStart = extendPartition->m_sectorStart;
            unallocatedPartition->m_sectorEnd = extendPartition->m_sectorEnd;
            unallocatedPartition->m_sectorSize = extendPartition->m_sectorSize;
            unallocatedPartition->m_type = PartitionType::Extended;
            unallocatedPartition->blFormatPartition = false;

            OperationDisk operation(OperatorTYPE::Delete, extendPartition, unallocatedPartition);
            m_operations.append(operation);
            operation.applyToShow(device);

            device->partitions.removeAt(extendIndex);
        } else if(isPartitionsJoint(extendPartition, partition)) {
            Partition::Ptr newExtendPartition(new Partition(*extendPartition));
            newExtendPartition->m_sectorStart = logicalPartitions.first()->m_sectorStart - oneMebiByteSector;
            newExtendPartition->m_sectorEnd = logicalPartitions.last()->m_sectorEnd;

            if(isPartitionsJoint(newExtendPartition, partition)) {
                qCritical() << "failed to shrink extended partition!";
                return false;
            }
            OperationDisk operation(OperatorTYPE::Resize, extendPartition, newExtendPartition);
            m_operations.append(operation);
            operation.applyToShow(device);
        }

    }
    Partition::Ptr newPartition(new Partition);
    newPartition->m_devPath = partition->m_devPath;
    newPartition->m_partitionPath = partition->m_partitionPath;
    newPartition->m_sectorSize = partition->m_sectorSize;
    newPartition->m_status = PartitionStatus::New;
    newPartition->m_type = partitionType;
    newPartition->blFormatPartition = blformat;
    newPartition->fs = fsType;
    newPartition->m_mountPoint = mountPoint;
    newPartition->m_FSlabel = label;

    int partitionNum = allocPrimaryPartitionNumber(device);
    if(partitionNum < 0) {
        qCritical() << "failed to allocate primary partition number!";
        return false;
    }
    newPartition->changeNumber(partitionNum);

    if(fsType == FSType::Recovery) {
        newPartition->flags << PartitionFlag::Hidden;
        newPartition->m_FSlabel = "backup";
        newPartition->m_mountPoint = "/backup";
    }
    const bool needMBR = (partition->m_sectorStart <= oneMebiByteSector);

    if(alignStart) {
        if(needMBR) {
            newPartition->m_sectorStart = oneMebiByteSector;
        } else {
            newPartition->m_sectorStart = partition->m_sectorStart;
        }
        newPartition->m_sectorEnd = qMin(partition->m_sectorEnd, totalSectors + newPartition->m_sectorStart);
    } else {
        newPartition->m_sectorEnd = partition->m_sectorEnd;
        if(needMBR) {
            newPartition->m_sectorStart = qMax(oneMebiByteSector, partition->m_sectorEnd - totalSectors);
        } else {
            newPartition->m_sectorStart = qMax(partition->m_sectorStart, partition->m_sectorEnd - totalSectors);
        }
    }
    alignPartition(newPartition);
    if(newPartition->m_sectorStart < partition->m_sectorStart ||
            newPartition->m_sectorStart >= partition->m_sectorEnd ||
            newPartition->getByteLength() < kMebiByte ||
            newPartition->m_sectorEnd > partition->m_sectorEnd) {
        qCritical() << "invalid partition sector range" << "newPartition:" << newPartition << "partition:" << partition;
        return false;
    }

    if(newPartition->m_type == PartitionType::Extended) {
        newPartition->m_sectorStart = partition->m_sectorStart /*+ oneMebiByteSector*/;
        newPartition->m_sectorEnd = partition->m_sectorEnd;
        newPartition->fs = FSType::Extended;
        newPartition->m_name = "extend";
    }

    newPartition->getByteLength();
    newPartition->m_freespace = newPartition->getByteLength();

    resetOperationMountPoint(mountPoint);
    OperationDisk operation(OperatorTYPE::Create, partition, newPartition);
    operation.m_dev = device;
    m_operations.append(operation);
    operation.applyToShow(device);

    m_primaryPartitionLen++;
    return true;
}

void PartitionDelegate::deletePartition(const Partition::Ptr partition)
{

    Partition::Ptr newPartition(new Partition(*partition));
    newPartition->m_num = -1;
    newPartition->m_partitionPath = "";
    newPartition->m_devPath = partition->m_devPath;
    newPartition->m_sectorSize = partition->m_sectorSize;
    newPartition->m_sectorStart = partition->m_sectorStart;
    newPartition->m_sectorEnd = partition->m_sectorEnd;
    newPartition->m_type = PartitionType::Unallocated;
    newPartition->fs = FSType::Empty;
    newPartition->m_status = PartitionStatus::Delete;
    newPartition->m_mountPoint = "";

    if(partition->m_type == PartitionType::Logical) {
        const qint64 oneMebiByteSector = 1 * kMebiByte / partition->m_sectorSize;
        newPartition->m_sectorStart -= oneMebiByteSector;
    }
    Device::Ptr device = findDevice(partition->m_devPath);

    if(device == nullptr) {
        return;
    }

    if(partition->m_type == PartitionType::Extended) {
        PartitionList& partitions = device->partitions;

        Partition::Ptr extendPartition = partitions.at(extendedPartitionIndex(partitions));
        const PartitionList logicalPartitions = getLogicalPartitions(partitions);
        for(Partition::Ptr part : logicalPartitions) {
            deletePartition(part);
        }
    }

//#ifdef QT_DEBUG
//    if(partition->m_status == PartitionStatus::New) {
//        for(int index = m_operations.length() - 1; index >= 0; index--) {
//            OperationDisk operation = m_operations.at(index);
//            if(operation.m_type == OperatorTYPE::Create &&
//                    operation.m_newPartition.data() == partition.data()) {
//                partition->m_type = PartitionType::Unallocated;
//                partition->fs = FSType::Empty;
//                partition->m_status = PartitionStatus::Delete;
//                partition->m_mountPoint = "";

//                const qint64 startSize = operation.m_origPartition->m_sectorStart;
//                m_operations.removeAt(index);

//                const qint64 endSize = partition->m_sectorEnd +1;
//                for(auto it = m_operations.begin(); it != m_operations.end(); it++) {
//                    if(it->m_type == OperatorTYPE::Create &&
//                            partition->m_devPath == it->m_origPartition->m_devPath &&
//                            it->m_origPartition->m_sectorStart == endSize) {
//                        it->m_origPartition->m_sectorStart = startSize;
//                    }
//                }
//                break;
//            }
//        }
//    } else {
//#endif
    OperationDisk operation(OperatorTYPE::Delete, partition, newPartition);
    m_operations.append(operation);
    operation.m_dev = device;
    operation.applyToShow(device);
    freshVirtualDeviceList();
    qDebug() << "add delete operation " ;//<< *newPartition.data();

//#ifdef QT_DEBUG
//    }
//#endif
//    if(partition->m_type == PartitionType::Logical) {
//        PartitionList& partitions = device->partitions;

//        Partition::Ptr extendPartition = partitions.at(extendedPartitionIndex(partitions));
//        const PartitionList logicalPartitions = getLogicalPartitions(partitions);

//        if(!extendPartition.isNull()) {
//            if((logicalPartitions.length() == 1 && logicalPartitions.at(0) == partition) ||
//                    (logicalPartitions.length() == 0)) {
//                Partition::Ptr unallocatePartition(new Partition);
//                unallocatePartition->m_devPath = extendPartition->m_devPath;
//                unallocatePartition->m_sectorStart = extendPartition->m_sectorStart;
//                unallocatePartition->m_sectorEnd = extendPartition->m_sectorEnd;
//                unallocatePartition->m_sectorSize = extendPartition->m_sectorSize;
//                unallocatePartition->m_type = PartitionType::Unallocated;
//                unallocatePartition->blFormatPartition = false;
//                OperationDisk operation(OperatorTYPE::Delete, extendPartition, unallocatePartition);
//                m_operations.append(operation);
//                operation.applyToShow(device);
//            } else {
//                qint64 extendStartSector = -1;
//                qint64 extendEndSector = -1;

//                if(reCalculateExtPartBoundry(partitions, PartitionAction::RemeveLogicalPartition,
//                                             extendPartition, extendStartSector, extendEndSector)) {
//                    Partition::Ptr newExtendPartition(new Partition(*extendPartition));
//                    newExtendPartition->m_sectorStart = extendStartSector;
//                    newExtendPartition->m_sectorEnd = extendEndSector;

//                    OperationDisk operation(OperatorTYPE::Resize, extendPartition, newExtendPartition);
//                    m_operations.append(operation);
//                    operation.applyToShow(device);

//                }
//            }
//        }
//    }

}

void PartitionDelegate::formatPartition(const Partition::Ptr partition, FSType fsType, QString &mountPoint, bool blformat)
{
    qDebug() << "formatPartition()";// << partition << fsType << mountPoint;

    resetOperationMountPoint(mountPoint);

    if(partition->m_status == PartitionStatus::New ||
            partition->m_status == PartitionStatus::Format) {
        for(int index = m_operations.length() - 1; index >= 0; index--) {
            OperationDisk& operation = m_operations[index];
            if((operation.m_newPartition->m_partitionPath == partition->m_partitionPath) &&
                    (operation.m_type == OperatorTYPE::Format ||
                     operation.m_type == OperatorTYPE::Create)) {
                operation.m_newPartition->m_mountPoint = mountPoint;
                operation.m_newPartition->fs = fsType;
//                operation.applyToShow(findDevice(partition->m_devPath));
                return;
            }
        }
    }
    Partition::Ptr newPartition(new Partition);
    newPartition->m_sectorSize = partition->m_sectorSize;
    newPartition->m_partitionPath = partition->m_partitionPath;
    newPartition->m_sectorStart = partition->m_sectorStart;
    newPartition->m_sectorEnd = partition->m_sectorEnd;
    newPartition->m_partitionPath = partition->m_partitionPath;
    newPartition->m_devPath = partition->m_devPath;
    newPartition->m_num = partition->m_num;
    newPartition->fs = fsType;
    newPartition->m_type = partition->m_type;
    newPartition->m_mountPoint = mountPoint;
    newPartition->blFormatPartition = blformat;


    if(partition->m_status == PartitionStatus::Real) {
        newPartition->m_status = PartitionStatus::Format;
    } else if(partition->m_status == PartitionStatus::New ||
              partition->m_status == PartitionStatus::Format) {
        newPartition->m_status = partition->m_status;
    }

    if(fsType == FSType::Recovery) {
        newPartition->flags << PartitionFlag::Hidden;
        newPartition->m_FSlabel = "backup";
        newPartition->m_mountPoint = "/recovery";
    }

    Device::Ptr device = findDevice(partition->m_devPath);
    if(device.isNull()) {
        return;
    }

    OperationDisk operation(OperatorTYPE::Format, partition, newPartition);
    m_operations.append(operation);
    operation.applyToShow(device);
}

void PartitionDelegate::onDeviceRefreshed( DeviceList devices)
{
    qDebug() << Q_FUNC_INFO << "0" ;
    m_realDevices = getDeviceListByDeviceModel(devices);
    m_operations.clear();
    m_virtualDevices = filterInstallerDevice(m_realDevices);

    for(Device::Ptr device : m_virtualDevices) {
        device->partitions = filterFragmentationPartition(device->partitions);
    }
    emit deviceRefreshed(m_virtualDevices);
    qDebug() << Q_FUNC_INFO << "1";
}

void PartitionDelegate::setBootLoaderPath(const QString &path)
{
    m_bootLoaderPath = path;
}


void PartitionDelegate::refreshShow()
{
//    m_virtualDevices = filterInstallerDevice(m_realDevices);

    for(Device::Ptr device : m_virtualDevices) {
        device->partitions = filterFragmentationPartition(device->partitions);
    }

    for(Device::Ptr device : m_virtualDevices) {
        mergeAllUnallocatesPartitions(device->partitions);

//        for(OperationDisk& operation : m_operations) {
//            if((operation.m_type == OperatorTYPE::NewPartitionTable &&
//                operation.m_dev.data() == device.data()) ||
//                    (operation.m_type != OperatorTYPE::NewPartitionTable &&
//                     operation.m_origPartition->m_devPath == device->m_path)) {
//                operation.applyToShow(device);
//            }
//        }

        mergeAllUnallocatesPartitions(device->partitions);
    }

    qDebug() << "devices:" ;//<< m_virtualDevices;
    qDebug() << "operation:";// << m_operations;
    emit deviceRefreshed(m_virtualDevices);

}

Partition::Ptr PartitionDelegate::getRealPartition(Partition::Ptr virtualPart) const
{
    const int index = deviceIndex(m_realDevices, virtualPart->m_devPath);

    if(index == -1) {
        qWarning() << "failed to find device:" << virtualPart->m_devPath;
        return Partition::Ptr();
    }

    for(Partition::Ptr partition : m_realDevices.at(index)->partitions) {
        if(partition->m_type == PartitionType::Extended) {
            continue;
        }
        if((partition->m_sectorStart <= virtualPart->m_sectorStart) &&
                (partition->m_sectorEnd >= virtualPart->m_sectorEnd)) {
            return partition;
        }
    }
    qWarning() << "failed to find partition at :" << virtualPart;
    return Partition::Ptr();

}

void PartitionDelegate::resetOperationMountPoint(const QString mountPoint)
{
    qDebug() << Q_FUNC_INFO << mountPoint;

    for(auto it = m_operations.begin(); it != m_operations.end(); it++) {
        OperationDisk& operation = *it;
        if(operation.m_type == OperatorTYPE::NewPartitionTable) {
            continue;
        }

        if(operation.m_newPartition->m_mountPoint == mountPoint) {
            if(operation.m_type == OperatorTYPE::MountPoint) {
                it = m_operations.erase(it);//remove
                return;
            } else {
                operation.m_newPartition->m_mountPoint = "";
                qDebug() << "Clear mountPoint of operation:" ;//<< operation;
            }
        }
    }
}


void PartitionDelegate::updateMountPoint(const Partition::Ptr partition, const QString mountPoint)
{
    resetOperationMountPoint(mountPoint);
    qDebug() << Q_FUNC_INFO ;//<< partition->m_partitionPath << mountPoint;
    if(!mountPoint.isEmpty()) {
        Partition::Ptr newPartition(new Partition(*partition));
        newPartition->m_mountPoint = mountPoint;
        Device::Ptr device = findDevice(partition->m_devPath);

        if(device.isNull()) {
            return;
        }
        OperationDisk operation(OperatorTYPE::MountPoint, partition, newPartition);
        m_operations.append(operation);

        operation.applyToShow(device);
    }
}
bool PartitionDelegate::reCalculateExtPartBoundry(PartitionList &partitions,
                                                  PartitionAction action,
                                                  Partition::Ptr curPartition,
                                                  qint64 &startSector,
                                                  qint64 &endSector)
{
    if(partitions.length() == 0) {
        return false;
    }

    bool startFound = false;
    bool endFound = false;

    for(const Partition::Ptr& p : partitions) {
        if(p->m_type != PartitionType::Logical) {
            continue;
        }
        if(p == curPartition) {
            if(action == PartitionAction::RemeveLogicalPartition) {
                continue;
            }
        }
        if(!startFound || startSector > p->m_sectorStart) {
            startFound = true;
            const qint64 oneMebiByteSector = 1 * kMebiByte / p->m_sectorStart;
            startSector = p->m_sectorStart - oneMebiByteSector;
        }

        if(!endFound || endSector < p->m_sectorEnd) {
            endFound = true;
            endSector = p->m_sectorEnd;
        }
    }
    return startFound && endFound;
}

bool PartitionDelegate::reCalculateExtPartBoundry(PartitionAction action,
                                                  Partition::Ptr curPartition,
                                                  qint64 &startSector,
                                                  qint64 &endSector)
{
    const int devIndex = deviceIndex(virtualDevices(), curPartition->m_devPath);
    if(devIndex == -1) {
        return false;
    }

    Device::Ptr device = virtualDevices()[devIndex];
    PartitionList& partitions = device->partitions;

    return false; //reCalculateExtPartBoundry(partitions, action, curPartition, startSector, endSector);
}


Device::Ptr PartitionDelegate::findDevice(const QString &devPath)
{
    const int devIndex = deviceIndex(m_virtualDevices, devPath);
    if(devIndex == -1) {
        qCritical() << "createPartition() device index out of range:" << devPath;
        return nullptr;
    }
    Device::Ptr device = m_virtualDevices[devIndex];
    return device;
}

ValidateStates PartitionDelegate::validate() const
{
    const Partition::Ptr root_partition = m_selPartition;
    ValidateStates states = {};
    const int devIndex = deviceIndex(m_virtualDevices, root_partition->m_partitionPath);
    if(devIndex == -1) {
        qCritical() << Q_FUNC_INFO << root_partition->m_partitionPath;
        states << ValidateState::RootMissing;
    }
    const Device::Ptr device = m_virtualDevices.at(devIndex);

    if(device->m_partTableType == PartTableType::Empty) {
        states << ValidateState::OK;
    }

    if(root_partition->m_partitionPath.isEmpty()) {
        states << ValidateState::RootMissing;
    }

//    if((root_partition->m_type == PartitionType::Unallocated) && !canAddPrimary(root_partition)
//            && !canAddLogical(root_partition)) {
//        states << ValidateState::Max
//    }

    //TODO
    //是否需要设置根分区,从配置文件中获取
    const int root_required = true;
    const qint64 root_min_bytes = root_required * kGibiByte;
    const qint64 root_real_bytes = root_partition->getByteLength() + kMebiByte;

    if(root_real_bytes < root_min_bytes) {
        states << ValidateState::RootTooSmall;
    }

    if(states.isEmpty()) {
        states << ValidateState::OK;
    }
    return (ValidateStates)states;
}

void PartitionDelegate::removeRepetition()
{
    auto operator_ = operations();



}

}
