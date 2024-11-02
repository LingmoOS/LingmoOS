#include "libparted_unit.h"
#include <QDebug>
#include "../PluginService/kcommand.h"
#include "filesystem.h"
#include <QDir>
using namespace  KServer;
namespace KInstaller {

namespace Partman {
bool syncUdev(int timeout)
{
    QStringList args = {};
    args << "settle";
    QString timestr = QString("%1").arg(timeout);
    args << timestr;
    bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe",{});
    qDebug() << "partprob:" << partbrobl;
    bool result = KServer::KCommand::getInstance()->RunScripCommand("udevadm", args);
    if(!result) {
        qWarning() << "RunScripCommand(udevadm, args) is failed";
    }
    args.clear();
    args << timestr;
    KServer::KCommand::getInstance()->RunScripCommand("sync", args);
    KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});


}

bool commit(PedDisk* pdisk)
{
    int isSuccess = ped_disk_commit(pdisk);
    qDebug() << "commit:" << isSuccess;
    if(!isSuccess) {
        isSuccess = ped_disk_commit_to_dev(pdisk);
        isSuccess = ped_disk_commit_to_os(pdisk);
    }
    bool syncUdev(300);
    if(isSuccess) {
        return true;
    } else {
        return false;
    }
}

void destroyDevice(PedDevice* pdev)
{
    if(pdev != nullptr) {
        ped_device_destroy(pdev);
        pdev = nullptr;

    }

}

void destroyDisk(PedDisk* pdisk)
{
    if(pdisk != nullptr) {
        ped_disk_destroy(pdisk);
        pdisk = nullptr;
    }
}

bool createPartitionTable(const QString &devpath, PartTableType tabletype)
{
    qDebug() << "createPartitionTable()" << devpath;
    PedDiskType* pDiskType = nullptr;
    PedDevice* pDevice = nullptr;
    pDevice = ped_device_get(devpath.toStdString().c_str());

    switch (tabletype) {
    case PartTableType::GPT:
        pDiskType = ped_disk_type_get("gpt");
        qDebug() << "createPartitionTable gpt" << pDevice;
        break;
    case PartTableType::MsDos:
        pDiskType = ped_disk_type_get("msdos");
        qDebug() << "createPartitionTable msdos" << pDiskType;
        break;
    default:
        qCritical() << "PartTableType tableType is unkown  ";
        break;
    }

    if(pDiskType == nullptr) {
        qCritical() << "PartTableType is null";
    }

    if(pDevice != nullptr) {
        PedDisk* pDisk = nullptr;
        //Create a new partition table on dev.
        //This new partition table is only created in-memory,
        //and nothing is written to disk until ped_disk_commit_to_dev() is called.
        pDisk = ped_disk_new_fresh(pDevice, pDiskType);
        if(pDisk != nullptr) {
            commit(pDisk);
            destroyDevice(pDevice);
            destroyDisk(pDisk);
            return true;
        } else {
            qDebug() << "ped_disk_new_fresh return nullptr" << devpath;
            destroyDevice(pDevice);
            return false;
        }

    } else {
        qDebug() << "ped_device_get return nullptr" << devpath;
        return false;
    }

}
bool getDevice(const QString devpath, PedDevice* &pdev)
{
    if(devpath.isEmpty()) {
        qWarning() << "devpath is null";
        return false;
    } else {
        pdev = ped_device_get(devpath.toStdString().c_str());
        if(!pdev) {
            destroyDevice(pdev);
            return false;
        } else {
            flushDevice(pdev);
            return true;
        }
    }
}
bool getDisk(PedDevice* &pdev, PedDisk* &pdisk)
{
    if(pdev) {
        pdisk = ped_disk_new(pdev);
        if(pdisk) {
            return true;
        } else {
            destroyDisk(pdisk);
            return false;
        }
    } else {
        destroyDevice(pdev);
    }
}

PedPartitionType getPedPartitionType(Partition::Ptr partition)
{
    PedPartitionType type;
    switch (partition->m_type) {
    case PartitionType::Normal:
        type = PedPartitionType::PED_PARTITION_NORMAL;
        break;
    case PartitionType::Logical:
        type = PedPartitionType::PED_PARTITION_LOGICAL;
        break;
    case PartitionType::Extended:
        type = PedPartitionType::PED_PARTITION_EXTENDED;
        break;
    default:
        type = PedPartitionType::PED_PARTITION_FREESPACE;
        break;
     }
    return type;
}
QString getPedFsTypeName(Partition::Ptr partition)
{
    QString fstypeName = "";
    fstypeName = findNameByFSType(partition->fs);
    if(fstypeName.isEmpty()) {
        qWarning() << "没有设置文件系统,默认为ext4";
        fstypeName = findNameByFSType(FSType::Ext4);
    }
    return fstypeName;
}
PedFileSystemType* getPedFSType(Partition::Ptr partition)
{
    PedFileSystemType* fstype = nullptr;
    QString fsname = getPedFsTypeName(partition);
    fstype = ped_file_system_type_get(fsname.toStdString().c_str());
    return fstype;

}


bool createPartition(Partition::Ptr partition)
{
    qDebug() << "createPartition()" << partition->m_partitionPath
             << partition->m_sectorStart << "___" << partition->m_sectorEnd;
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {

        PedFileSystemType* fstype = getPedFSType(partition);
        PedPartitionType type = getPedPartitionType(partition);

        PedPartition* pPartition = ped_partition_new(pDisk, type, fstype,
                                                     partition->m_sectorStart, partition->m_sectorEnd);

        if(pPartition) {
            PedConstraint* pConstraint = nullptr;
            PedGeometry* pGemo = ped_geometry_new(pDevice, partition->m_sectorStart, partition->getSectorLenth());
            if(pGemo) {
                pConstraint = ped_constraint_exact(pGemo);
            } else {
                qWarning() << "分区中gemo返回为空";
            }
            if(!pConstraint) {
                pConstraint = ped_constraint_new_from_max(pGemo);
            }
            if(pConstraint) {
                isOK = (bool)ped_disk_add_partition(pDisk, pPartition, pConstraint);
                if(isOK) {
                    isOK = commit(pDisk);
                    qDebug() << "commit isOK:" <<partition->m_partitionPath<< isOK;
                } else {
                    qCritical() << "将分区添加到磁盘失败";
                }
                ped_geometry_destroy(pGemo);
                ped_constraint_destroy(pConstraint);
            } else {
                qCritical() << "PedConstraint返回为空";
            }

        } else {
            qCritical() << "ped_partition_new返回为空" << pPartition;
        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);

    } else {
        qCritical() << "获取设备和磁盘对象失败";
    }
    KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

    return isOK;
}

bool flushDevice(PedDevice *pdev)
{
    bool success = false;
    if(ped_device_open(pdev)) {
        success = static_cast<bool>(ped_device_sync(pdev));
        ped_device_close(pdev);
    }
    return success;
}


bool DeletePartition(Partition::Ptr partition)
{
    qDebug() << "DeletePartition()删除分区" << partition->m_partitionPath << partition
                << partition->m_sectorStart << "___" << partition->m_sectorEnd;
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        PedPartition* pPartition = nullptr;
        if(partition->m_type == PartitionType::Extended) {
            pPartition = ped_disk_extended_partition(pDisk);
        } else {
            pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        }
        if(pPartition) {
            isOK = (bool)ped_disk_delete_partition(pDisk, pPartition);
            KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
            qDebug() << "DeletePartition:" <<partition->m_partitionPath << isOK;
            if(isOK) {
                isOK = commit(pDisk);
            } else {
                qCritical() << "从磁盘上删除分区失败";
            }
        } else {
            qCritical() << pDisk << "获取分区对象为空";
        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);
    } else {
         qCritical() << "获取设备和磁盘对象失败";
    }
    return isOK;
}

bool resetPartitionFSType(Partition::Ptr partition)
{
    qDebug() << "resetPartitionType()重设分区类型";
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        QString FStype = getPedFsTypeName(partition);
        PedPartition* pPartition = nullptr;
        if(partition->m_type == PartitionType::Extended) {
            pPartition = ped_disk_extended_partition(pDisk);
        } else {
            pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        }
        if(pPartition) {
            isOK = (bool)ped_partition_set_system(pPartition, getPedFSType(partition));
            if(isOK) {
                isOK = commit(pDisk);
            } else {
                qCritical() << "重新设置文件系统失败";
            }
        } else {
            qCritical() << "获取分区对象为空";
        }

        destroyDevice(pDevice);
        destroyDisk(pDisk);
    } else {
        qCritical() << "获取设备和磁盘对象失败";
    }
    return isOK;
}

bool resetPartitionSize(Partition::Ptr partition)
{
    qDebug() << "resetPartitionType()重设分区类型";
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        QString FStype = getPedFsTypeName(partition);
        PedPartition* pPartition = nullptr;
        if(partition->m_type == PartitionType::Extended) {
            pPartition = ped_disk_extended_partition(pDisk);
        } else {
            pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        }
        if(pPartition) {
            PedGeometry* gemo = ped_geometry_new(pDevice, partition->m_sectorStart,
                                                 partition->getSectorLenth());
            PedConstraint* pConstraint = nullptr;
            if(gemo) {
                pConstraint = ped_constraint_exact(gemo);
            }
            if(pConstraint) {
                isOK = (bool)ped_disk_set_partition_geom(pDisk, pPartition, pConstraint,
                                                         partition->m_sectorStart,
                                                         partition->m_sectorEnd);
                if(isOK) {
                    isOK = commit(pDisk);
                } else {
                    qCritical() << "重新设置文件系统失败";
                }
                ped_geometry_destroy(gemo);
                ped_constraint_destroy(pConstraint);
            } else {
                qCritical() << "获取pConstraint对象为空";
            }
        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);
    }

    return isOK;
}

bool checkCanCreate(PedDevice* pdev)
{
    qDebug() << "checkCanCreate检测当前磁盘是否能创建分区";
//    PedDisk* pDisk = nullptr;
//    pDisk = ped_disk_new(pdev);
//    if(pDisk) {
//        if(pDisk->type == PartTableType::MsDos) {
//            if(ped_disk_get_primary_partition_count(pDisk) >=
//                    ped_disk_get_max_primary_partition_count(pDisk) &&
//                    ped_disk_extended_partition(pDisk)) {
//                qDebug() << "MSDOS主分区数量和扩展分区数量和不能超过4个,不能再进行分区";
//                return false;
//            }
//            return true;
//        }
//        destroyDisk(pDisk);
//        return true;
//    }
}

//QString getDevicePath(PedPartition* pPartition)
//{
//    char*  pPath = ped_partition_get_path(pPartition);
//    QString path;
//    if(pPath != nullptr) {
//        path = pPath;
//        free(pPath);
//    }
//    return path;
//}

QString getPartitionPath(PedPartition* pPartition)
{
    char*  pPath = ped_partition_get_path(pPartition);
    QString path = QString("");
    if(pPath != nullptr) {
        path = pPath;
        free(pPath);
    }
    return path;
}


bool upatePartitionNumber(Partition::Ptr partition)
{
    qDebug() << "upatePartitionNumber()更新分区编号";
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        PedPartition* pPartition = nullptr;
        if(partition->m_type == PartitionType::Extended) {
            pPartition = ped_disk_extended_partition(pDisk);
        } else {
            pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        }
        if(pPartition) {
            partition->m_num = pPartition->num;
            partition->m_partitionPath = getPartitionPath(pPartition);
//            partition->m_devPath =getPartitionPath(pPartition);
//            partition->m_devPath.replace(QRegExp(QString::number(partition->m_num)),"");
            isOK = true;
        } else {
            qCritical() << "该分区编号为空";
        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);
    } else {
        qCritical() << "新分区编号获取失败" << partition;
    }
    return isOK;

}

bool setPartitionFlag(Partition::Ptr partition, PedPartitionFlag flag, bool blset)
{
    qDebug() << "setPartitionFlag()" << partition << flag << blset;

    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        PedPartition* pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        if(pPartition) {
            isOK = (bool)ped_partition_set_flag(pPartition, flag, blset);
        }
        if(isOK) {
            isOK = commit(pDisk);
        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);
    }
    return isOK;
}

bool setPartitionFlags(Partition::Ptr partition)
{
    for(PartitionFlag flag : partition->flags) {
        if(!setPartitionFlag(partition, static_cast<PedPartitionFlag>(flag), true)) {
            return false;
        }
    }
    return true;
}

bool resizePartition(Partition::Ptr partition)
{
    qDebug() << "resizePartition()" << partition;
    PedDevice* pDevice = nullptr;
    PedDisk* pDisk = nullptr;
    bool isOK = false;

    if(getDevice(partition->m_devPath, pDevice) && getDisk(pDevice, pDisk)) {
        PedPartition* pPartition = nullptr;

        if(partition->m_type == PartitionType::Extended) {
            pPartition = ped_disk_extended_partition(pDisk);
        } else {
            pPartition = ped_disk_get_partition_by_sector(pDisk, partition->getSector());
        }
        if(pPartition) {
            PedGeometry* geom = ped_geometry_new(pDevice, partition->m_sectorStart, partition->getSectorLenth());
            PedConstraint* constraint = nullptr;
            if(geom) {
                constraint = ped_constraint_exact(geom);
            }
            if(constraint) {
                isOK = (bool)ped_disk_set_partition_geom(pDisk, pPartition, constraint, partition->m_sectorStart, partition->m_sectorEnd);
                if(isOK) {
                    isOK = commit(pDisk);
                }
                ped_geometry_destroy(geom);
                ped_constraint_destroy(constraint);
            }

        }
        destroyDevice(pDevice);
        destroyDisk(pDisk);
    }
    return isOK;
}

}


}
