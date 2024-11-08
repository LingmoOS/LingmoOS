#include <QDebug>
#include "custom_partition_delegate.h"
#include "partman/partition_unit.h"
#include "customize_device_model.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "validatestate.h"
#include "../partman/filesystem.h"

namespace KInstaller {

using namespace Partman;
using namespace KServer;

CustomPartitiondelegate::CustomPartitiondelegate(QObject *parent) : PartitionDelegate(parent)
{

}

CustomPartitiondelegate::~CustomPartitiondelegate()
{

}

//针对不同的体系架构，支持的文件系统不一样
QStringList CustomPartitiondelegate::getFsTypeList()
{
    FsTypeList fstype = {};
    QStringList fsnames = {};
    if (fstype.isEmpty()) {
        QString name = KServer::ReadSettingIni("setting", "FileSystem");
        Q_ASSERT(!name.isEmpty());
        fsnames = name.split(';');
        for (QString& fsname : fsnames) {
            QString cmd = FSFormatCmdMap[findFSTypeByName(fsname)];
            if (cmd.isEmpty())
                continue;
            FSType type = findFSTypeByName(fsname);
            fstype.append(type);
        }
    }
    return (QStringList)fsnames;
}

void CustomPartitiondelegate::writeMountOperate()
{
    iniPartitions.clear();
    for (int var = 0; var < m_virtualDevices.length(); ++var) {
        for(Partition::Ptr partition : m_virtualDevices.at(var)->partitions) {
            if(partition->fs == FSType::LinuxSwap || partition->fs == FSType::LingmoData) {
                QString fsstr = findNameByFSType(partition->fs);
                if(partition->m_devPath.contains("nvme", Qt::CaseInsensitive)) {
                    iniPartitions.append(QString("%1:%2p%3;").arg(fsstr,
                                                                  partition->m_devPath,
                                                                  QString::number(partition->m_num)));
                } else
                    iniPartitions.append(QString("%1:%2%3;").arg(fsstr,
                                                                 partition->m_devPath,
                                                                 QString::number(partition->m_num)));
            } else if(partition->m_isMounted || partition->m_mountPoint != ""){
                if(partition->m_devPath.contains("nvme", Qt::CaseInsensitive)) {
                    iniPartitions.append(QString("%1:%2p%3;").arg(partition->m_mountPoint,
                                                                  partition->m_devPath,
                                                                  QString::number(partition->m_num)));
                } else
                    iniPartitions.append(QString("%1:%2%3;").arg(partition->m_mountPoint,
                                                                 partition->m_devPath,
                                                                 QString::number(partition->m_num)));
            }
        }
    }

}

QString CustomPartitiondelegate::comboxOperatorPartitions()
{
    auto operations_ = operations();
    QString tmp = "";
    for(OperationDisk operate : operations_) {
//        tmp.append(operate.OperationDiskDescription());
        if(operate.m_type == OperatorTYPE::Create) {
            tmp.append(QString(tr("#%1 partition on the device %2 will be created.\n")).arg(QString::number(operate.m_newPartition->m_num),
                                                                                  operate.m_newPartition->m_devPath));
            if(operate.m_newPartition->m_isMounted || operate.m_newPartition->m_mountPoint != "") {
                tmp.append(QString(tr("#%1 partition on the device %2 will be mounted %3.\n")).arg(QString::number(operate.m_newPartition->m_num),
                                                                                                operate.m_newPartition->m_devPath, operate.m_newPartition->m_mountPoint));
            }
            if(operate.m_newPartition->fs != FSType::Empty) {
                tmp.append(QString(tr("#%1 partition on the device %2 will be formated %3.\n")).arg(QString::number(operate.m_newPartition->m_num),
                                                                                operate.m_newPartition->m_devPath, findNameByFSType(operate.m_newPartition->fs)));
            }
        }
         if(operate.m_type == OperatorTYPE::Delete) {
            tmp.append(QString(tr("#%1 partition on the device %2 will be deleted.\n")).arg(QString::number(operate.m_origPartition->m_num),
                                                                                  operate.m_origPartition->m_devPath));
         }
         if(operate.m_type == OperatorTYPE::Format) {
             tmp.append(QString(tr("#%1 partition on the device %2 will be formated %3.\n")).arg(QString::number(operate.m_newPartition->m_num),
                                                                                             operate.m_newPartition->m_devPath,  findNameByFSType(operate.m_newPartition->fs)));
         }
         if(operate.m_type == OperatorTYPE::MountPoint) {
             tmp.append(QString(tr("#%1 partition  on the device %2 will be mounted %3.\n")).arg(QString::number(operate.m_newPartition->m_num),
                                                                                              operate.m_newPartition->m_devPath, operate.m_newPartition->m_mountPoint));
         }
//        case OperatorTYPE::Format:
//            tmp.append(QString(tr("#%1 partition on the device %2 will be formated %3\n")).arg(QString::number(operate.m_newPartition->m_num),
//                                                                                  operate.m_newPartition->m_devPath,
//                                                                                   findNameByFSType(operate.m_newPartition->fs)));
//            break;
//        case OperatorTYPE::MountPoint:
//            tmp.append(QString(tr("#%1 partition on the device %2 was mounted %3\n")).arg(QString::number(operate.m_newPartition->m_num),
//                                                                                    operate.m_newPartition->m_devPath,
//                                                                                    operate.m_newPartition->m_mountPoint));
//            break;
        if(operate.m_type == OperatorTYPE::NewPartitionTable) {
            if(operate.m_dev->m_partTableType == PartTableType::GPT) {
              tmp.append(QString(tr("%1 GPT new partition table will be created.\n")).arg(operate.m_dev->m_path));
        } else if(operate.m_dev->m_partTableType == PartTableType::MsDos) {
              tmp.append(QString(tr("%1 MsDos new partition table will be created.\n")).arg(operate.m_dev->m_path));
            }
        }
    }
    return (QString)tmp;
}

FsTypeList CustomPartitiondelegate::getBootFsTypeList() const
{
    FsTypeList fstypes = {};
    if(fstypes.isEmpty()) {
        QString name = KServer::ReadSettingIni("setting", "FileSystemBoot");
        Q_ASSERT(!name.isEmpty());
        QStringList fsnames = name.split(';');
        for(QString fsname : fsnames) {
            FSType type = findFSTypeByName(fsname);
            fstypes.append(type);
        }
    }
    return (FsTypeList)fstypes;
}
QStringList CustomPartitiondelegate::getMountPoints()
{
    QStringList mountPoints = {};
    if(mountPoints.isEmpty()) {
        mountPoints = KServer::ReadSettingIni("setting", "PartitionMountedPoints").split(';');
    }
    return (QStringList)mountPoints;
}



QList<Device::Ptr> CustomPartitiondelegate::getAllUsedDevice()
{
    QList<Device::Ptr> list;
    auto operations_ = operations();
    auto realDevices_ = realDevices();

    for(const OperationDisk& operation : operations_) {
        if(operation.m_type != OperatorTYPE::NewPartitionTable) {
            for(Device::Ptr dev : realDevices_) {
                if(list.contains(dev)) {
                    continue;
                }
                if(dev->m_path == operation.m_origPartition->m_devPath) {
                    list << dev;
                    break;
                }
            }
        }
    }
    return (QList<Device::Ptr>)list;
}

void CustomPartitiondelegate::onCustomPartDone(const DeviceList& devices)
{
    qDebug() << "Custom_Partition_delegate::onCustomPartDone" <<  devices;

    QString rootDisk = "";
    QString rootPath = "";
    QStringList mountPoints = {};
    bool foundswap = false;
    QString espPath = "";
    Device::Ptr rootDev;
    Partition::Ptr efiPartition;

    for(const Device::Ptr dev : devices) {
        for(const Partition::Ptr partiton : dev->partitions) {
            if(!partiton->m_mountPoint.isEmpty()) {
                QString record(QString("%1=%2").arg(partiton->m_partitionPath).arg(partiton->m_mountPoint));
                mountPoints.append(record);
                if(partiton->m_mountPoint == "/") {
                    rootDisk = partiton->m_devPath;
                    rootPath = partiton->m_partitionPath;
                    rootDev = dev;
                }
            }
        }
    }
    for(Partition::Ptr partition : rootDev->partitions) {
        if(partition->fs == FSType::EFI && espPath != partition->m_partitionPath) {
            espPath = partition->m_partitionPath;
            break;
        }
    }

    for(Partition::Ptr partition : rootDev->partitions) {
        if(partition->fs == FSType::LinuxSwap) {
            foundswap = true;
            QString record(QString("%1=swap").arg(partition->m_partitionPath));
            mountPoints.append(record);
        }
    }
    if(!isMBRPreferred(realDevices())) {
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

    bool use_swap_file;
    if(foundswap) {
        use_swap_file = false;
    } else if(!getSettingBool("EnableSwap")) {
        use_swap_file = false;
    } else {
        use_swap_file = isSwapAreaNeeded();
    }
    m_setting.swapFileRequire = use_swap_file;
    writeDiskPartitionSetting(m_setting);

}
bool CustomPartitiondelegate::unFormatPartition(Partition::Ptr part)
{
    Q_ASSERT(part->m_status == PartitionStatus::Format);
    if(part->m_status == PartitionStatus::Format) {
        OperationDiskList operatordisks = operations();
        for(int index = operatordisks.length() - 1; index >= 0; --index) {
            const OperationDisk& operation = operatordisks.at(index);

            if(operation.m_type == OperatorTYPE::Format && operation.m_newPartition == part) {
                operatordisks.removeAt(index);
                return true;
            }
        }
        qCritical() << "No formatPartition found:" << part;
    } else {
        qCritical() << "Invalid partition status:" << part;
    }
    return false;
}

bool CustomPartitiondelegate::isPartitionTableMatchDe(const QString &devPath)
{
    qDebug() << Q_FUNC_INFO << realDevices();
    return isPartitionTableMatch(realDevices(), devPath);
}

Device::Ptr CustomPartitiondelegate::FlushAddPartition(Device::Ptr dev, Partition::Ptr orgPartition, Partition::Ptr newPartition)
{
    Device::Ptr device(dev);
    if(orgPartition->getTotalSpace() == newPartition->getTotalSpace()) {

    }
    if(newPartition->m_type == PartitionType::Logical) {

    }
    return (Device::Ptr)device;
}

ValidateStates CustomPartitiondelegate::validate() const
{
    ValidateStates states = {};
    bool found_efi = false;

    bool repeated_mount = false;
    QStringList repeated_mount_listStr;

    Partition::Ptr rootPartition;
    bool root_size_enough = false;
    int rootcount = 0;
    FSType rootfs = FSType::Empty;

    Partition::Ptr bootPartition;
    bool boot_size_enough = false;
    FSType bootfs = FSType::Empty;

    Partition::Ptr efiPartition;
    bool efi_size_enough = true;
    int efiCount = 0;

    //TODO
    //从配置文件中读取检测要求
    int root_required = 0;//>15G
    //HUAWEI990,9a0 root >=25 2021-3-11
    QString tmp = getDeviceStrByType(getDeviceModel());
    tmp.remove(QRegExp("\\s"));//去除字符串中间空格
    if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {
        root_required = 25;
    } else {
        root_required = 15;
    }

    int boot_required = 500;//>500M
    qint64 efi_maxsize = 2*kGibiByte;//2G
    qint64 efi_minsize = 256*kMebiByte;//256M
    int partition_minsize_gb = 0 ;
    qint64 partition_minsize_bytes = partition_minsize_gb * kGibiByte;

    Device::Ptr root_device;
    for(const Device::Ptr device : virtualDevices()) {
        for(const Partition::Ptr partition : device->partitions) {
            QString str = partition->m_mountPoint;
            if(!str.isEmpty()) {
                repeated_mount_listStr.append(str);
            }
            //2021-3-6添加提示fat16和fat32文件系统使用问题
            if( partition->m_mountPoint !="") {
                if(partition->m_mountPoint != "/boot/efi") {
                    if(partition->fs == FSType::Fat16 || partition->fs == FSType::Fat32) {
                        states.append(ValidateState::InvalidFS);
                    }
                }
            }

            if(partition->m_mountPoint == "/") {
                ++rootcount;
                root_device = device;
                rootPartition = partition;
                rootfs = partition->fs;
                const qint64 root_real_bytes = partition->getByteLength() + kMebiByte;
                const qint64 root_minsize_bytes = root_required * kGibiByte;
                root_size_enough = (root_real_bytes >= root_minsize_bytes);
            } else if(partition->m_mountPoint == "/boot") {
                //适配龙芯的boot分区必须ext3才能启动的问题,龙芯EFI分区
//                if(checkSpecialModel() == 0) {
//                    states.append(ValidateState::MipsBootExt3);
//                }
                bootPartition = partition;
                bootfs = partition->fs;
                const qint64 boot_required_bytes = boot_required * kMebiByte;
                const qint64 boot_real_bytes = partition->getByteLength() +kMebiByte;
                boot_size_enough = (boot_real_bytes >= boot_required_bytes);
            } else if(partition->m_mountPoint == "/boot/efi") {
                //适配龙芯的mips龙芯不能有EFI分区
                if(getMips() && !checkSpecialModel()) {
                    states.append(ValidateState::MipsNotSupportEFI);
                }
                efiCount++;
                if(efiCount > 1) {
                    states.append(ValidateState::EfiTooMany);
                }
                efiPartition = partition;
                qint64 efi_real_bytes=partition->getByteLength();
              //  efi_size_enough = !(efi_real_bytes > efi_maxsize || efi_real_bytes < efi_minsize);
                qDebug()<<"efi_real_bytes="<<efi_real_bytes<<";efi_maxsize="<<efi_maxsize<<";efi_minsize="<<efi_minsize<<";efi_size_enough="<<efi_size_enough;
            }

        }
    }

    if(!root_device.isNull()) {
        for(Partition::Ptr partition : root_device->partitions) {
            if(partition->fs == FSType::EFI) {
                found_efi = true;
                efiPartition = partition;
                if(partition->m_status == PartitionStatus::Real) {//使用了原有分区
//                    const qint64 efi_minsize_bytes = efi_minsize * kMebiByte;
//                    const qint64 efi_real_bytes = partition->getByteLength() + kMebiByte;
//                    efi_size_enough = (efi_real_bytes >= efi_minsize_bytes);
                } else if(partition->m_status == PartitionStatus::New || partition->m_status == PartitionStatus::Format) {
//                    const qint64 efi_requird_bytes = efi_required * kMebiByte;
//                    const qint64 efi_real_bytes = partition->getByteLength() + kMebiByte;
//                    efi_size_enough = (efi_real_bytes >= efi_requird_bytes);
                    qint64 efi_real_bytes=partition->getByteLength();
                    efi_size_enough = !(efi_real_bytes > efi_maxsize || efi_real_bytes < efi_minsize);
                    qDebug()<<"efi_real_bytes="<<efi_real_bytes<<";efi_maxsize="<<efi_maxsize<<";efi_minsize="<<efi_minsize<<";efi_size_enough="<<efi_size_enough;
                    if(!efi_size_enough)
                        break;//避免后续efi分区的检查结果覆盖掉此次出错提示
                }
                break;
            }
        }
    }
    if(!rootPartition.isNull()) {
        if(!root_size_enough) {
            states.append(ValidateState::RootTooSmall);
        }
    } else {
        states.append(ValidateState::RootMissing);
    }


    if(!isMBRPreferredDe()) {
        if(found_efi & (efiCount > 0)) {
            if(!efi_size_enough) {
                states.append(ValidateState::EfiSizeError);
            }
        } else if(efiCount == 0){
            if(!rootPartition.isNull()) {
                states.append(ValidateState::EfiMissing);
            }
        }
    }
//    if(checkSpecialModel() == 0 && bootPartition.isNull()) {
//        states.append(ValidateState::BootMissing);
//    }
//    if(!bootPartition.isNull() && !boot_size_enough) {
//        states.append(ValidateState::BootTooSmall);
//    }
//    if(!efiPartition.isNull() && !efi_size_enough){
//        states.append(ValidateState::EfiSizeError);
//    }


/*注释boot分区文件系统限制
    if(!bootPartition.isNull() || !rootPartition.isNull()) {
        const FSType boot_root_fs = !bootPartition.isNull() ? bootfs : rootfs;
        FsTypeList bootfsList = getBootFsTypeList();
        if(!bootfsList.contains(boot_root_fs)) {
            states.append(ValidateState::BootFsInvalid);
        }
    }
    //TODO
    //boot on first partition*/
    if(checkSpecialModel() == 0 && !bootPartition.isNull()) {
        for(const Device::Ptr device : virtualDevices()) {
            PartitionList list = {};
            for(const Partition::Ptr partition : device->partitions) {
                if(partition->m_type != PartitionType::Unallocated) {
                    list << partition;
                }
            }
            if(list.isEmpty()) {
                continue;
            }
//            if(isEFIEnabled()) {
//                int index = 0;
//                index = list.indexOf(efiPartition);
//                if(index != -1 && index != 0) {
//                    states.append(ValidateState::EfiPartNumberInvalid);
//                    break;
//                }
//                continue;
//            }
            int index = 0;
            index = list.indexOf(bootPartition);
            if(index != -1 && index != 0 && getMips()) {
                states.append(ValidateState::BootPartNumberInvalid);
                break;
            }
//            index = list.indexOf(rootPartition);
//            if(bootPartition.isNull() && index != -1 && index != 0) {
//                states.append(ValidateState::BootPartNumberInvalid);
//                break;
//            }
        }
    }
    const QStringList knownmounts{"/", "/boot","/data","/backup"};
    int bldatabackup = 0;
    for(const Device::Ptr& device : virtualDevices()) {
        for(const Partition::Ptr& partition : device->partitions) {
            if(partition->fs == FSType::EFI) {
                continue;
            }
            if(partition->m_mountPoint.isEmpty()) {
                continue;
            }
            if(knownmounts.contains(partition->m_mountPoint)) {
                continue;
            }
            if(partition->getByteLength() < partition_minsize_bytes) {
                states.append(ValidateState(ValidateState::PartitionTooSmall, partition));
            }
        }
    }

    //检测是否存在备份还原分区

    return (ValidateStates)states;
}

}
