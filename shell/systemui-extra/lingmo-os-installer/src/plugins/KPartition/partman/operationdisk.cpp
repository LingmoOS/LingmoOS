#include "operationdisk.h"
#include "libparted_unit.h"
#include "partition_format.h"
#include <QDir>
#include <QFileInfoList>
#include "datastruct.h"
#include "filesystem.h"
#include <QDebug>
#include "../PluginService/kcommand.h"
namespace KInstaller{
namespace Partman {
OperationDisk::OperationDisk(Device::Ptr dev):
    m_dev(dev),
    m_type(OperatorTYPE::NewPartitionTable)
{

}

OperationDisk::OperationDisk(OperatorTYPE type, Partition::Ptr origPartition, Partition::Ptr newPartition):
      m_origPartition(origPartition),
      m_newPartition(newPartition),
      m_type(type)
{

}

OperationDisk::~OperationDisk()
{

}
void OperationDisk::umount(Partition::Ptr part)
{
    if(part->fs == FSType::Lvm2_PV) {
        QStringList paths(part->m_partitionPath);
        QString output="", error="";
        int excode =-1;
        KServer::KCommand::getInstance()->RunScripCommand("pvdisplay", part->m_partitionPath, output, error, excode);
        QStringList msglist = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        if(msglist.size() > 0) {
            for(QString msg : msglist) {
                int index = msg.indexOf("VG Name");
                if(index > -1) {
                    msg.replace("VG name", "");
                    msg.replace("\n", "");
                    msg.replace(" ", "");
                    if(msg.size() > 0) {
                        QDir dir("/dev" + msg);
                        QFileInfoList filelist = dir.entryInfoList();
                        for(QFileInfo fileinfo : filelist) {
                            if(fileinfo.fileName() == "." || fileinfo.fileName() == ".") continue;
                            KServer::KCommand::getInstance()->RunScripCommand("umount", QStringList(fileinfo.absoluteFilePath()));
                            KServer::KCommand::getInstance()->RunScripCommand("lvremove", QStringList({fileinfo.absoluteFilePath(), "-y"}));
                        }
                        KServer::KCommand::getInstance()->RunScripCommand("vgremove", QStringList({"/dev/" + msg, "-y"}));
                    }
                }
            }
        }
    }
    KServer::KCommand::getInstance()->RunScripCommand("umount", QStringList(part->m_partitionPath));

}

void OperationDisk::umount(Device::Ptr dev)
{
    if(dev) {
        for(Partition::Ptr part : dev->partitions) {
            umount(part);
        }
    }
}

bool OperationDisk::applyToDisk()
{
    switch (m_type) {
    qDebug() << "applyToDisk:" << m_newPartition->m_partitionPath;
    case OperatorTYPE::Create: {
        if(m_newPartition->fs == FSType::Empty) {
            qCritical() << "创建分区的文件系统是未知的";
            return false;
        }
        umount(m_origPartition);
        bool createbl = false;
        createbl = createPartition(m_newPartition);
        if(!createbl) {
            qCritical() << "创建分区失败";
            return false;
        }
        if(!upatePartitionNumber(m_newPartition)) {
            qCritical() << "新分区编号获取失败" << m_newPartition->m_partitionPath;
            return false;
        }

        if(createbl && m_newPartition->m_type != PartitionType::Extended && m_newPartition->fs != FSType::Empty) {
            KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
            if(!MKfs(m_newPartition)) {
                if(!MKfs(m_newPartition)){
                    qCritical() << "创建分区格式化失败" << m_newPartition->m_partitionPath;
                    return false;
                }
            }
        }
        if(m_newPartition->fs == FSType::EFI) {
//            m_newPartition->flags.append(PartitionFlag::Boot);
            m_newPartition->flags.append(PartitionFlag::ESP);
            setPartitionFlags(m_newPartition);
        }
        if(m_newPartition->fs == FSType::LinuxSwap) {
            m_newPartition->flags.append(PartitionFlag::Swap);
            setPartitionFlags(m_newPartition);
        }
        return true;
    }

    case OperatorTYPE::Delete:
        umount(m_origPartition);
        if(!DeletePartition(m_origPartition)) {
             qCritical() << "删除分区操作失败" << m_origPartition->m_partitionPath;
             return false;
        } else {
            return true;
        }

    case OperatorTYPE::Format:
        if(m_newPartition->fs == FSType::Empty) {
            qCritical() << "partition unsed" << m_newPartition->m_partitionPath;
            return false;
        }
        umount(m_origPartition);
        if(!resetPartitionFSType(m_newPartition)) {
            qCritical() << "设置文件系统失败" << m_newPartition->m_partitionPath;
            return false;
        }

        if(!upatePartitionNumber(m_newPartition)) {
            qCritical() << "新分区编号获取失败" << m_newPartition->m_partitionPath;
            return false;
        }

        if(!MKfs(m_newPartition)) {
            qCritical() << "创建分区格式化失败" << m_newPartition->m_partitionPath;
            return false;
        }
        if(!setPartitionFlags(m_newPartition)) {
            qCritical() << "格式化分区标志设置失败" << m_newPartition->m_partitionPath;
            return false;
        }
        return true;

    case OperatorTYPE::Invalid:
        //qCritical() << "无效操作!";
        return false;

    case OperatorTYPE::MountPoint:
        if(!setPartitionFlags(m_newPartition)) {
            qCritical() << "格式化分区标志设置失败"<< m_newPartition->m_partitionPath;
            return false;
        } else {
            return true;
        }

    case OperatorTYPE::NewPartitionTable:
        if(!createPartitionTable(m_dev->m_path, m_dev->m_partTableType)) {
            qCritical() << "新建分区表失败" << m_newPartition->m_partitionPath;
            return false;
        } else {
            return true;
        }

    case OperatorTYPE::Resize:
        umount(m_origPartition);
        if(!resizePartition(m_newPartition)) {
            qCritical() << "调整分区大小失败" << m_newPartition->m_partitionPath;
            return false;
        } else {
            return true;
        }

    default:
        qCritical() << "未知的操作类型";
        return false;
    }
}

QString OperationDisk::OperationDiskDescription()
{
    QString desc = "";
    switch (m_type) {
    case OperatorTYPE::Create:
        if(m_newPartition->m_type == PartitionType::Extended) {
            desc = QObject::tr("Extended partition %1 has \n").arg(m_newPartition->m_partitionPath);
        } else if(m_newPartition->m_mountPoint.isEmpty()) {
            desc = QObject::tr("Create new partition %1,%2\n").arg(m_newPartition->m_partitionPath)
                    .arg(getPedFsTypeName(m_newPartition));
        } else {
            desc = QObject::tr("Create new partition %1,%2,%3\n").arg(m_newPartition->m_partitionPath)
                    .arg(m_newPartition->m_mountPoint).arg(getPedFsTypeName(m_newPartition));
        }
        break;
    case OperatorTYPE::Delete:
        desc = QObject::tr("Delete partition %1\n").arg(m_newPartition->m_partitionPath);
        break;
    case OperatorTYPE::Format:
        if(m_newPartition->m_mountPoint.isEmpty()) {
            desc = QObject::tr("Format %1 partition, %2\n").arg(m_newPartition->m_partitionPath)
                    .arg(getPedFsTypeName(m_newPartition));
        } else {
            desc = QObject::tr("Format partition %1,%2,%3\n").arg(m_newPartition->m_partitionPath)
                    .arg(m_newPartition->m_mountPoint).arg(getPedFsTypeName(m_newPartition));
        }
        break;
    case OperatorTYPE::MountPoint:
        desc = QObject::tr("%1 partition mountPoint %2\n").arg(m_newPartition->m_partitionPath)
                .arg(m_newPartition->m_mountPoint);
        break;
    case OperatorTYPE::NewPartitionTable:
        desc = QObject::tr("New Partition Table %1\n").arg(m_dev->m_path);
        break;
    case OperatorTYPE::Resize:
        desc = QObject::tr("Reset size %1 partition\n").arg(m_newPartition->m_partitionPath);
        break;
    default:
        break;
    }
    return (QString)desc;
}

void OperationDisk::applyCreateShow(PartitionList &partitions)
{
    qDebug() << "applyCreateShow(),partition:" << partitions;
    qDebug() << "m_origPartition:" << m_origPartition;
    qDebug() << "m_newPartition:" << m_newPartition;

    int index = partitionIndex(partitions, m_origPartition);
    if(index == -1) {
      //  qCritical() << "applyCreateShow() 没有找到需要提交的分区";// << m_origPartition;
        return;
    }
    if(m_newPartition->m_type == PartitionType::Extended) {
        partitions.insert(index, m_newPartition);
        return;
    } else {
        partitions[index] = m_newPartition;
    }
    const qint64 oneMebiByteSector = 1 * kMebiByte / m_origPartition->m_sectorSize;
    const qint64 twoMebiByteSector = 2 * kMebiByte / m_origPartition->m_sectorSize;

    if(m_newPartition->m_sectorStart - m_origPartition->m_sectorStart > twoMebiByteSector) {
        Partition::Ptr unallocated(new Partition);
        unallocated->m_devPath = m_origPartition->m_devPath;
        unallocated->m_sectorSize = m_origPartition->m_sectorSize;
        unallocated->m_type = m_origPartition->m_type;
        unallocated->m_sectorStart = m_origPartition->m_sectorStart + 1;
        unallocated->m_sectorEnd = m_newPartition->m_sectorStart - 1;
        unallocated->m_num = m_origPartition->m_num;
        unallocated->m_partitionPath = m_origPartition->m_partitionPath;

        if(m_newPartition->m_type == PartitionType::Logical) {
            unallocated->m_sectorEnd -= oneMebiByteSector;
        }
        partitions.insert(index, unallocated);
        index += 1;
    }

    if(m_origPartition->m_sectorEnd - m_newPartition->m_sectorEnd >twoMebiByteSector) {
        Partition::Ptr unallocated(new Partition);
        unallocated->m_devPath = m_origPartition->m_devPath;
        unallocated->m_sectorSize = m_origPartition->m_sectorSize;
        unallocated->m_type = m_origPartition->m_type;
        unallocated->m_sectorStart = m_newPartition->m_sectorEnd + 1;
        unallocated->m_sectorEnd = m_origPartition->m_sectorEnd - 1;
        unallocated->m_num = m_origPartition->m_num;
        unallocated->m_partitionPath = m_origPartition->m_partitionPath;
        if(index + 1 == partitions.length()) {
            partitions.append(unallocated);
        } else {
            partitions.insert(index + 1, unallocated);
        }
    }
    mergeAllUnallocatesPartitions(partitions);
}

void OperationDisk::applyDeleteShow(PartitionList &partitions)
{
   // qDebug() << Q_FUNC_INFO << m_origPartition << m_newPartition;

    int index = partitionIndex(partitions, m_origPartition);
    if(index >= 0) {
        if(partitions[index]->m_type == PartitionType::Extended) {
            partitions.removeAt(index);
            return;
        }
    }
    substitutePartition(partitions);
    mergeAllUnallocatesPartitions(partitions);
}

void OperationDisk::substitutePartition(PartitionList &partitions)
{
    const int index = partitionIndex(partitions, m_origPartition);
    if(index == -1) {
      //  qCritical() << "substitutePartition()查找需要替代的分区失败" ;//<< m_origPartition->m_devPath;
    } else {
        partitions[index] = m_newPartition;
    }
}

void OperationDisk::applyResizeShow(PartitionList &partitions)
{
    if(m_newPartition->m_type == PartitionType::Extended) {
        substitutePartition(partitions);
    }
}

void OperationDisk::applyNewTableShow(Device::Ptr dev)
{
    dev->m_partTableType = m_dev->m_partTableType;
    dev->partitions.clear();
    Partition::Ptr freePart(new Partition);
    freePart->m_devPath = dev->m_path;
    freePart->m_partitionPath = "";
    freePart->m_num = -1;
    freePart->m_sectorStart = 1;
    freePart->m_sectorEnd = dev->m_length - (dev->m_partTableType == PartTableType::GPT ? 33 : 0);
    freePart->m_sectorSize = dev->m_sectorSize;
    freePart->m_type = PartitionType::Unallocated;

    dev->partitions.append(freePart);

    if(dev->m_partTableType == PartTableType::MsDos) {
        dev->m_maxPrims = MsDosPartitionTableMaxPrims;
    } else if(dev->m_partTableType == PartTableType::GPT) {
        dev->m_maxPrims = GPTPartitionTableMaxPrims;
    }

}

void OperationDisk::applyToShow(Device::Ptr dev)
{
    PartitionList& partitions = dev->partitions;
    switch (m_type) {
    case OperatorTYPE::Create:
        applyCreateShow(partitions);
        break;
    case OperatorTYPE::Delete:
        applyDeleteShow(partitions);
        break;
    case OperatorTYPE::Format:
        substitutePartition(partitions);
        break;
    case OperatorTYPE::MountPoint:
        substitutePartition(partitions);
        break;
    case OperatorTYPE::Resize:
        applyResizeShow(partitions);
        break;
    case OperatorTYPE::NewPartitionTable:
        applyNewTableShow(dev);
        break;
    default:
        break;
    }
}
void mergeAllOperation(OperationDiskList &operations, OperationDisk &operate)
{
    Q_UNUSED(operations);
    Q_UNUSED(operate);
}

void mergeAllUnallocatesPartitions(PartitionList &partitions)
{
    if(partitions.isEmpty()) {
        return;
    }
    int global_index = 0;
    while (global_index < partitions.length()) {
        int index;
        for(index = global_index; index < partitions.length(); index++) {
            if(partitions.at(index)->m_type == PartitionType::Unallocated) {
                break;
            }
        }

        if(index >= partitions.length()) {
            break;
        }
        global_index = index;
        while((index + 1) < partitions.length()) {
            const Partition::Ptr nextPart = partitions.at(index + 1);
            if(nextPart->m_type == PartitionType::Unallocated) {
                partitions[global_index]->m_sectorEnd = nextPart->m_sectorEnd;
                partitions.removeAt(index + 1);
            } else if (nextPart->m_type == PartitionType::Extended) {
                ++index;
                break;
            } else {
                break;
            }
        }
        ++global_index;
    }
}
}
}
